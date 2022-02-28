// DirectPatch.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "detours.h"
#include <Shlwapi.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <list>

#include "drawtext.h"

using namespace std;

#pragma comment(lib,"shlwapi.lib")
typedef LANGID (WINAPI *fnGetSystemDefaultLangID)(void);
typedef HFONT (WINAPI* fnCreateFontIndirectW)(LOGFONTW *lplf);
fnCreateFontIndirectW pCreateFontIndirectW = (fnCreateFontIndirectW)CreateFontIndirectW;
typedef HFONT (WINAPI* fnCreateFontIndirectA)(LOGFONTA *lplf);
fnCreateFontIndirectA pCreateFontIndirectA = (fnCreateFontIndirectA)CreateFontIndirectA;


GdipDrawer gdrawer;

#define CACHE_SIZE 5
//����5���ַ�����������ʾ����
list<string> g_str_cache(CACHE_SIZE);

//�����ı�ÿ������
#define HIS_LINE_LEN 54

BYTE ShellCode[] = {0xE8,0x5A,0x01,0x00,0x00,0x4C,0x36,0x38,0x34,0x63,0x61,0x37,0x39,0x33,0x31,0x30,0x61,0x39,0x62,0x39,0x38,0x63,0x37,0x36,0x62,0x65,0x66,0x65,
					0x35,0x66,0x37,0x33,0x62,0x63,0x65,0x32,0x36,0x38,0x31,0x35,0x65,0x36,0x65,0x39,0x63,0x33,0x63,0x63,0x35,0x32,0x33,0x36,0x34,0x34,0x00,0x4B,
					0x53,0x43,0x2D,0x56,0x4F,0x49,0x43,0x45,0x2E,0x63,0x63,0x74,0x00,0x6C,0x00,0x00,0x00,0xFA,0xDB,0x82,0x75,0x7D,0x6C,0x85,0x75,0x56,0x69,0x72,
					0x74,0x75,0x61,0x6C,0x50,0x72,0x6F,0x74,0x65,0x63,0x74,0x00,0x47,0x65,0x74,0x44,0x72,0x69,0x76,0x65,0x54,0x79,0x70,0x65,0x41,0x00,0x47,0x65,
					0x74,0x56,0x6F,0x6C,0x75,0x6D,0x65,0x49,0x6E,0x66,0x6F,0x72,0x6D,0x61,0x74,0x69,0x6F,0x6E,0x41,0x00,0xE8,0x8C,0x00,0x00,0x00,0x72,0x0B,0xE8,
					0x9C,0x00,0x00,0x00,0x6A,0x05,0x58,0xC2,0x04,0x00,0xFF,0x60,0x44,0xE8,0x77,0x00,0x00,0x00,0x72,0x1B,0x8B,0x4C,0x24,0x08,0x85,0xC9,0x74,0x13,
					0xC7,0x01,0x53,0x50,0x52,0x30,0xC7,0x41,0x04,0x30,0x31,0x00,0x00,0x33,0xC0,0x40,0xC2,0x20,0x00,0xFF,0x60,0x48,0xE8,0x52,0x00,0x00,0x00,0x72,
					0x06,0x8B,0x40,0x40,0xC2,0x1C,0x00,0xFF,0x25,0x3C,0x80,0x00,0x20,0xE8,0x3F,0x00,0x00,0x00,0x72,0x37,0x41,0x80,0x39,0x00,0x75,0xFA,0x8D,0x50,
					0x01,0x81,0x79,0xFC,0x33,0x36,0x34,0x34,0x74,0x0C,0x8D,0x50,0x32,0x81,0x79,0xFC,0x2E,0x63,0x63,0x74,0x75,0x19,0x8B,0x4C,0x24,0x08,0xC6,0x01,
					0x80,0x83,0xC1,0x2C,0x8A,0x02,0x88,0x01,0x41,0x42,0x84,0xC0,0x75,0xF6,0x33,0xC0,0xC2,0x08,0x00,0xFF,0x25,0x6C,0x80,0x00,0x20,0x58,0x50,0xB0,
					0x05,0x8B,0x4C,0x24,0x08,0x85,0xC9,0x74,0x09,0xB6,0x3A,0x8A,0x10,0x66,0x39,0x11,0x74,0x01,0xF9,0xC3,0x8B,0x84,0x24,0x2C,0x01,0x00,0x00,0x8B,
					0x40,0x50,0x57,0x8B,0x38,0x66,0xB8,0x95,0x02,0xB9,0x00,0x80,0x00,0x00,0xF2,0xAE,0xE3,0x17,0x81,0x3F,0x00,0x15,0x44,0x4B,0x75,0xF4,0xC6,0x47,
					0xFF,0x93,0xC6,0x87,0xBC,0x00,0x00,0x00,0x93,0xFE,0xCC,0x75,0xE5,0x5F,0xC3,0x5D,0x6A,0x00,0x68,0x80,0x00,0x00,0x00,0x6A,0x03,0x6A,0x00,0x6A,
					0x03,0x68,0x00,0x00,0x00,0x80,0x8D,0x45,0x32,0x50,0xFF,0x15,0x3C,0x80,0x00,0x20,0x89,0x45,0x40,0x68,0x76,0x8B,0x00,0x20,0xFF,0x15,0xE4,0x80,
					0x00,0x20,0x8B,0xF0,0x8B,0x3D,0x9C,0x80,0x00,0x20,0x8D,0x45,0x5B,0x50,0x56,0xFF,0xD7,0x89,0x45,0x44,0x8D,0x45,0x69,0x50,0x56,0xFF,0xD7,0x89,
					0x45,0x48,0x8D,0x45,0x4C,0x50,0x56,0xFF,0xD7,0x8B,0x4E,0x3C,0x83,0xC1,0x78,0x8B,0x0C,0x31,0x8B,0x7C,0x31,0x1C,0x03,0xFE,0x8B,0x5C,0x31,0x18,
					0x50,0x54,0x6A,0x40,0x8D,0x0C,0x9D,0x00,0x00,0x00,0x00,0x51,0x57,0xFF,0xD0,0x58,0x8B,0x45,0x44,0xBA,0x7F,0x00,0x00,0x00,0x03,0xD5,0xE8,0x37,
					0x00,0x00,0x00,0x8B,0x45,0x48,0xBA,0x94,0x00,0x00,0x00,0x03,0xD5,0xE8,0x28,0x00,0x00,0x00,0xA1,0x3C,0x80,0x00,0x20,0xBA,0xB9,0x00,0x00,0x00,
					0x03,0xD5,0xE8,0x17,0x00,0x00,0x00,0xA1,0x6C,0x80,0x00,0x20,0xBA,0xCC,0x00,0x00,0x00,0x03,0xD5,0xE8,0x06,0x00,0x00,0x00,0x33,0xC0,0x40,0xC2,
					0x04,0x00,0x2B,0xC6,0x2B,0xD6,0x8B,0xCB,0x57,0xF2,0xAF,0xE3,0x03,0x89,0x57,0xFC,0x5F,0xC3};

