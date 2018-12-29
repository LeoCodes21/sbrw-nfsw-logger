// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "Memory.h"
#include <cstdio>
#include <corecrt_share.h>
#include <cstdlib>

FILE* logStream;
FILE* consoleStream;
DWORD gameLogJumpOut = 0x0052bdd8;

DWORD gameLogTextPointer;
DWORD gameLogCategoryPointer;
const char* gameLogText;
const char* gameLogCategory;

void __declspec(naked) GameLogHook()
{
	__asm {
		mov ecx, [esp+76]
		mov [gameLogTextPointer], ecx
		mov [gameLogCategoryPointer], eax
	}

	gameLogText = (const char*)gameLogTextPointer;
	gameLogCategory = (const char*)gameLogCategoryPointer;

	if (strlen(gameLogText) < 1024)
	{
		printf("INFO: %s\n", gameLogText);
	} else
	{
		printf("WARN: [%s] Long message (%d chars) was not logged\n", gameLogCategory, strlen(gameLogText));
	}

	fwrite(gameLogText, 1, strlen(gameLogText), logStream);

	__asm jmp[gameLogJumpOut];
}

DWORD WINAPI Init(LPVOID)
{
	Memory::Init();
	AllocConsole();
	freopen_s(&consoleStream, "CONOUT$", "w+", stdout);
	logStream = _fsopen("log.txt", "w", _SH_DENYWR);
	Memory::writeJMP(0x52bd56, (DWORD)GameLogHook);

	printf("INFO: nfsw-logger initialized\n");

	std::atexit([]()
	{
		FreeConsole();
		fclose(consoleStream);
		fclose(logStream);
	});

	return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(0, 0, Init, 0, 0, 0);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

