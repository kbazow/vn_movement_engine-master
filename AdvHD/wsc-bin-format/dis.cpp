
#include "pch.h"
#include "usr.h"

int dissociate_text(std::string& os, const char* in, const int siz)
{
    const char* const max = in + siz;

    const char* iter = in;
    const char* last = iter;

    int lengthC = 0; // ǰ������, ������������ʼ�� 0x0F
    int lengthT = 0; // �����ı���, ������ %K...0x00 ��β, ������ 0x00 0x0F ��ͷ

    int order = 0;

    char textA[2048]; // ����
    char textB[2048]; // ����
    char textC[8]; // ģʽ
    char textM[16384]; // �ı���

    while(1)
    {
        if(iter >= max) break;

        if(*iter == 0)
        {
            ++iter;
            if(iter >= max) break;

            if(*iter == 0xf)
            {
                ++iter;
                if(iter >= max) break;

                ++order; // ��Ӽ���
                lengthC = iter - last;
                last = iter; // ����ǰ������

                // ��ʼ�����ı�
                textA[0] = textB[0] = textC[0] = textM[0] = 0; // ��ջ���������ֹ�����ظ�����

                if(*iter == 0xf)
                {
                    sprintf(textC, "double");
                    ++iter;
                    for(int i = 0; iter < max;)
                    {
                        if((textA[i++] = *iter++) == 0) break;
                    }

                    for(int i = 0; iter < max;)
                    {
                        if((textB[i++] = *iter++) == 0) break;
                    }
                }
                else
                {
                    sprintf(textC, "single");
                    for(int i = 0; iter < max;)
                    {
                        if((textB[i++] = *iter++) == 0) break;
                    }
                }

                // textB Ӧ���ı�, ��ʱ iter ָ���ַ�����β�� 0x00 ����һλ
                --iter;

                for(unsigned int i = 0; i < strlen(textB); ++i) // '%' != 0, ���ⳤ�Ȳ���ﵽ 2048
                {
                    if(textB[i] == '%' && textB[i+1] == 'K')
                    {
                        iter -= (strlen(textB) - i);
                        textB[i] = 0;
                        break;
                    }
                }

                lengthT = iter - last;
                last = iter; // �����ı��鳤��

                // ��������Ϣ
                sprintf(textM, "\r\n<%04d, %04X, %04X, %s>\r\n%s\r\n<!%s>\r\n%s\r\n", order, lengthC, lengthT, textC, textA, textB, textB);
                os += textM;
            }
        }
        else
        {
            ++iter;
        }
    }
    return order;
}

void dis(char* ipf, char* opf)
{
    FILE* ipfs = 0;
    FILE* opfs = 0;

    char* ipbuf = 0;

    do
    {
        ipfs = fopen(ipf, "rb");
        if(ipfs == 0) break;

        fseek(ipfs, 0, SEEK_END);
        int fsiz = ftell(ipfs);

        fseek(ipfs, 0, SEEK_SET);

        ipbuf = new(std::nothrow) char[fsiz];
        if(ipbuf == 0) break;

        fread(ipbuf, sizeof(char), fsiz, ipfs);

        std::string out("");
        out += marker;
        out += "\r\n";

        int order = dissociate_text(out, ipbuf, fsiz);
        char tmp[100];
        sprintf(tmp, "\r\n<@%04d, EOF>\r\n", order);
        out += tmp;

        printf("%s, %d\n", opf, order);

        opfs = fopen(opf, "wb+");
        if(opfs == 0) break;
        fputs(out.c_str(), opfs);
    }
    while (0);

    if(ipfs) fclose(ipfs);
    if(opfs) fclose(opfs);

    if(ipbuf) delete [] ipbuf;
}