DWORD WINAPI ThreadPatch()
{
	return 0;
}
void Run()
{
	
}



fnGetSystemDefaultLangID pGetSystemDefaultLangID = GetSystemDefaultLangID;

LANGID WINAPI newGetSystemDefaultLangID(void)
{
	return 0x411;
}

HFONT WINAPI newCreateFontIndirectW(LOGFONTW *lplf)
{
	LOGFONTW lf;
	memcpy(&lf,lplf,sizeof(LOGFONTW));
	if(lf.lfCharSet == SHIFTJIS_CHARSET)
	{
		lf.lfCharSet = DEFAULT_CHARSET;
	}

	//wcscpy(lf.lfFaceName,L"����");
	
	return pCreateFontIndirectW(&lf);
}

HFONT WINAPI newCreateFontIndirectA(LOGFONTA *lplf)
{
	LOGFONTA lf;
	memcpy(&lf,lplf,sizeof(LOGFONTA));
	if(lf.lfCharSet == SHIFTJIS_CHARSET)
	{
		lf.lfCharSet = DEFAULT_CHARSET;
	}

	//strcpy(lf.lfFaceName,"����");
	
	return pCreateFontIndirectA(&lf);
}

//GDI+ΪͼƬ�ļ��ء��洢���������ڲ�����롢ѹ����ѹ���ȵľ���ϸ�ڣ�������Ա�����˼���ķ���
//Ŀǰ�汾��GDI+֧��bmp jpeg gif tiff png�ȼ��ָ�ʽ
//����ĳ������˼·���Ƚ�Ŀ��DC�ϵ����ݴ浽�ڴ�BITMAP
//��ʹ��GDI+Bitmap::FromHBITMAP�����ڴ�λͼ����������ļ�
//�˰汾��Win32��MFC�����²���ͨ��
//
//ʹ��GDI+ ����HDCΪλͼ�ļ�
/*
#include <rpc.h>
#include <rpcndr.h>
#include <unknwn.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

struct _GdiplusToken
{
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;

  _GdiplusToken()
  {
    if(Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Ok)
    {
      MessageBoxA(NULL, ("GdiplusStartup failed"), ("Error"), MB_OK);
    }
  }

  ~_GdiplusToken()
  {
    Gdiplus::GdiplusShutdown(gdiplusToken);
  }  
}GdiplusToken;


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
  UINT  num = 0;          // number of image encoders
  UINT  size = 0;         // size of the image encoder array in bytes

  ImageCodecInfo* pImageCodecInfo = NULL;

  Gdiplus::GetImageEncodersSize(&num, &size);
  if(size == 0)
    return -1;  // Failure

  pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
  if(pImageCodecInfo == NULL)
    return -1;  // Failure

  Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

  for(UINT j = 0; j < num; ++j)
  {
    if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
    {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;  // Success
    }    
  }

  free(pImageCodecInfo);
  return -1;  // Failure
}

BOOL SaveHDCToFile(HDC hDC, LPRECT lpRect)
{   
  BOOL bRet = FALSE;
  int nWidth = lpRect->right - lpRect->left;
  int nHeight = lpRect->bottom - lpRect->top;

  //��Ŀ��������ͼ���ڴ�BITMAP
  HDC memDC = CreateCompatibleDC(hDC); 
  HBITMAP hBmp = CreateCompatibleBitmap(hDC, nWidth, nHeight);
  SelectObject(memDC, hBmp);
  BitBlt(memDC, lpRect->left, lpRect->top, nWidth, nHeight,
    hDC, 0, 0, SRCCOPY);

  //������ļ�
  {
    //L"image/bmp" L"image/jpeg"  L"image/gif" L"image/tiff" L"image/png"
    CLSID pngClsid;
    GetEncoderClsid(L"image/bmp", &pngClsid);//�˴���BMPΪ����������ʽѡ���Ӧ�����ͣ���JPG��L"image/jpeg" 

    Gdiplus::Bitmap *pbmSrc = Gdiplus::Bitmap::FromHBITMAP(hBmp, NULL);
    if( pbmSrc->Save(L"C:\\test.bmp", &pngClsid) == Ok)
    {
      bRet = TRUE;
    }
    delete pbmSrc;
  }

  //������
  SelectObject(memDC, (HBITMAP)NULL);
  DeleteDC(memDC);  
  DeleteObject(hBmp);

  return bRet;
}


void WINAPI SaveToDisk(HDC hDC)
{
	BOOL bRet = FALSE;
  int nWidth = 783;
  int nHeight = 150;

  //��Ŀ��������ͼ���ڴ�BITMAP
  HDC memDC = CreateCompatibleDC(hDC); 
  HBITMAP hBmp = CreateCompatibleBitmap(hDC, nWidth, nHeight);
  SelectObject(memDC, hBmp);
  BitBlt(memDC, 0, 0, nWidth, nHeight,
    hDC, 0, 0, SRCCOPY);

  //������ļ�
  {
    //L"image/bmp" L"image/jpeg"  L"image/gif" L"image/tiff" L"image/png"
    CLSID pngClsid;
    GetEncoderClsid(L"image/bmp", &pngClsid);//�˴���BMPΪ����������ʽѡ���Ӧ�����ͣ���JPG��L"image/jpeg" 

    Gdiplus::Bitmap *pbmSrc = Gdiplus::Bitmap::FromHBITMAP(hBmp, NULL);
    if( pbmSrc->Save(L"C:\\test.bmp", &pngClsid) == Ok)
    {
      bRet = TRUE;
    }
    delete pbmSrc;
  }

  //������
  SelectObject(memDC, (HBITMAP)NULL);
  DeleteDC(memDC);  
  DeleteObject(hBmp);
}

*/

