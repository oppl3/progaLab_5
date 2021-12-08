#define _CRT_SECURE_NO_WARNINGS
#include "bmp_functions.h"

BMPfile* readBMP(const char* fname)
{
    FILE* file;
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER bmp_header;
    int** matrix = NULL;
    RGBQUAD* quads = NULL;

    /// ��������� ����
    file = fopen(fname, "rb");
    if (!file)
    {
        printf("Wrong: seed file is not opened.\n");
        return NULL;
    }

    /// ��������� file_header
    fread(&file_header.bfType, sizeof(unsigned char), 2, file);
    fread(&file_header.bfSize, sizeof(unsigned char), 4, file);
    fread(&file_header.bfReserved1, sizeof(unsigned char), 2, file);
    fread(&file_header.bfReserved2, sizeof(unsigned char), 2, file);
    fread(&file_header.bfOffBits, sizeof(unsigned char), 4, file);

    // ��������� ��� ����� (BMP)
    if (file_header.bfType != 0x4d42)
    {
        printf("Wrong: seed file is not BMP.\n");
        fclose(file);
        return NULL;
    }

    // ��������� bmp_header
    fread(&bmp_header.biSize, sizeof(unsigned char), 4, file);
    fread(&bmp_header.biWidth, sizeof(unsigned char), 4, file);
    fread(&bmp_header.biHeight, sizeof(unsigned char), 4, file);
    fread(&bmp_header.biPlanes, sizeof(unsigned char), 2, file);
    fread(&bmp_header.biBitCount, sizeof(unsigned char), 2, file);
    fread(&bmp_header.biCompression, sizeof(unsigned char), 4, file);
    fread(&bmp_header.biSizeImage, sizeof(unsigned char), 4, file);
    fread(&bmp_header.biXPelsPerMeter, sizeof(unsigned char), 4, file);
    fread(&bmp_header.biYPelsPerMeter, sizeof(unsigned char), 4, file);
    fread(&bmp_header.biClrUsed, sizeof(unsigned char), 4, file);
    fread(&bmp_header.biClrImportant, sizeof(unsigned char), 4, file);

    // ��������� ������ bmp_header �� 40 ����
    if (bmp_header.biSize != 0x28)
    {
        printf("Wrong: seed file`s header is not correct.\n");
        fclose(file);
        return NULL;
    }

    // ���������� ������� ������� ������
    int size_file_header = 14;
    int colorbytes = file_header.bfOffBits - bmp_header.biSize - size_file_header;
    // ���������� ������ ��� file_header
    int totalbytes = file_header.bfSize - size_file_header;

    /// ��������� �������� �������
    quads = (RGBQUAD*)malloc(colorbytes);
    BYTE* pixel = (BYTE*)malloc(totalbytes - colorbytes - bmp_header.biSize);
    for (int i = 0; i < colorbytes / sizeof(RGBQUAD); ++i)
    {
        fread(&quads[i].rgbBlue, sizeof(BYTE), 1, file);
        fread(&quads[i].rgbGreen, sizeof(BYTE), 1, file);
        fread(&quads[i].rgbRed, sizeof(BYTE), 1, file);
        fread(&quads[i].rgbReserved, sizeof(BYTE), 1, file);
    }

    /// ������� ������� ��� �������� ���� ���� � ���������, ��������� ������� � �����
    matrix = (int**)malloc(bmp_header.biHeight * sizeof(int*));
    for (int i = 0; i < bmp_header.biHeight; ++i)
    {
        matrix[i] = (int*)malloc(bmp_header.biWidth * sizeof(int));
    }
    fread(pixel, sizeof(BYTE), totalbytes - colorbytes - bmp_header.biSize, file);

    /// ������, ������� ������ ������ ������������ �������, � ������� - �������������� �� 4-�
    int used_bytes = bmp_header.biWidth / 8;
    if (bmp_header.biWidth % 8 != 0)
        ++used_bytes;
    int to4_bytes = used_bytes % 4;
    if (to4_bytes != 0)
        to4_bytes = 4 - to4_bytes;

    int buff_byte_number = 0;
    BYTE cur_byte;

    /// ��������� ������������ ���������� � ���� � �������
    for (int row = 0; row < bmp_header.biHeight; ++row)
    {
        /// ���������� ������������ ������
        for (int col_byte = 0; col_byte < used_bytes - 1; ++col_byte)
        {
            cur_byte = pixel[buff_byte_number];
            for (int col = 7; col >= 0; --col)
            {
                int result = 1;
                if ((cur_byte & (1 << col)) == 0)
                    result = 0;
                matrix[row][col_byte * 8 + 7 - col] = result;
            }
            ++buff_byte_number;
        }

        /// ���������� ���������� �����
        cur_byte = pixel[buff_byte_number];
        for (int col = 7; col >= 8 - bmp_header.biWidth % 8; --col)
        {
            /// ��������� ��������� ����� �� ������ �����
            int result = 1;
            if ((cur_byte & (1 << (col))) == 0)
                result = 0;
            matrix[row][(used_bytes - 1) * 8 + 7 - col] = result;
        }

        /// ������� �������������� ������
        buff_byte_number += 1 + to4_bytes;
    }

    // ��������� ����
    fclose(file);

    /// �� ��������� ������ ��������� BMPfile ������ ���������
    BMPfile* bmp_file = (BMPfile*)malloc(sizeof(BMPfile));
    bmp_file->fileHeader = file_header;
    bmp_file->bmpHeader = bmp_header;
    bmp_file->palett = quads;
    bmp_file->pixels = matrix;
    return bmp_file;
}

