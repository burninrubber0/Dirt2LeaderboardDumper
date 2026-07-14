#include "version/version.h"
#include <program.h>

#include <string.h>
#include <Windows.h>

Program* gProgram = nullptr;

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		setupWrappers();
		char exePath[MAX_PATH];
		GetModuleFileNameA(NULL, exePath, MAX_PATH);
		if (strstr(exePath, "dirt2_game.exe") != nullptr)
		{
			gProgram = new Program();
			gProgram->onProcessAttach();
		}
		break;
	}
	case DLL_PROCESS_DETACH:
		if (gProgram != nullptr)
		{
			gProgram->onProcessDetach();
			delete gProgram;
		}
		break;
	default:
		break;
	}
	return TRUE;
}