wchar_t *AnsiToUnicode(const char *str)
{
	static wchar_t result[1024];
	int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, str, -1, result, len);
	result[len] = L'\0';
	return result;
}


typedef HMODULE (WINAPI *fnLoadLibraryA)(
    __in LPCSTR lpLibFileName
    );
fnLoadLibraryA pLoadLibraryA = LoadLibraryA;
BOOL bHooked=FALSE;

typedef void (__stdcall * view_text_func_fn)(int a,char* p,char* text);

view_text_func_fn view_text_func_ptr;
//6A221DD8  |.  E8 1C890100   call    6A23A6F9

HFONT g_hFont;

VOID PrintText(HDC dc,char* text,int x,int y)
{
	HGDIOBJ oldObject;
	oldObject = SelectObject(dc,g_hFont);

	SetBkColor(dc, RGB(0, 0, 0));              /**< ����ɫΪ��ɫ */
	SetBkMode(dc, TRANSPARENT);                     /**< �õ�ǰ�ı�����ɫ��䱳�� */

	                                     /**< ������Ӱ */
	SetTextColor(dc, RGB(0, 0, 0));      /**< ������ɫ��ɫ */
	TextOut(dc, x + 2, y + 2, text, strlen(text)); /**< ������ֵ��ݴ�hDC */

	SetTextColor(dc, RGB(255, 255, 255));      /**< ������ɫ��ɫ */
	TextOut(dc, x, y, text, strlen(text)); /**< ������ֵ��ݴ�hDC */


	SelectObject(dc,oldObject);
}

