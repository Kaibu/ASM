#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <process.h>
#include <psapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SMALSTRINGSIZE 32
#define FUNCTIONSIZE 2048
#define OUTPUTSIZE 4096
#define PAGESIZE 4096
#define MAX_ARMA_INSTANCES 4
#define FILEMAPSIZE PAGESIZE * MAX_ARMA_INSTANCES


struct ARMA_SERVER_INFO 
{
	WORD	PID;
	WORD	OBJ_COUNT_0;
	WORD	OBJ_COUNT_1;
	WORD	OBJ_COUNT_2;
	WORD	PLAYER_COUNT;
	WORD	AI_LOC_COUNT;
	WORD	AI_REM_COUNT;
	WORD	SERVER_FPS;
	WORD	SERVER_FPSMIN;
	WORD	FSM_CE_FREQ;
	DWORD	MEM;
	char	MISSION[SMALSTRINGSIZE];
	char	PROFILE[SMALSTRINGSIZE];
	DWORD	TICK_COUNT;
};