/// ������� ��������� ������ ����������� BMP ���� ��� ������ ������ 
bool saveBMP(BMPfile* bmp_file, const char* fname)
{
    /// ������� ���� � ��������� � ���� ���, ����� �������� ����, �� ������� �����
    BITMAPFILEHEADER file_header = bmp_file->fileHeader;
    BITMAPINFOHEADER bmp_header = bmp_file->bmpHeader;
    int** matrix = bmp_file->pixels;
    RGBQUAD* quads = bmp_file->palett;

    FILE* file;
    file = fopen(fname, "wb");
    if (!file)
    {
      //  printf("Wrong: can not save iteration file.\n");
      //  return false;
    }

    fwrite(&file_header.bfType, sizeof(WORD), 1, file);
    fwrite(&file_header.bfSize, sizeof(DWORD), 1, file);
    fwrite(&file_header.bfReserved1, sizeof(WORD), 1, file);
    fwrite(&file_header.bfReserved2, sizeof(WORD), 1, file);
    fwrite(&file_header.bfOffBits, sizeof(DWORD), 1, file);

    fwrite(&bmp_header.biSize, sizeof(unsigned char), 4, file);
    fwrite(&bmp_header.biWidth, sizeof(unsigned char), 4, file);
    fwrite(&bmp_header.biHeight, sizeof(unsigned char), 4, file);
    fwrite(&bmp_header.biPlanes, sizeof(unsigned char), 2, file);
    fwrite(&bmp_header.biBitCount, sizeof(unsigned char), 2, file);
    fwrite(&bmp_header.biCompression, sizeof(unsigned char), 4, file);
    fwrite(&bmp_header.biSizeImage, sizeof(unsigned char), 4, file);
    fwrite(&bmp_header.biXPelsPerMeter, sizeof(unsigned char), 4, file);
    fwrite(&bmp_header.biYPelsPerMeter, sizeof(unsigned char), 4, file);
    fwrite(&bmp_header.biClrUsed, sizeof(unsigned char), 4, file);
    fwrite(&bmp_header.biClrImportant, sizeof(unsigned char), 4, file);

    int colorbytes = file_header.bfOffBits - bmp_header.biSize - 14;
    for (int i = 0; i < colorbytes / sizeof(RGBQUAD); ++i)
    {
        fwrite(&quads[i].rgbBlue, sizeof(BYTE), 1, file);
        fwrite(&quads[i].rgbGreen, sizeof(BYTE), 1, file);
        fwrite(&quads[i].rgbRed, sizeof(BYTE), 1, file);
        fwrite(&quads[i].rgbReserved, sizeof(BYTE), 1, file);
    }

    /// ������, ������� ������ ����� ��������� ������������ �������, � ������� �����
    /// ��������� �� 4-�
    int used_bytes = bmp_header.biWidth / 8;
    if (bmp_header.biWidth % 8 != 0)
        ++used_bytes;
    int to4_bytes = used_bytes % 4;
    if (to4_bytes != 0)
        to4_bytes = 4 - to4_bytes;

    int pixelSize = (used_bytes + to4_bytes) * bmp_header.biHeight;

    int buff_byte_number = 0;
    BYTE cur_byte;
    for (int row = 0; row < bmp_header.biHeight; ++row)
    {
        /// ������ ������������ ������
        for (int col_byte = 0; col_byte < used_bytes - 1; ++col_byte)
        {
            cur_byte = 0;
            int position = 128;
            for (int bit = 0; bit < 8; ++bit)
            {
                if (matrix[row][col_byte * 8 + bit] == 1)
                {
                    cur_byte += position;
                }
                position /= 2;
            }
            fwrite(&cur_byte, sizeof(BYTE), 1, file);
        }

        /// ������������ � ������ ���������� �����
        cur_byte = 0;
        int position = 128;
        for (int bit = 0; bit <= bmp_header.biWidth % 8; ++bit)
        {
            /// ������� 8 ��������� ����� � ���� ����
            if (matrix[row][(used_bytes - 1) * 8 + bit] == 1)
            {
                cur_byte += position;
            }
            position /= 2;
        }
        fwrite(&cur_byte, sizeof(BYTE), 1, file);

        /// ������ �������������� ������
        BYTE empty_byte = 0;
        fwrite(&empty_byte, sizeof(BYTE), to4_bytes, file);
    }

    /// ��������� ����
    fclose(file);
    return true;
}