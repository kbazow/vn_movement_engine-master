// CQ_Text.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "CQ_Text.h"

int wmain(int argc, wchar_t* argv[])
{
    try
    {
        // �������
        if(argc < 2)
        {
            fputs("CQ_Text.exe <input>", stderr);
            throw -1;
        }
        //////////////////////////
        // ��ȡ�ļ�����
        FILE* fp = 0;
        _wfopen_s(&fp, argv[1], L"rb");
        if(!fp) throw 2;
        fseek(fp, 0, SEEK_END);
        const long fl = ftell(fp);
        unsigned char* buf = new unsigned char[MEMPRTCT(fl)];
        rewind(fp);
        fread_s(buf, MEMPRTCT(fl), sizeof(char), fl, fp);
        fclose(fp);
        decode(buf, 0, key_r, fl);
        /////////////////////////
        // �����ļ���
        wchar_t* flag = wcsrchr(argv[1], L'.');
        if(!flag)
        {
            flag = argv[1];
            wcscat_s(argv[1], wcslen(L"_dir"), L"_dir");
        }
        else
        {
            *flag = 0;
        }
        std::wstring dir_name = argv[1];
        CreateDirectoryW(dir_name.c_str(), 0);
        dir_name += L"\\";
        /////////////////////////
        // ����ļ�����
        process(buf, fl, dir_name);
        /////////////////////////
        // �˳�����
        delete [] buf;
        return 0;
    } 
    catch(int& e) 
    {
        return e;
    }
    catch(...)
    {
        return -3;
    }
}
