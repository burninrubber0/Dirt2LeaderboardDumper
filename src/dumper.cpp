#include <dumper.h>
#include <cppcodec/base64_rfc4648.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

Dumper::Dumper(int argc, char* argv[])
{
	dumpLeaderboard();
}

void Dumper::dumpLeaderboard()
{
	// Client that sends POSTs
	std::unique_ptr<httplib::Client> client(new httplib::Client("ranking-view-b01.u0.np.community.playstation.net"));

	// Headers (matching PS3)
	std::unique_ptr<httplib::Headers> headers(new httplib::Headers());
	headers->insert(std::pair("Connection", "Keep-Alive"));
	headers->insert(std::pair("Accept-Encoding", "identity"));
	headers->insert(std::pair("User-Agent", "PS3Community-agent/1.0.0 libhttp/1.0.0"));

	// Build the XML body
	std::unique_ptr<tinyxml2::XMLDocument> request(new tinyxml2::XMLDocument());
	setupRequest(*request);
	std::unique_ptr<tinyxml2::XMLPrinter> printer(new tinyxml2::XMLPrinter(0, true));

	// Create necessary folders
	if (!std::filesystem::exists("xml"))
		std::filesystem::create_directory("xml");
	if (!std::filesystem::exists("csv"))
		std::filesystem::create_directory("csv");

	// Begin sending requests
	tinyxml2::XMLDocument* board;
	for (int i = 1; i <= numBoards; ++i)
	{
		// Get total number of entries
		updateRequest(*request, i, 1, 1);
		request->Print(printer.get());
		int totalEntries = getTotal(client->Post("/ranking_view/func/get_ranking", *headers, printer->CStr(), "application/xml"));
		printer->ClearBuffer();

		// Get the individual XML documents ("segments"), combine them afterward
		std::vector<tinyxml2::XMLDocument*> segments;
		std::string totalSegments = std::to_string((totalEntries / entriesPerRequest) + 1);
		for (int j = 1; j <= totalEntries; j += entriesPerRequest)
		{
			updateRequest(*request, i, j, entriesPerRequest);
			request->Print(printer.get());
			httplib::Result result = client->Post("/ranking_view/func/get_ranking", *headers, printer->CStr(), "application/xml");
			printer->ClearBuffer();
			segments.push_back(new tinyxml2::XMLDocument());
			segments.back()->Parse(result.value().body.c_str());
			std::cout << "\rLeaderboard " << i << ": retrieved " << ((j - 1) / entriesPerRequest) + 1 << "/" << totalSegments << " segments";
		}
		std::cout << '\n';
		board = mergeSegments(segments, i);

		// Save files
		int trackId = (((i - 1) / numDisciplines) * numDisciplines) + 1;
		int disciplineId = (i - 1) % numDisciplines;
		saveXml(board, trackId <= 287 ? trackId : i, disciplineId, trackId <= 287);
		saveCsv(board, trackId <= 287 ? trackId : i, disciplineId, trackId <= 287);

		// Destroy XML object
		delete board;
		board = nullptr;
	}

	std::cout << "\nDump complete.\n";
}

void Dumper::setupRequest(tinyxml2::XMLDocument& request)
{
	const char* initialRequestText = "<?xml version=\"1.0\" encoding=\"utf-8\"?><ranking platform=\"ps3\" sv=\"4.91\"><titleid>NPWR00429_00</titleid><board>1</board><start>1</start><list-max>10</list-max><option message=\"false\" info=\"true\"/></ranking>";
	request.Parse(initialRequestText);
}

void Dumper::updateRequest(tinyxml2::XMLDocument& request, int boardId, int entryIndex, int entryCount)
{
	request.RootElement()->FirstChildElement("board")->SetText(std::to_string(boardId).c_str());
	request.RootElement()->FirstChildElement("start")->SetText(std::to_string(entryIndex).c_str());
	request.RootElement()->FirstChildElement("list-max")->SetText(std::to_string(entryCount).c_str());
}

// Get the entry count for the leaderboard
int Dumper::getTotal(httplib::Result result)
{
	std::unique_ptr<tinyxml2::XMLDocument> response(new tinyxml2::XMLDocument());
	response->Parse(result.value().body.c_str());
	return std::stoi(response->RootElement()->FirstChildElement("total")->GetText());
}

