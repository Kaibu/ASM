#include "ASM.h"

HANDLE FileMapHandle;
LPVOID FileMap = NULL;
HANDLE hProcess;

DWORD InstanceID;
ARMA_SERVER_INFO *ArmaServerInfo = NULL;
LARGE_INTEGER PCF, PCS;
char OCI0[SMALSTRINGSIZE] = "0";
char OCI1[SMALSTRINGSIZE] = "0";
char OCI2[SMALSTRINGSIZE] = "0";
char OCC0[FUNCTIONSIZE] = "";
char OCC1[FUNCTIONSIZE] = "";
char OCC2[FUNCTIONSIZE] = "";


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

	GetPrivateProfileString("ASM", "objectcountinterval0", "0", &OCI0[0], SMALSTRINGSIZE - 1, ".\\asm.ini"); 
	GetPrivateProfileString("ASM", "objectcountinterval1", "0", &OCI1[0], SMALSTRINGSIZE - 1, ".\\asm.ini"); 
	GetPrivateProfileString("ASM", "objectcountinterval2", "0", &OCI2[0], SMALSTRINGSIZE - 1, ".\\asm.ini"); 
	GetPrivateProfileString("ASM", "objectcountcommand0", "", &OCC0[0], FUNCTIONSIZE - 1, ".\\asm.ini"); 
	GetPrivateProfileString("ASM", "objectcountcommand1", "", &OCC1[0], FUNCTIONSIZE - 1, ".\\asm.ini"); 
	GetPrivateProfileString("ASM", "objectcountcommand2", "", &OCC2[0], FUNCTIONSIZE - 1, ".\\asm.ini");
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
				ArmaServerInfo->TICK_COUNT = GetTickCount();
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
				ArmaServerInfo->FSM_CE_FREQ = static_cast<int>(floor(conditionNo * 1000 / tnsec + 0.5));
			
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
				strcpy_s(ArmaServerInfo->MISSION, SMALSTRINGSIZE, &function[2]);
			}
			return;
		}

	case '4':// OBJ_COUNT_0 update 
		{
			if (FileMap) {
				unsigned obj;
				obj = strtol(&function[2], &stopstring, 10);			
				ArmaServerInfo->OBJ_COUNT_0 = obj;
			}
			return;
		}

	case '5':// OBJ_COUNT_1 update 
		{
			if (FileMap) {
				unsigned obj;
				obj = strtol(&function[2], &stopstring, 10);			
				ArmaServerInfo->OBJ_COUNT_1 = obj;
			}
			return;
		}

	case '6':// OBJ_COUNT_2 update 
		{
			if (FileMap) {
				unsigned obj;
				obj = strtol(&function[2], &stopstring, 10);			
				ArmaServerInfo->OBJ_COUNT_2 = obj;
			}
			return;
		}

	case '9':// init
		{
			if (FileMap) {
				if (ArmaServerInfo == NULL) {
					DWORD DeadTimeout = GetTickCount() - 10000;
					for (InstanceID = 0 ; InstanceID < MAX_ARMA_INSTANCES ; InstanceID++) {
						ArmaServerInfo = (ARMA_SERVER_INFO*)((DWORD)FileMap + (InstanceID * PAGESIZE));
						if((ArmaServerInfo->PID == 0) || (ArmaServerInfo->TICK_COUNT < DeadTimeout)) {
							ArmaServerInfo->TICK_COUNT = DeadTimeout + 10000;
							ArmaServerInfo->PID = GetCurrentProcessId();
							strcpy_s(ArmaServerInfo->PROFILE, SMALSTRINGSIZE, &function[2]);
							break;
						}
					}
					if (InstanceID < MAX_ARMA_INSTANCES) ArmaServerInfo->MEM = 0;
				} else { ArmaServerInfo->MEM = 0;}
				FlushViewOfFile(ArmaServerInfo, PAGESIZE);
			}
			sprintf_s(output, OUTPUTSIZE, "_ASM_OPT=[%s,%s,%s,\"%s\",\"%s\",\"%s\"];", OCI0, OCI1, OCI2, OCC0, OCC1, OCC2);
			return;
		}

	default:
		{
			return;
		};
	};
}
