// PassKey.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include <Windows.h>
#include <string>
#include "detours.h"


/*
����֤�ķ���Ϊ�޸�Initialize.tjs�е�����һ����룬�� "PACKED" �����޸�Ϊ�����ַ���
Scripts.exec(@"@set (PACKED=${inXP3archivePacked})");

���³���˼·Ϊ����krkr2�������LoadLibraryA��������load��tpm�ļ�ʱ��ʹ��detours hook���ڴ�
�нű����ܺ������һ����䣬�ж��Ƿ�ΪInitialize.tjs���������������ķ����޸�PACKED

Ŀǰ���´��뻹���ܹ�tpm�ļ�����У�顣
������Ҫ�ֶ��޸�tpm��ȥ��smc����У�顣
*/


/*
1E02EC57   .  8BCB          MOV ECX, EBX
1E02EC59   .  55            PUSH EBP
1E02EC5A   .  E8 51D5FFFF   CALL <������_1.tjs_decrypt>;  eax = src
1E02EC5F   .  8B46 10       MOV EAX, DWORD PTR DS : [ESI + 10]
1E02EC62   .  2BC7          SUB EAX, EDI
1E02EC64   .  0346 04       ADD EAX, DWORD PTR DS : [ESI + 4]
1E02EC67   .  8BCB          MOV ECX, EBX
*/

DWORD p_after_decrypt = 0;

void WINAPI FixTjs(DWORD tjs_offset)
{
	static bool isFixed = false;

	//����ֻ�ܽ���һ���޸ģ�������bug
	if (!isFixed)
	{
		if (!strncmp((char*)tjs_offset, "// Initialize.tjs", 17))
		{
			memset((void*)(tjs_offset + 0x30B), 'KCUF', 4);
			isFixed = true;
		}
	}
}

__declspec(naked)void fix_tjs()
{
	__asm
	{
		pushad
		sub eax, edi
		push eax
		call FixTjs
		popad
		jmp p_after_decrypt
	}
}


typedef HMODULE (WINAPI *fnLoadLibraryA)(LPCSTR lpLibFileName);
fnLoadLibraryA pLoadLibraryA = LoadLibraryA;

HMODULE hdll;
HMODULE WINAPI newLoadLibraryA(LPCSTR lpLibFileName)
{
	static bool isHooked = false;
	std::string str(lpLibFileName);
	if ((str.find("tpm") != std::string::npos) && (!isHooked))
	{
		hdll = pLoadLibraryA(lpLibFileName);
		
		if (hdll)
		{
			p_after_decrypt = (DWORD)hdll + 0x2EC5F;

			DetourTransactionBegin();
			DetourAttach((void**)&p_after_decrypt, fix_tjs);
			DetourTransactionCommit();

			isHooked = true;
		}
		return hdll;
	}
	
	return pLoadLibraryA(lpLibFileName);
}

PVOID g_pOldCreateFontIndirectA = CreateFontIndirectA;
typedef int (WINAPI *PfuncCreateFontIndirectA)(LOGFONTA *lplf);
int WINAPI NewCreateFontIndirectA(LOGFONTA *lplf)
{
	lplf->lfCharSet = GB2312_CHARSET;
	//lplf->lfHeight = 100;
	//strcpy(lplf->lfFaceName, "My����");
	//lplf->lfCharSet = GB2312_CHARSET;

	return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}


PVOID g_pOldCreateFontIndirectW = CreateFontIndirectW;
typedef int (WINAPI *PfuncCreateFontIndirectW)(LOGFONTW *lplf);
int WINAPI NewCreateFontIndirectW(LOGFONTW *lplf)
{
	lplf->lfCharSet = GB2312_CHARSET;
	//lplf->lfHeight = 100;
	//strcpy(lplf->lfFaceName, "My����");
	//lplf->lfCharSet = GB2312_CHARSET;

	return ((PfuncCreateFontIndirectW)g_pOldCreateFontIndirectW)(lplf);
}

void SetHook()
{
	DetourTransactionBegin();
	DetourAttach((void**)&pLoadLibraryA, newLoadLibraryA);
	DetourTransactionCommit();


	DetourTransactionBegin();
	//DetourUpdateThread(GetCurrentThread());
	DetourAttach(&g_pOldCreateFontIndirectA, NewCreateFontIndirectA);
	DetourTransactionCommit();

	DetourTransactionBegin();
	//DetourUpdateThread(GetCurrentThread());
	DetourAttach(&g_pOldCreateFontIndirectW, NewCreateFontIndirectW);
	DetourTransactionCommit();
}

 __declspec(dllexport) void Dummy()
{

}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		SetHook();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