tinyxml2::XMLDocument* Dumper::mergeSegments(std::vector<tinyxml2::XMLDocument*>& segments, int boardId)
{
	// Start with the first segment as a base
	tinyxml2::XMLDocument* board = new tinyxml2::XMLDocument();
	segments[0]->DeepCopy(board);

	// Merge all other segments into board
	for (int i = 1; i < segments.size(); ++i)
	{
		tinyxml2::XMLElement* list = segments[i]->RootElement()->FirstChildElement("list");
		int numRecords = std::stoi(list->Attribute("number"));
		tinyxml2::XMLElement* segRecord = list->FirstChildElement("record");
		for (int j = 0; j < numRecords; ++j)
		{
			tinyxml2::XMLElement* boardRecord = board->RootElement()->FirstChildElement("list")->InsertNewChildElement("record");
			boardRecord->SetAttribute("serial", segRecord->Attribute("serial"));
			boardRecord->InsertNewChildElement("jid");
			boardRecord->FirstChildElement("jid")->SetText(segRecord->FirstChildElement("jid")->GetText());
			boardRecord->InsertNewChildElement("rank");
			boardRecord->FirstChildElement("rank")->SetText(segRecord->FirstChildElement("rank")->GetText());
			boardRecord->InsertNewChildElement("max-rank");
			boardRecord->FirstChildElement("max-rank")->SetText(segRecord->FirstChildElement("max-rank")->GetText());
			boardRecord->InsertNewChildElement("score");
			boardRecord->FirstChildElement("score")->SetText(segRecord->FirstChildElement("score")->GetText());
			boardRecord->InsertNewChildElement("name");
			boardRecord->FirstChildElement("name")->SetText(segRecord->FirstChildElement("name")->GetText());
			boardRecord->InsertNewChildElement("info");
			boardRecord->FirstChildElement("info")->SetText(segRecord->FirstChildElement("info")->GetText());
			boardRecord->InsertNewChildElement("data");
			boardRecord->FirstChildElement("data")->SetText(segRecord->FirstChildElement("data")->GetText());
			boardRecord->InsertNewChildElement("date");
			boardRecord->FirstChildElement("date")->SetText(segRecord->FirstChildElement("date")->GetText());
			segRecord = segRecord->NextSiblingElement();
		}
		std::cout << "\rLeaderboard " << boardId << "/" << numBoards
			<< ": merged records for " << i + 1 << "/" << segments.size() << " segments";
	}
	std::cout << '\n';

	// Set list number attribute to total number of records
	board->RootElement()->FirstChildElement("list")->SetAttribute("number",
		board->RootElement()->FirstChildElement("total")->GetText());

	// Destroy segments, no longer needed
	for (int i = 0; i < segments.size(); ++i)
		delete segments[i];

	return board;
}

void Dumper::saveXml(tinyxml2::XMLDocument* board, int boardId, int disciplineId, bool isTimeBoard)
{
	if (isTimeBoard)
		board->SaveFile(std::string("xml/" + boards.at(boardId) + " - " + disciplines.at(disciplineId) + ".xml").c_str());
	else
		board->SaveFile(std::string("xml/" + boards.at(boardId) + ".xml").c_str());
}

void Dumper::saveCsv(tinyxml2::XMLDocument* board, int boardId, int disciplineId, bool isTimeBoard)
{
	std::string csv = makeCsv(board, isTimeBoard);
	std::ofstream csvOut;
	if (isTimeBoard)
		csvOut.open(std::string("csv/" + boards.at(boardId) + " - " + disciplines.at(disciplineId) + ".csv"));
	else
		csvOut.open(std::string("csv/" + boards.at(boardId) + ".csv"));
	csvOut << csv;
	csvOut.close();
}

std::string Dumper::makeCsv(tinyxml2::XMLDocument* board, bool isTimeBoard)
{
	// Header
	std::string csv = "Rank,Region,Name,Score,Date";
	if (isTimeBoard)
		csv +=",Vehicle,Grade";
	csv += "\n";

	int total = std::stoi(board->RootElement()->FirstChildElement("total")->GetText());
	tinyxml2::XMLElement* record = board->RootElement()->FirstChildElement("list")->FirstChildElement("record");
	for (int i = 0; i < total; ++i)
	{
		// Rank (ties are allowed)
		csv += std::string(record->FirstChildElement("rank")->GetText()) + ",";
		
		// Get two-letter region code from jid
		std::string region = record->FirstChildElement("jid")->GetText();
		region = region.substr(region.size() - 21, 2); // 2 characters to the left of .np.playstation.net
		std::transform(region.begin(), region.end(), region.begin(), ::toupper);
		csv += region + ",";
		
		// Player name
		csv += std::string(record->FirstChildElement("name")->GetText()) + ",";
		
		// Score
		std::string score = std::string(record->FirstChildElement("score")->GetText());
		if (score.starts_with('-'))
		{
			// Convert to time
			score = score.substr(1, score.size() - 1);
			score = formatAsTime(score);
		}
		// Otherwise it is an actual score and doesn't need to be formatted
		csv += score + ",";
		
		// Date
		csv += std::string(record->FirstChildElement("date")->GetText());
		
		if (isTimeBoard)
		{
			// Vehicle
			std::string infoEncoded = record->FirstChildElement("info")->GetText();
			std::vector<uint8_t> info = cppcodec::base64_rfc4648::decode(infoEncoded.c_str(), infoEncoded.size());
			uint32_t vehicleId = (((info[0xA] & 0xF) << 8) + (info[9] & 0xF0)) >> 3;
			csv += "," + vehicles.at(vehicleId) + ",";

			// Grade
			uint8_t vehicleGradeId = info[0xE];
			csv += grades.at(vehicleGradeId);
		}

		csv += "\n";

		record = record->NextSiblingElement();
	}

	return csv;
}

// Dumb and probably bad way of converting to minutes:seconds.milliseconds
std::string Dumper::formatAsTime(std::string milliseconds)
{
	auto ms = std::chrono::milliseconds(std::stoull(milliseconds));
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
