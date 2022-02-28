// NtleahLoader.cpp : ����Ӧ�ó������ڵ㡣
//
#include <Windows.h>
#include <tchar.h>
#include "NtleahLoader.h"


#pragma pack(1)
// 00   00 00 00 00   A4 03 00 00   11 04 00 00   E4 FD FF FF   64 00 00 00
typedef struct _NtLeahParameter
{
	BYTE zero1;			
	DWORD zero2;		
	DWORD pagecode1;	
	DWORD pagecode2;	
	DWORD param;		
	DWORD sleep_time;	
}NtLeahParameter;
#pragma pack()



int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	HANDLE hEvent;
	VOID *pVirtualMemory;
	HANDLE hMapFile;
	VOID *pMapFile;
	HANDLE hRemoteThread;
	DWORD ThreadId;
	NtLeahParameter NLParam;

	BOOL status;
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	wchar_t wszExePath[MAX_PATH], wszDllPath[MAX_PATH];


	hEvent = CreateEventA(NULL, FALSE, FALSE, "RcpEvent000");
	if (hEvent == INVALID_HANDLE_VALUE)
		return -1;

	hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4, "RcpFileMap000");
	if (hMapFile == INVALID_HANDLE_VALUE)
		return -1;

	pMapFile = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0x4);
	if (!pMapFile)
		return -1;


	GetCurrentDirectoryW(sizeof(wszExePath), wszExePath);
	wcscat(wszExePath, L"\\DllLoader.exe");

	si.cb = sizeof(STARTUPINFOW);
	GetStartupInfoW(&si);
	status = CreateProcessW
		(
			wszExePath,
			NULL,
			NULL,
			NULL,
			FALSE,
			CREATE_SUSPENDED,
			NULL,
			NULL,
			&si,
			&pi
		);
	if (!status)
	{
		MessageBoxA(NULL, "Ŀ¼�¿���û�� ��DllLoader.exe�� �ļ�", "Error", MB_OK);
		return -1;
	}
		

	BYTE oep_code[2];
	DWORD oep = 0x00401000; //��ͬ����һ����������ʱӲ���롣
	ReadProcessMemory(pi.hProcess, (LPCVOID)oep, oep_code, 2, NULL);

	BYTE jmp_code[] = {0xEB, 0xFE}; //���ر���ָ���ѭ��
	WriteProcessMemory(pi.hProcess, (LPVOID)oep, jmp_code, 2, NULL);
	FlushInstructionCache(pi.hProcess, (LPVOID)oep, 2);

	//ResumeThread(pi.hThread);

	pVirtualMemory = VirtualAllocEx(pi.hProcess, NULL, 0x1000, MEM_COMMIT, PAGE_READWRITE);
	if (!pVirtualMemory)
		goto ErrorExit;

	GetCurrentDirectoryW(sizeof(wszDllPath), wszDllPath);
	wcscat(wszDllPath, L"\\ntleah.dll");
	DWORD PathLen = (wcslen(wszDllPath) + 1)*sizeof(wchar_t);
	WriteProcessMemory(pi.hProcess, pVirtualMemory, wszDllPath, PathLen, NULL);

	NLParam.zero1 = 0;
	NLParam.zero2 = 0;
	NLParam.pagecode1 = 0x3A4;
	NLParam.pagecode2 = 0x411;
	NLParam.param = 0xFFFFFDE4;
	NLParam.sleep_time = 100;
	WriteProcessMemory(pi.hProcess, (BYTE*)pVirtualMemory + PathLen, &NLParam, sizeof(NtLeahParameter), NULL);

	*(DWORD*)pMapFile = (DWORD)pVirtualMemory;

	hRemoteThread = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, pVirtualMemory, 0, &ThreadId);
	if (!hRemoteThread)
		goto ErrorExit;

	HANDLE WaitEvent[2];
	WaitEvent[0] = hEvent;
	WaitEvent[1] = pi.hProcess;
	WaitForMultipleObjects(2, WaitEvent, FALSE, 3000);

	CloseHandle(hEvent);
	UnmapViewOfFile(pMapFile);
	CloseHandle(hMapFile);

	//SuspendThread(pi.hThread);
	WriteProcessMemory(pi.hProcess, (LPVOID)oep, oep_code, 2, NULL); //�ָ�ԭOEP��ָ��
	ResumeThread(pi.hThread);
	VirtualFreeEx(pi.hProcess, pVirtualMemory, 0x1000, MEM_RELEASE);

	return 0;



ErrorExit:
	MessageBoxA(NULL, "δ֪����!", "Error", MB_OK);
	TerminateProcess(pi.hProcess, 0);
	return -1;
}

