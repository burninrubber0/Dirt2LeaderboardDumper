#include <config.h>
#include <dumper.h>
#include <xlive.h>

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <Windows.h>

Dumper::Dumper(HMODULE xlive, const Config& config)
	: config(config)
{
	XUserCreateStatsEnumeratorByRank = (XUserCreateStatsEnumeratorByRank_t)GetProcAddress(xlive, MAKEINTRESOURCEA(5284));
	XEnumerate = (XEnumerate_t)GetProcAddress(xlive, MAKEINTRESOURCEA(5256));
}

void Dumper::dump() const
{
	// Set up logging and the output folder.
	std::string folderName = titles.at((Dumper::TitleId)config.titleId) + " Leaderboards";
	std::filesystem::path folder(folderName);
	bool createDirectoryResult = std::filesystem::create_directory(folder);
	if (!createDirectoryResult && !std::filesystem::exists(folder))
	{
		std::ofstream logFile(folderName + ".log", std::ios::out);
		log(logFile, std::format("Failed to create directory: {}\n", folderName));
		logFile.close();
		return;
	}

	std::ofstream logFile(folder / (folderName + ".log"), std::ios::out);

	// Loop through the leaderboards.
	DWORD startLeaderboard = config.startLeaderboard < 1 || config.startLeaderboard > totalNumLeaderboards ? 1 : config.startLeaderboard;
	DWORD endLeaderboard = config.endLeaderboard < 1 || config.endLeaderboard > totalNumLeaderboards ? totalNumLeaderboards : config.endLeaderboard;
	DWORD numLeaderboards = endLeaderboard - startLeaderboard + 1;
	std::cout << std::format("Dumping {} leaderboard(s) for {}\n", numLeaderboards, titles.at((Dumper::TitleId)config.titleId));
	for (DWORD leaderboard = startLeaderboard; leaderboard <= endLeaderboard; leaderboard++)
		dumpBoard((Dumper::TitleId)config.titleId, leaderboard, folder, logFile);
	
	logFile.close();
	std::cout << "Dump complete.\n";
}

void Dumper::dumpBoard(TitleId titleId, DWORD viewId,
	std::filesystem::path& folder, std::ofstream& logFile) const
{
	bool isTimeBoard = viewId <= 287;

	// Create the leaderboard spec.
	// The time spec is used on views 1 through 287 and has 4 columns:
	// Display score, vehicle ID, vehicle grade, and an unknown value that is either 0 or 1.
	// The Dirt Tour XP and Online Fame leaderboards (views 288 and 289) have no columns.
	int specReserve = isTimeBoard ? 0x10 : 8;
	auto specBuffer = std::make_unique<char[]>(specReserve);
	XUSER_STATS_SPEC* spec = reinterpret_cast<XUSER_STATS_SPEC*>(specBuffer.get());
	spec->dwViewId = viewId;
	if (isTimeBoard)
	{
		spec->dwNumColumnIds = 4;
		for (DWORD i = 0; i < spec->dwNumColumnIds; i++)
			spec->rgwColumnIds[i] = (WORD)i + 1;
	}
	else
	{
		spec->dwNumColumnIds = 0;
	}

	// Get the leaderboard name.
	DWORD trackId = (((viewId - 1) / disciplines.size()) * disciplines.size()) + 1;
	DWORD disciplineId = (viewId - 1) % disciplines.size();
	DWORD boardId = viewId <= 287 ? trackId : viewId;
	std::string leaderboardName = viewId <= 287
		? boards.at(boardId) + " - " + disciplines.at(disciplineId)
		: boards.at(boardId);

	// Retrieve the top ten.
	// This is used to get the total number of rows and for error checking.
	auto topTenBuffer = getEntryData(titleId, 1, 10, spec, logFile);
	if (topTenBuffer == nullptr)
	{
		logFile.close();
		return;
	}
	XUSER_STATS_READ_RESULTS* topTenReadResults = reinterpret_cast<XUSER_STATS_READ_RESULTS*>(topTenBuffer.get());
	if (topTenReadResults->dwNumViews != 1)
	{
		log(logFile, std::format("View {} ({}): number of views is not 1. Skipping.\n", viewId, leaderboardName));
		return;
	}
	if (topTenReadResults->pViews[0].dwNumRows == 0)
	{
		log(logFile, std::format("View {} ({}): no rows present. Skipping.\n", viewId, leaderboardName));
		return;
	}
	if ((isTimeBoard && topTenReadResults->pViews[0].pRows[0].dwNumColumns != 4)
		|| (!isTimeBoard && topTenReadResults->pViews[0].pRows[0].dwNumColumns != 0))
	{
		log(logFile, std::format("View {} ({}): number of columns does not match the spec. Skipping.\n", viewId, leaderboardName));
		return;
	}

	DWORD totalNumRows = topTenReadResults->pViews[0].dwTotalViewRows;
	log(logFile, std::format("Retrieved info for leaderboard {} ({}): {} entries present.\n", viewId, leaderboardName, totalNumRows));

	// Create the CSV file.
	std::ofstream csv(folder / (leaderboardName + ".csv"), std::ios::out);
	if (isTimeBoard)
		csv << "Rank,Gamertag,Score,Vehicle,Grade\n";
	else
		csv << "Rank,Gamertag,Score\n";

	// The maximum number of ranks that can be retrieved at once is 100.
	// Retrieve each segment and output the data into a CSV.
	DWORD startRank = config.startRank < 1 || config.startRank > totalNumRows ? 1 : config.startRank;
	DWORD endRank = config.endRank < 1 || config.endRank > totalNumRows ? totalNumRows : config.endRank;
	DWORD numRowsToGet = endRank - startRank + 1;
	for (DWORD rank = startRank; rank <= endRank; rank += maxRowsPerRequest)
	{
		DWORD remaining = numRowsToGet - rank + 1;
		DWORD numRows = remaining >= maxRowsPerRequest ? maxRowsPerRequest : remaining;
		auto entryData = getEntryData(titleId, rank, numRows, spec, logFile);
		if (entryData == nullptr)
		{
			log(logFile, std::format("Failed to retrieve entry data for leaderboard {} ({}) at rank {}.\n", viewId, leaderboardName, rank));
			return;
		}
		XUSER_STATS_READ_RESULTS* readResults = reinterpret_cast<XUSER_STATS_READ_RESULTS*>(entryData.get());

		if (isTimeBoard)
		{
			for (DWORD row = 0; row < readResults->pViews[0].dwNumRows; row++)
			{
				csv << readResults->pViews[0].pRows[row].dwRank << ',';
				csv << readResults->pViews[0].pRows[row].szGamertag << ',';
				csv << formatAsTime(readResults->pViews[0].pRows[row].pColumns[0].Value.i64Data) << ',';
				csv << vehicles.at(readResults->pViews[0].pRows[row].pColumns[1].Value.nData) << ',';
				csv << grades.at(readResults->pViews[0].pRows[row].pColumns[2].Value.nData) << "\n";
			}
		}
		else
		{
			for (DWORD row = 0; row < readResults->pViews[0].dwNumRows; row++)
			{
				csv << readResults->pViews[0].pRows[row].dwRank << ',';
				csv << readResults->pViews[0].pRows[row].szGamertag << ',';
				csv << readResults->pViews[0].pRows[row].i64Rating << "\n";
			}
		}
		updateStatus(viewId, leaderboardName, rank - startRank + numRows, numRowsToGet, rank + numRows > endRank);
		if (rank + numRows > endRank)
		{
			std::string message = std::format("Finished dumping leaderboard {} ({}).", viewId, leaderboardName);
			if (message.length() < 80)
				message.append(80 - message.length(), ' ');
			message += '\n';
			log(logFile, message, true);
		}
	}

	csv.close();
}

