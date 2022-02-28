#ifndef __RCT_H__
#define __RCT_H__

#include <Windows.h>
#include <stdio.h>
#include <string>

#include "zlib.h"
#include "zconf.h"
#include "png.h"
#include "pngconf.h"


using namespace std;

#pragma pack (1)

typedef struct {
	BYTE magic[8];		/* "����TC00" and "����TC01" and "����TS00" and "����TS01" */
	DWORD width;
	DWORD height;
	DWORD data_length;
} rct_header_t;

typedef struct {		/* ������0x300�ֽ��ǵ�ɫ�� */
	BYTE magic[8];		/* "����8_00" */
	DWORD width;
	DWORD height;
	DWORD data_length;
} rc8_header_t;

 typedef struct _pic_data
{
	unsigned int width, height; /* �ߴ� */
	int bit_depth;  /* λ�� */
	int flag;   /* һ����־����ʾ�Ƿ���alphaͨ�� */

	unsigned char *rgba; /* ͼƬ���� */
 } pic_data;

#pragma pack ()		

#define PNG_BYTES_TO_CHECK 4
#define HAVE_ALPHA 1
#define NO_ALPHA 0


class RCT
{
public:
	RCT();
	bool LoadRCT(string fname);
	bool LoadPNG(string pngname);
	void RCT2PNG();
	void PNG2RCT();
	~RCT();
private:
	DWORD rc8_decompress(BYTE *uncompr, DWORD uncomprLen, BYTE *compr, DWORD comprLen, DWORD width);
	DWORD rct_decompress(BYTE *uncompr, DWORD uncomprLen, BYTE *compr, DWORD comprLen, DWORD width);

	//����αѹ������ռ�
	DWORD rc_compressBound(DWORD srclen, int channels);
	//αѹ������
	DWORD rc8_compress(BYTE *compr, DWORD comprLen, BYTE *uncompr, DWORD uncomprLen);
	DWORD rct_compress(BYTE *compr, DWORD comprLen, BYTE *uncompr, DWORD uncomprLen);

	bool dump_rc8(rc8_header_t *rc8, BYTE *&ret_rgb);
	bool dump_rct(rct_header_t *rct, BYTE *&ret_rgb);

	bool write_rc8(rc8_header_t *hrc8, BYTE *alp);
	bool write_rct(rct_header_t *hrct, BYTE *rgb);

	static uLong hash_table[256];
	static uLong hash_table2[256];
	static void init_hash(void);
	static uLong do_hash(uLong seed, byte* name, uLong name_len);
	static void init_hash2(byte* name, uLong name_len);
	void decrypt(byte* buff, uLong len);

	int read_png_file(string filepath, pic_data *out);
	int write_png_file(string file_name, pic_data *graph);

	//���ڶ�rct�ļ�����rc8ʱΪ��
	bool have_alpha;
	string fn_png;

	string fn_rct; //��ǰ������rct�ļ���
	BYTE* p_rct;
	rct_header_t *h_rct;

	string fn_rc8; //��ǰ������rc8�ļ���
	BYTE* p_rc8;
	rc8_header_t *h_rc8;

	//png��Ϣ
	pic_data png_info;

	bool is_encrypt;
	bool is_ts01;
};
#endif