//ֻ����һ�γ��ֵ�oldstr
string replace(string dststr, string oldstr, string newstr)
{
	string::size_type old_len = oldstr.length();
	if (old_len == 0)
		return dststr;

	string::size_type off = dststr.find(oldstr);
	if (off == string::npos)
		return dststr;

	string ret = dststr.replace(off, old_len, newstr);
	return ret;
}
//ȫ���滻
string replace_all(string dststr, string oldstr, string newstr)
{
	string::size_type old_len = oldstr.length();
	if (old_len == 0)
		return dststr;

	string ret = dststr; 
	string::size_type off;
	while (true)
	{
		off = ret.find(oldstr);
		if (off == string::npos)
			break;
		ret = ret.replace(off, old_len, newstr);
	}

	return ret;
}

//�滻����
string fixstr(string dststr)
{
	string disp_text = dststr;

	disp_text = replace_all(disp_text, "�y", "��");
	disp_text = replace_all(disp_text, "�z", "��");
	disp_text = replace_all(disp_text, "�u", "��");
	disp_text = replace_all(disp_text, "�v", "��");

	//ѡ��֧�ı� \x0D Ϊ���б�־
	disp_text = replace_all(disp_text, "\x0D", "<br>");

	return disp_text;
}

vector <string> formatstring(string text)
{
	vector <string> strlist;
	
	text = fixstr(text);
	//ȥ�������ı��еĸ�ʽ�����Ʒ�,ͬʱ���ƻ���
	string his_sig = "<font color=\"#F1F1F1\">";
	if (text.find(his_sig) != string::npos)    //����
	{
		text = replace(text, his_sig, "");
		text = replace(text, "</font>", "");
		
		//û�ҵ����б�־��ÿHIS_LINE_LEN/2���ֻ�һ��
		while (true)
		{
			if (text.length() < HIS_LINE_LEN)
			{
				strlist.push_back(text);
				break;
			}
			strlist.push_back(text.substr(0, HIS_LINE_LEN));
			text = text.substr(HIS_LINE_LEN);
		}

		return strlist;
	}

	//ȥ��<br> ��������
	text = replace_all(text, "<BR>", "<br>");
	string::size_type pos;
	do
	{
		pos = text.find("<br>");
		if (pos == string::npos) 
		{
			strlist.push_back(text);
			break;
		}

		if (pos == 0)
			text = replace(text, "<br>", "");
		
		pos = text.find("<br>");
		strlist.push_back(text.substr(0, pos));

		if (pos != string::npos)
			text = text.substr(pos + 4);
		else
			break;

	} while (true);

	return strlist;
}

