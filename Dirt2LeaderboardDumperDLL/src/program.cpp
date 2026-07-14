#define NOMINMAX

#include <config.h>
#include <dumper.h>
#include <program.h>

#include <fkyaml/node.hpp>

#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <Windows.h>

void Program::onProcessAttach()
{
	PTHREAD_START_ROUTINE loadThreadProc = [](LPVOID lpThreadParameter) -> DWORD
		{
			static_cast<Program*>(lpThreadParameter)->load();
			return 0;
		};
	loadThreadHandle = CreateThread(nullptr, 0, loadThreadProc, this, 0, nullptr);
}

void Program::onProcessDetach() const
{
	unload();
}

void Program::load() const
{
	createConsole();
	auto config = loadConfig();
	std::cout << "Waiting 60 seconds before dumping. Please sign in before this time elapses.\n";
	Sleep(60000);
	auto leaderboardDumper = std::make_unique<Dumper>(GetModuleHandleA("xlive.dll"), config);
	leaderboardDumper->dump();
}

void Program::createConsole() const
{
	bool newConsole = AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
}

Config Program::loadConfig() const
{
	std::string yaml;
	if (!std::filesystem::exists(configFileName))
	{
		std::cout << "No configuration detected. Defaulting to dumping Dirt 2 PC leaderboards.\n";
		yaml = "titleId: 0x434D0820\n"
			"startLeaderboard: 0\n"
			"endLeaderboard: 0\n"
			"startRank: 0\n"
			"endRank: 0\n";
		std::ofstream configFile(configFileName, std::ios::out | std::ios::binary);
		configFile.write(yaml.c_str(), yaml.length());
		configFile.close();
	}
	else
	{
		auto configFileSize = std::filesystem::file_size(configFileName);
		yaml = std::string((size_t)configFileSize, '\0');
		std::ifstream configFile(configFileName, std::ios::in);
		configFile.read(&yaml[0], configFileSize);
	}

	auto configNode = fkyaml::node::deserialize(yaml);
	Config config;
	config.titleId = configNode["titleId"].get_value<DWORD>();
	config.startLeaderboard = configNode["startLeaderboard"].get_value<DWORD>();
	config.endLeaderboard = configNode["endLeaderboard"].get_value<DWORD>();
	config.startRank = configNode["startRank"].get_value<DWORD>();
	config.endRank = configNode["endRank"].get_value<DWORD>();
	switch (config.titleId)
	{
	case (DWORD)Dumper::TitleId::Dirt2Pc:
	case (DWORD)Dumper::TitleId::Dirt2X360:
		break;
	default:
		std::cout << "Invalid title ID used. Defaulting to dumping Dirt 2 PC leaderboards.\n";
		config.titleId = (DWORD)Dumper::TitleId::Dirt2Pc;
		break;
	}

	return config;
}

void Program::unload() const
{
	try
	{
		FreeConsole();
		CloseHandle(loadThreadHandle);
	}
	catch (const std::exception& e)
	{
		MessageBoxA(NULL, e.what(), "LeaderboardDumper", MB_ICONERROR);
	}
}
