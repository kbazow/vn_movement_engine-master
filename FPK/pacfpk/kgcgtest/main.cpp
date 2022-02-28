#include<Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>

#include "png.h"
#include "zlib.h"

using namespace std;
/******************************ͼƬ����*********************************/
typedef struct _pic_data pic_data;
struct _pic_data
{
	unsigned int width, height; /* �ߴ� */
	int bit_depth;  /* λ�� */
	int flag;   /* һ����־����ʾ�Ƿ���alphaͨ�� */

	unsigned char *rgba; /* ͼƬ���� */
};

#pragma pack(1)

typedef struct KGHDR_S {
	unsigned char  signature[4]; // "GCGK"
	unsigned short width;
	unsigned short height;
	unsigned long  data_length;
} KGHDR;

#pragma pack()
/**********************************************************************/
#define PNG_BYTES_TO_CHECK 4
#define HAVE_ALPHA 1
#define NO_ALPHA 0

int read_png_file(string filepath, pic_data *out)
/* ���ڽ���pngͼƬ */
{
	FILE *pic_fp;
	pic_fp = fopen(filepath.c_str(), "rb");
	if (pic_fp == NULL) /* �ļ���ʧ�� */
		return -1;

	/* ��ʼ�����ֽṹ */
	png_structp png_ptr;
	png_infop   info_ptr;
	char        buf[PNG_BYTES_TO_CHECK];
	int         temp;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	info_ptr = png_create_info_struct(png_ptr);

	setjmp(png_jmpbuf(png_ptr)); // ������Ҫ

	temp = fread(buf, 1, PNG_BYTES_TO_CHECK, pic_fp);
	temp = png_sig_cmp((png_const_bytep)buf, (png_size_t)0, PNG_BYTES_TO_CHECK);

	/*����Ƿ�Ϊpng�ļ�*/
	if (temp != 0) return 1;

	rewind(pic_fp);
	/*��ʼ���ļ�*/
	png_init_io(png_ptr, pic_fp);
	// ���ļ���
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

	int color_type, channels;

	/*��ȡ��ȣ��߶ȣ�λ���ɫ����*/
	channels = png_get_channels(png_ptr, info_ptr); /*��ȡͨ����*/

	/////////////////�˶δ���������� FPK Galgame, ������ͼƬ��Ϊ4ͨ��//////////////////////////////////////////////////////////////////////////////////////////////////////
	if (channels < 4)
		return 1;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	out->bit_depth = png_get_bit_depth(png_ptr, info_ptr); /* ��ȡλ�� */
	color_type = png_get_color_type(png_ptr, info_ptr); /*��ɫ����*/

	int i, j;
	int size;
	/* row_pointers��߾���rgba���� */
	png_bytep* row_pointers;
	row_pointers = png_get_rows(png_ptr, info_ptr);
	out->width = png_get_image_width(png_ptr, info_ptr);
	out->height = png_get_image_height(png_ptr, info_ptr);

	size = out->width * out->height; /* ����ͼƬ�������ص����� */

	if (channels == 4 || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{/*�����RGB+alphaͨ��������RGB+�����ֽ�*/
		size *= (4 * sizeof(unsigned char)); /* ÿ�����ص�ռ4���ֽ��ڴ� */
		out->flag = HAVE_ALPHA;    /* ��� */
		out->rgba = (unsigned char*)malloc(size);
		if (out->rgba == NULL)
		{/* ��������ڴ�ʧ�� */
			fclose(pic_fp);
			puts("����(png):�޷������㹻���ڴ湩�洢����!");
			return 1;
		}

		temp = (4 * out->width);/* ÿ����4 * out->width���ֽ� */
		for (i = 0; i < out->height; i++)
		{
			for (j = 0; j < temp; j += 4)
			{/* һ���ֽ�һ���ֽڵĸ�ֵ */
				out->rgba[i*temp + j] = row_pointers[i][j];       // red
				out->rgba[i*temp + j + 1] = row_pointers[i][j + 1];   // green
				out->rgba[i*temp + j + 2] = row_pointers[i][j + 2];   // blue
				out->rgba[i*temp + j + 3] = row_pointers[i][j + 3];   // alpha
			}
		}
	}
	else if (channels == 3 || color_type == PNG_COLOR_TYPE_RGB)
	{/* �����RGBͨ�� */
		size *= (3 * sizeof(unsigned char)); /* ÿ�����ص�ռ3���ֽ��ڴ� */
		out->flag = NO_ALPHA;    /* ��� */
		out->rgba = (unsigned char*)malloc(size);
		memset(out->rgba, 0, size);
		if (out->rgba == NULL)
		{/* ��������ڴ�ʧ�� */
			fclose(pic_fp);
			puts("����(png):�޷������㹻���ڴ湩�洢����!");
			return 1;
		}

		temp = (3 * out->width);/* ÿ����3 * out->width���ֽ� */
		for (i = 0; i < out->height; i++)
		{
			for (j = 0; j < temp; j += 3)
			{/* һ���ֽ�һ���ֽڵĸ�ֵ */
				out->rgba[i*temp + j] = row_pointers[i][j];       // red
				out->rgba[i*temp + j + 1] = row_pointers[i][j + 1];   // green
				out->rgba[i*temp + j + 2] = row_pointers[i][j + 2];   // blue
			}
		}
	}
	else return 1;

	/* ��������ռ�õ��ڴ� */
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	return 0;
}

/*
bool if_diff(unsigned char x, unsigned char y)
{
	return (x != y);
}

vector<unsigned int> convert_channel(unsigned char*& data, unsigned int width, unsigned int height, unsigned int &data_len)
{
	vector<unsigned int> offset_table;
	vector<unsigned char> kg_data;
	kg_data.reserve(5 * width * height);

	vector<unsigned char>r, g, b, a;
	vector<unsigned char>::iterator ires;
	unsigned int x, y;
	unsigned int row_size = width * 4;

	unsigned int n = 0;

	for (y = 0; y < height; y++)
	{
		offset_table.push_back(kg_data.size());

		unsigned char *cur_row = data + y * row_size;
		for (x = 0; x < row_size; x += 4)
		{
			r.push_back(cur_row[x]);
			g.push_back(cur_row[x+1]);
			b.push_back(cur_row[x+2]);
			a.push_back(cur_row[x+3]);
		}

		while (true)
		{
			if (a.size() >= 256)
			{
				ires = adjacent_find(a.begin(), a.begin()+256, if_diff);
				if (ires != a.begin() + 256)
					n = distance(a.begin(), ires) + 1 ;
				else
					n = distance(a.begin(), ires);

				kg_data.push_back(a[0]);
				kg_data.push_back((unsigned char)n);
				if (a[0])
				{
					for (unsigned int i = 0; i < n; i++)
					{
						kg_data.push_back(r[i]);
						kg_data.push_back(g[i]);
						kg_data.push_back(b[i]);
					}
				}
				a.erase(a.begin(), a.begin() + n);
				r.erase(r.begin(), r.begin() + n);
				g.erase(g.begin(), g.begin() + n);
				b.erase(b.begin(), b.begin() + n);
			}
			else
			{
				ires = adjacent_find(a.begin(), a.end(), if_diff);
				if (ires != a.end())
					n = distance(a.begin(), ires) + 1;
				else
					n = distance(a.begin(), ires);

				kg_data.push_back(a[0]);
				kg_data.push_back((unsigned char)n);
				if (a[0])
				{
					for (unsigned int i = 0; i < n; i++)
					{
						kg_data.push_back(r[i]);
						kg_data.push_back(g[i]);
						kg_data.push_back(b[i]);
					}
				}
				a.erase(a.begin(), a.begin() + n);
				r.erase(r.begin(), r.begin() + n);
				g.erase(g.begin(), g.begin() + n);
				b.erase(b.begin(), b.begin() + n);
			}
			if (!a.size()) break;
		}
	}
	delete[]data;

	unsigned int out_len = kg_data.size();
	unsigned char *out_data = new unsigned char[out_len];
	memcpy(out_data, &kg_data[0], out_len); //kg_data�ں������غ��Զ����գ���
	data = out_data;
	data_len = out_len;

	return offset_table;
}
*/

unsigned int find_diff(unsigned char*buff, unsigned int begin, unsigned int end)
{
	unsigned int res = 0;
	unsigned int i;
	for (i = begin; i < end; i++)
	{
		if ((i + 1) == end)
		{
			return end;
		}
		if (buff[i] != buff[i + 1])
		{
			res = i;
			break;
		}
	}
	return res;
}

vector<unsigned int> convert_channel(unsigned char*& data, unsigned int width, unsigned int height, unsigned int &data_len)
{
	vector<unsigned int> offset_table;

	unsigned int row_size = width * 4;
	unsigned char *kg_data = new unsigned char[row_size * height];

	unsigned char *r = new unsigned char[width];
	unsigned char *g = new unsigned char[width];
	unsigned char *b = new unsigned char[width];
	unsigned char *a = new unsigned char[width];

	unsigned int x, y;

	unsigned int n = 0;
	unsigned int pos = 0, pix = 0, cur_front = 0;

	for (y = 0; y < height; y++)
	{
		offset_table.push_back(pos);

		unsigned char *cur_row = data + y * row_size;
		pix = 0;
		for (x = 0; x < row_size; x += 4)
		{
			r[pix] = (cur_row[x]);
			g[pix] = (cur_row[x + 1]);
			b[pix] = (cur_row[x + 2]);
			a[pix] = (cur_row[x + 3]);
			++pix;
		}
		cur_front = 0;
		while (true)
		{
			if (pix - cur_front >= 256)
			{
				n = find_diff(a, cur_front, cur_front+256);
				if (n != cur_front + 256)
					n = n - cur_front + 1;
				else
					n -= cur_front;

				kg_data[pos++] = a[cur_front];
				kg_data[pos++] = (unsigned char)n;
				if (a[cur_front])
				{
					for (unsigned int i = 0; i < n; i++)
					{
						kg_data[pos++] = r[cur_front + i];
						kg_data[pos++] = g[cur_front + i];
						kg_data[pos++] = b[cur_front + i];
					}
				}
				cur_front += n;
			}
			else
			{
				n = find_diff(a, cur_front, pix);
				if (n != pix)
					n = n - cur_front + 1;
				else
					n -= cur_front;

				kg_data[pos++] = a[cur_front];
				kg_data[pos++] = (unsigned char)n;
				if (a[cur_front])
				{
					for (unsigned int i = 0; i < n; i++)
					{
						kg_data[pos++] = r[cur_front + i];
						kg_data[pos++] = g[cur_front + i];
						kg_data[pos++] = b[cur_front + i];
					}
				}
				cur_front += n;
			}
			if (pix == cur_front) break;
		}
	}
	//free(data);
	delete[]data;
	delete[]r;
	delete[]g;
	delete[]b;
	delete[]a;

	unsigned int out_len = pos;
	unsigned char *out_data = new unsigned char[out_len];
	memcpy(out_data, kg_data, out_len);

	delete[]kg_data;

	data = out_data;
	data_len = out_len;

	return offset_table;
}

void png2kgcg(string filename, unsigned char*&out_buff, unsigned int &out_len)
{
	pic_data pic;
	//�ж��Ƿ�4ͨ��ͼƬ
	int is_not_4ch = 0;
	is_not_4ch = read_png_file(filename, &pic);
	//��������򷵻�
	if (is_not_4ch)
	{
		printf("��֧�� ��4ͨ��ͼƬ\n");
		return;
	}
		

	unsigned int data_len = 0;
	vector<unsigned int> off_tbl;

	off_tbl = convert_channel(pic.rgba, pic.width, pic.height, data_len);

	KGHDR kg;
	memcpy(kg.signature, "GCGK", 4);
	kg.height = (unsigned short)pic.height;
	kg.width = (unsigned short)pic.width;
	kg.data_length = data_len;

	unsigned int out_size = sizeof(KGHDR) + sizeof(unsigned int) * off_tbl.size() + data_len;
	unsigned char *out_data = new unsigned char[out_size];
	unsigned char *p = out_data;

	memcpy(p, &kg, sizeof(KGHDR));
	p += sizeof(KGHDR);
	memcpy(p, &off_tbl[0], sizeof(unsigned int)* off_tbl.size());
	p += sizeof(unsigned int)* off_tbl.size();
	memcpy(p, pic.rgba, data_len);

	out_buff = out_data;
	out_len = out_size;
	delete[]pic.rgba;
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "png2kgcg v1.0 by ����\n\n");
		fprintf(stderr, "usage: %s <input.png>\n", argv[0]);
		return -1;
	}
	unsigned char *data = NULL;
	unsigned int len = 0;
	png2kgcg(argv[1], data,  len);

	string pngname = argv[1];
	string kgname = pngname.substr(0, pngname.find_last_of('.')) + ".kg";
	FILE *fout = fopen(kgname.c_str(), "wb");
	if (!fout)
	{
		printf("Create KG file failed!");
		return -1;
	}
	fwrite(data, 1, len, fout);
	fclose(fout);
	delete[]data;
	return 0;
}