#pragma once

#include <config.h>

#include <Windows.h>

class Program
{
public:
	void onProcessAttach();
	void onProcessDetach() const;

private:
	void load() const;
	void createConsole() const;
	Config loadConfig() const;
	void unload() const;

	const char* configFileName = "dumper_config.yaml";

	HANDLE loadThreadHandle = NULL;
};
