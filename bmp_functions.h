#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>

typedef unsigned __int16 WORD;
typedef unsigned __int32 DWORD;
typedef long int LONG;

/// ���������, ���������� ���������� fileHeader`a
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, * PBITMAPFILEHEADER;

/// ���������, ���������� ���������� infoHeader`a
typedef struct tagBITMAPINFOHEADER
{
    DWORD  biSize;
    LONG   biWidth;
    LONG   biHeight;
    WORD   biPlanes;
    WORD   biBitCount;
    DWORD  biCompression;
    DWORD  biSizeImage;
    LONG   biXPelsPerMeter;
    LONG   biYPelsPerMeter;
    DWORD  biClrUsed;
    DWORD  biClrImportant;
} BITMAPINFOHEADER, * PBITMAPINFOHEADER;

typedef unsigned __int8 BYTE;

/// ���������, ���������� ���������� ���������� � ����� �������
typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;

/// ��������� ��� �������� ������������� bmp ����� ������ ���������
typedef struct BMPfile
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER bmpHeader;
    RGBQUAD* palett;
    int** pixels;
} BMPfile;

/// ������� ��������� ����������� bmp ���� � �������� ������ fname
/// � ��������� BMPfile � ���������� ��������� �� ��.
/// � ������ ������� ������� ��������� � �������� � ������� � ���������� NULL.
BMPfile* readBMP(const char* fname);

/// ������� ��������� ������ ����������� BMP ���� ��� ������ ������.
/// � ������ ������� ������� ��������� � �������� � ������� � ���������� false.
bool saveBMP(BMPfile* bmp_file, const char* fname);
