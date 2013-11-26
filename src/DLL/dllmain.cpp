#include "ASM.h"

HANDLE FileMapHandle;
LPVOID FileMap = NULL;
HANDLE hProcess;

DWORD InstanceID;
ARMA_SERVER_INFO *ArmaServerInfo = NULL;
LARGE_INTEGER PCF, PCS;
char options[SMALSTRINGSIZE] = "";

void Init()
{
	SECURITY_ATTRIBUTES FSA;
	SECURITY_DESCRIPTOR FSD;

	InitializeSecurityDescriptor(&FSD, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&FSD, TRUE, NULL, FALSE);

	FSA.nLength = sizeof(FSA);
	FSA.lpSecurityDescriptor = &FSD;
	FSA.bInheritHandle = TRUE;

	FileMapHandle = CreateFileMapping(INVALID_HANDLE_VALUE, &FSA, PAGE_READWRITE, 0, FILEMAPSIZE, "Global\\ASM_MapFile"); // for DS started as service
	
	if (FileMapHandle == NULL) FileMapHandle = CreateFileMapping(INVALID_HANDLE_VALUE, &FSA, PAGE_READWRITE, 0, FILEMAPSIZE, "ASM_MapFile"); 	
	
	if (FileMapHandle != NULL) FileMap = MapViewOfFile(FileMapHandle, FILE_MAP_WRITE, 0, 0, FILEMAPSIZE);

	hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId() );

	QueryPerformanceFrequency(&PCF);
	QueryPerformanceCounter(&PCS);

	GetPrivateProfileString("ASM", "objectcountinterval", "0", &options[0], SMALSTRINGSIZE - 1, ".\\asm.ini"); 

}

void Finit()
{	
	if (FileMap) {	
		memset(ArmaServerInfo, 0, PAGESIZE);
		FlushViewOfFile(FileMap, FILEMAPSIZE);
	}

	CloseHandle(hProcess);

	UnmapViewOfFile(FileMap);
	CloseHandle(FileMapHandle);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Init();
	break;
	case DLL_THREAD_ATTACH:
	break;
	case DLL_THREAD_DETACH: 
	break;
	case DLL_PROCESS_DETACH:
		Finit();
	break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) void __stdcall RVExtension(char *output, int outputSize, const char *function)
{
	char   *stopstring;

	switch(*function)
	{

	case '0':// FPS update 
		{
			if (FileMap) {
				unsigned FPS,FPSMIN;
				FPS =	strtol(&function[2], &stopstring, 10);			
				FPSMIN = strtol(&stopstring[1], &stopstring, 10);
				ArmaServerInfo->SERVER_FPS =	FPS;
				ArmaServerInfo->SERVER_FPSMIN =	FPSMIN;
				FlushViewOfFile(ArmaServerInfo, PAGESIZE);
			}
			return;
		}

	case '1':// CPS update 
		{
			if (FileMap) {
				LARGE_INTEGER PCE;
				double tnsec;
				unsigned conditionNo;

				QueryPerformanceCounter(&PCE);
				tnsec = (double)(PCE.QuadPart - PCS.QuadPart) / (double)PCF.QuadPart;

				conditionNo =	strtol(&function[2], &stopstring, 10);			
				ArmaServerInfo->FSM_CE_FREQ = conditionNo * 1000 / tnsec;
			
				PCS = PCE;
			}
			return;
		}

	case '2':// GEN update 
		{
			if (FileMap) {
				unsigned int players, ail, air;

				players = strtol(&function[2], &stopstring, 10);			
				ail		= strtol(&stopstring[1], &stopstring, 10);
				air		= strtol(&stopstring[1], &stopstring, 10);
				ArmaServerInfo->PLAYER_COUNT	= players;
				ArmaServerInfo->AI_LOC_COUNT	= ail;
				ArmaServerInfo->AI_REM_COUNT	= air;
				
				PROCESS_MEMORY_COUNTERS pmc;
				if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) ) {
					ArmaServerInfo->MEM = pmc.PagefileUsage;
				}
			}
			return;
		}

	case '3':// MISSION update 
		{
			if (FileMap) {
				memcpy(ArmaServerInfo->MISSION, &function[2], SMALSTRINGSIZE-1);
				ArmaServerInfo->MISSION[SMALSTRINGSIZE-1] = 0;
			}
			return;
		}

	case '4':// OBJECTCOUNT update 
		{
			if (FileMap) {
				unsigned int objs;
				objs = strtol(&function[2], &stopstring, 10);			
				ArmaServerInfo->OBJ_COUNT = objs;
			}
			return;
		}

	case '9':// init
		{
			if (FileMap) {
				if (ArmaServerInfo == NULL) {
					for (InstanceID = 0 ; InstanceID < MAX_ARMA_INSTANCES ; InstanceID++) {
						ArmaServerInfo = (ARMA_SERVER_INFO*)((DWORD)FileMap + (InstanceID * PAGESIZE));
						if(ArmaServerInfo->PID == 0) {
							ArmaServerInfo->PID = GetCurrentProcessId();
							memcpy(ArmaServerInfo->PROFILE, &function[2], SMALSTRINGSIZE-1);
							ArmaServerInfo->PROFILE[SMALSTRINGSIZE-1] = 0;
							break;
						}
					}
					if (InstanceID < MAX_ARMA_INSTANCES) ArmaServerInfo->MEM = 0;
				} else { ArmaServerInfo->MEM = 0;}
				FlushViewOfFile(ArmaServerInfo, PAGESIZE);
			}
			sprintf_s( output, SMALSTRINGSIZE, "%s", options);
			return;
		}

	default:
		{
			return;
		};
	};
}
