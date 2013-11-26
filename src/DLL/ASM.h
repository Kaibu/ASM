#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <process.h>
#include <psapi.h>
#include <stdio.h>
#include <stdlib.h>

#define SMALSTRINGSIZE 32
#define FUNCTIONSIZE 2048
#define OUTPUTSIZE 4096
#define PAGESIZE 4096
#define MAX_ARMA_INSTANCES 4
#define FILEMAPSIZE PAGESIZE * MAX_ARMA_INSTANCES

struct ARMA_SERVER_INFO 
{
	DWORD	PID;
	DWORD	OBJ_COUNT;
	DWORD	MEM;
	DWORD	PLAYER_COUNT;
	DWORD	AI_LOC_COUNT;
	DWORD	AI_REM_COUNT;
	DWORD	SERVER_FPS;
	DWORD	SERVER_FPSMIN;
	DWORD	FSM_CE_FREQ;
	char	MISSION[SMALSTRINGSIZE];
	char	PROFILE[SMALSTRINGSIZE];
};