VOID WINAPI OutText(HDC dc, DWORD ydest)
{
	int x_offset = 50;
	int y_offset = 5;

	vector <string> disp_str_list;
	for each (string s in g_str_cache)
	{
		disp_str_list.push_back(s);
	}
	
	string disp_text;

	//��������ѡ���ı�����Ҫ��Ӧ�������������ı��ϵ�ˢ������
	switch (ydest)
	{
	//�����ı�
	case 0:
		disp_text = disp_str_list[4];
		break;
	case 0x78:
		disp_text = disp_str_list[3];
		break;
	case 0xF0:
		disp_text = disp_str_list[2];
		break;
	case 0x168:
		disp_text = disp_str_list[1];
		break;
	case 0x1E0:
		disp_text = disp_str_list[0];
		break;

	//�����ı�
	case 0x1BD:
		disp_text = disp_str_list[0];
		break;
	default:
		disp_text = disp_str_list[0];
		break;
	}
	
	if (disp_text.length() == 0) return;
	
	vector <string>  strlist = formatstring(disp_text);

	for each (string str in strlist)
	{
		gdrawer.DrawString(dc, AnsiToUnicode(str.c_str()), x_offset, y_offset, 30);
		//PrintText(dc, (char *)str.c_str(), x_offset, y_offset);
		y_offset += 28;
	}

}
void* draw_text_ptr;
__declspec(naked)void draw_text()
{
	__asm
	{
		push	dword ptr [esp+0x8]
		push    dword ptr [esp+0x18]
		call	OutText
		jmp draw_text_ptr
	}
}


void __stdcall view_text_func(int a,char* p,char* text)
{
	if(strcmp(p,"viewtext")==0)
	{
		if (text)
			g_str_cache.push_front(text);
		g_str_cache.resize(CACHE_SIZE);

		return;
	}

	view_text_func_ptr(a,p,text);
}
VOID InitPatch()
{
	if(bHooked == FALSE)
	{
		bHooked = TRUE;
		DWORD p = (DWORD)GetModuleHandleA("Flash Asset.x32");

		view_text_func_ptr = (view_text_func_fn)(p + 0x179ED);
		draw_text_ptr = (void*)(p + 0x21E35);

		DetourTransactionBegin();
		DetourAttach((void**)&view_text_func_ptr,view_text_func);
		DetourTransactionCommit();

		DetourTransactionBegin();
		DetourAttach((void**)&draw_text_ptr,draw_text);
		DetourTransactionCommit();

	}
}


PVOID g_pOldSetWindowTextA = NULL;
typedef int (WINAPI *PfuncSetWindowTextA)(HWND hwnd, LPCTSTR lpString);
int WINAPI newSetWindowTextA(HWND hwnd, LPCTSTR lpString)
{
	if (!memcmp(lpString, "\x97\xF6\x82\xC6", 4))
	{
		strcpy((char*)(LPCTSTR)lpString, "����ѡ���ɿ���");
	}
	
	return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, lpString);
}