std::unique_ptr<char[]> Dumper::getEntryData(TitleId titleId, DWORD startRank, DWORD numRows,
	CONST XUSER_STATS_SPEC* spec, std::ofstream& logFile) const
{
	// Create the enumerator.
	DWORD enumerationReserveLength = 0;
	HANDLE enumerator = nullptr;
	DWORD createEnumeratorResult = XUserCreateStatsEnumeratorByRank((DWORD)titleId, startRank, numRows, 1, spec, &enumerationReserveLength, &enumerator);
	if (createEnumeratorResult != ERROR_SUCCESS)
	{
		log(logFile, std::format("Failed: XUserCreateStatsEnumeratorByRank returned {}.\n", createEnumeratorResult));
		if (enumerator != nullptr)
			CloseHandle(enumerator);
		return nullptr;
	}
	if (enumerator == nullptr)
	{
		log(logFile, "Failed: enumerator was not created.\n");
		return nullptr;
	}

	// Enumerate the results.
	auto enumerationBuffer = std::make_unique<char[]>(enumerationReserveLength);
	DWORD numEntries = 0;
	DWORD enumerateResult = XEnumerate(enumerator, enumerationBuffer.get(), enumerationReserveLength, &numEntries, NULL);
	CloseHandle(enumerator);
	if (enumerateResult != ERROR_SUCCESS)
	{
		log(logFile, std::format("Failed: XEnumerate returned {}.\n", enumerateResult));
		return nullptr;
	}
	if (enumerationBuffer == nullptr)
	{
		log(logFile, "Failed: the results were not enumerated.\n");
		return nullptr;
	}

	return enumerationBuffer;
}

void Dumper::updateStatus(DWORD viewId, std::string& leaderboardName,
	DWORD currentCount, DWORD numRowsToGet, bool isLastSegment) const
{
	std::string status = std::format("Leaderboard {} ({}): {}/{}",
		viewId, leaderboardName, currentCount, numRowsToGet);
	if (status.length() < 80)
		status.append(80 - status.length(), ' ');
	if (!isLastSegment)
		status += '\r';
	else
		status += '\n';
	std::cout << status;
}

void Dumper::log(std::ofstream& logFile, const std::string& message, bool outputToConsole) const
{
	logFile << message;
	if (outputToConsole)
		std::cout << message;
}

// Dumb and probably bad way of converting to minutes:seconds.milliseconds
std::string Dumper::formatAsTime(LONGLONG milliseconds) const
{
	auto ms = std::chrono::milliseconds(milliseconds);
	auto s = std::chrono::duration_cast<std::chrono::seconds>(ms);
	ms -= std::chrono::duration_cast<std::chrono::milliseconds>(s);
	auto m = std::chrono::duration_cast<std::chrono::minutes>(s);
	s -= std::chrono::duration_cast<std::chrono::seconds>(m);

	std::string mins(std::to_string(m.count()));
	std::string secs(std::to_string(s.count()));
	std::string millis(std::to_string(ms.count()));
	while (mins.size() < 2)
		mins.insert(0, 1, '0');
	while (secs.size() < 2)
		secs.insert(0, 1, '0');
	while (millis.size() < 3)
		millis.insert(0, 1, '0');

	return std::string(mins + ":" + secs + "." + millis);
}
