#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>

typedef unsigned __int16 WORD;
typedef unsigned __int32 DWORD;
typedef long int LONG;

/// Структура, отражающая устройство fileHeader`a
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, * PBITMAPFILEHEADER;

/// Структура, отражающая устройство infoHeader`a
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


/// Структура, отражающая устройство информации о цвете пикселя
typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;


/// Структура для представления bmp файла внутри программы
typedef struct BMPfile
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER bmpHeader;
    RGBQUAD* palett;
    int** pixels;
} BMPfile;


/// Функция считывает монохромный bmp файл с заданным именем 
BMPfile* readBMP(const char* test1);

/// Функция сохраняет данный монохромный BMP файл под данным именем.
bool saveBMP(BMPfile* bmp_file, const char* test1);