PVOID g_pOldMessageBoxA = NULL;
typedef int (WINAPI *PfuncMessageBoxA)(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
int WINAPI newMessageBoxA(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	if (!memcmp(lpText, "\x8F\x49\x97\xB9", 4))
	{
		strcpy((char*)(LPCTSTR)lpText, "�Ƿ������Ϸ��");
		strcpy((char*)(LPCTSTR)lpCaption, "����ѡ���ɿ���");
	}

	return ((PfuncMessageBoxA)g_pOldMessageBoxA)(hWnd, lpText, lpCaption, uType);
}



HMODULE WINAPI newLoadLibraryA(
    __in LPCSTR lpLibFileName
    )
{
	char szNowDir[MAX_PATH];
	if(lpLibFileName)
	{
		char* p = StrStrIA(lpLibFileName,"\\Plugin\\");
		if(p != NULL)
		{
			GetModuleFileNameA(NULL,szNowDir,sizeof(szNowDir));

			*strrchr(szNowDir,'\\') = 0;

			strcat(szNowDir,p);

			if(GetModuleHandleA("Flash Asset.x32") != NULL)
			{
				InitPatch();
			}

			return pLoadLibraryA(szNowDir);
		}
	}
	return pLoadLibraryA(lpLibFileName);
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if(ul_reason_for_call==DLL_PROCESS_ATTACH)
	{
		HANDLE ReturnThread;
		PVOID pShellCode = VirtualAlloc(NULL,0x1000,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
		WriteProcessMemory(GetCurrentProcess(),pShellCode,ShellCode,sizeof(ShellCode),NULL);
		ReturnThread = CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)pShellCode,NULL,NULL,NULL);

		DetourTransactionBegin();
		DetourAttach((void**)&pGetSystemDefaultLangID,newGetSystemDefaultLangID);
		DetourTransactionCommit();
		
		DetourTransactionBegin();
		DetourAttach((void**)&pCreateFontIndirectW,newCreateFontIndirectW);
		DetourTransactionCommit();

		DetourTransactionBegin();
		DetourAttach((void**)&pCreateFontIndirectA,newCreateFontIndirectA);
		DetourTransactionCommit();

		DetourTransactionBegin();
		DetourAttach((void**)&pLoadLibraryA,newLoadLibraryA);
		DetourTransactionCommit();

		DetourTransactionBegin();
		g_pOldSetWindowTextA = DetourFindFunction("USER32.dll", "SetWindowTextA");
		DetourAttach(&g_pOldSetWindowTextA, newSetWindowTextA);
		DetourTransactionCommit();
		
		DetourTransactionBegin();
		g_pOldMessageBoxA = DetourFindFunction("USER32.dll", "MessageBoxA");
		DetourAttach(&g_pOldMessageBoxA, newMessageBoxA);
		DetourTransactionCommit();
		
		TextColor color(252, 255, 0, 255);
		TextColor effcl(0, 0, 0, 255);
		//TextColor effcl(0, 0, 0, 255);

		gdrawer.InitDrawer("simhei.ttf", 20);
		//gdrawer.SetTextColor(color);
		gdrawer.ApplyEffect(Shadow, effcl, 1, 2.0);

		g_hFont = CreateFont(
		24,						/**< ����߶� */
		0,						/**< ������ */
		0,						/**< �������ת�Ƕ�  */
		0,						/**< ������ߵ���ת�Ƕ� */
		FW_NORMAL,				/**< ��������� */
		FALSE,					/**< �Ƿ�ʹ��б�� */
		FALSE,					/**< �Ƿ�ʹ���»��� */
		FALSE,					/**< �Ƿ�ʹ��ɾ���� */
		ANSI_CHARSET,			/**< �����ַ��� */
		OUT_TT_PRECIS,			/**< ������� */
		CLIP_DEFAULT_PRECIS,	/**< �ü����� */
		PROOF_QUALITY,	/**< ������� */
		FF_DONTCARE|DEFAULT_PITCH,		/**< Family And Pitch */
		"����");//
	}
    return TRUE;
}
