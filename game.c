#define _CRT_SECURE_NO_WARNINGS
#include "bmp_functions.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>


BMPfile* readBMP(const char* test1)
{
    FILE* file;
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER bmp_header;
    int** matrix = NULL;
    RGBQUAD* quads = NULL;

    /// файл
    file = fopen(test1, "rb");
    if (!file)
    {
        printf("Wrong: seed file is not opened.\n");
        return NULL;
    }

    /// считываем file_header
    fread(&file_header.bfType, sizeof(unsigned char), 2, file);
    fread(&file_header.bfSize, sizeof(unsigned char), 4, file);
    fread(&file_header.bfReserved1, sizeof(unsigned char), 2, file);
    fread(&file_header.bfReserved2, sizeof(unsigned char), 2, file);
    fread(&file_header.bfOffBits, sizeof(unsigned char), 4, file);

    // проверяем тип файла (BMP)
    if (file_header.bfType != 0x4d42)
    {
        printf("Wrong: seed file is not BMP.\n");
        fclose(file);
        return NULL;
    }

    // считываем bmp_header
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

    // Проверяем размер bmp_header на 40 байт
    if (bmp_header.biSize != 0x28)
    {
        printf("Wrong: seed file`s header is not correct.\n");
        fclose(file);
        return NULL;
    }

    // Определяем размера массива цветов
    int size_file_header = 14;
    int colorbytes = file_header.bfOffBits - bmp_header.biSize - size_file_header;
    // определем размер без file_header
    int totalbytes = file_header.bfSize - size_file_header;

    /// считываем цветовую палитру 
    quads = (RGBQUAD*)malloc(colorbytes);
    BYTE* pixel = (BYTE*)malloc(totalbytes - colorbytes - bmp_header.biSize);
    for (int i = 0; i < colorbytes / sizeof(RGBQUAD); ++i)
    {
        fread(&quads[i].rgbBlue, sizeof(BYTE), 1, file);
        fread(&quads[i].rgbGreen, sizeof(BYTE), 1, file);
        fread(&quads[i].rgbRed, sizeof(BYTE), 1, file);
        fread(&quads[i].rgbReserved, sizeof(BYTE), 1, file);
    }

    /// Заводим матрицу для хранения поля игры в программе, считываем пиксели в буфер
    matrix = (int**)malloc(bmp_header.biHeight * sizeof(int*));
    for (int i = 0; i < bmp_header.biHeight; ++i)
    {
        matrix[i] = (int*)malloc(bmp_header.biWidth * sizeof(int));
    }
    fread(pixel, sizeof(BYTE), totalbytes - colorbytes - bmp_header.biSize, file);

    /// Узнаем, сколько байтов хранят существенные пиксели, а сколько - дополнительные до 4-х
    int used_bytes = bmp_header.biWidth / 8;
    if (bmp_header.biWidth % 8 != 0)
        ++used_bytes;
    int to4_bytes = used_bytes % 4;
    if (to4_bytes != 0)
        to4_bytes = 4 - to4_bytes;

    int buff_byte_number = 0;
    BYTE cur_byte;

    /// Считываем существенную информацию о поле в матрицу
    for (int row = 0; row < bmp_header.biHeight; ++row)
    {
        /// считываем существенные байты
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

        /// считываем смешанные байты
        cur_byte = pixel[buff_byte_number];
        for (int col = 7; col >= 8 - bmp_header.biWidth % 8; --col)
        {
            /// получаем отдельные биты из целого байта
            int result = 1;
            if ((cur_byte & (1 << (col))) == 0)
                result = 0;
            matrix[row][(used_bytes - 1) * 8 + 7 - col] = result;
        }

        /// пропускаем доп байты
        buff_byte_number += 1 + to4_bytes;
    }


    fclose(file);

    /// Из собранных данных формируем BMPfile внутри программы
    BMPfile* bmp_file = (BMPfile*)malloc(sizeof(BMPfile));
    bmp_file->fileHeader = file_header;
    bmp_file->bmpHeader = bmp_header;
    bmp_file->palett = quads;
    bmp_file->pixels = matrix;
    return bmp_file;
}

/// Функция сохраняет данный монохромный BMP файл под данным именем
bool saveBMP(BMPfile* bmp_file, const char* test1)
{
    /// Создаем файл и дублируем в него все, кроме пикселей поля, из данного файла
    BITMAPFILEHEADER file_header = bmp_file->fileHeader;
    BITMAPINFOHEADER bmp_header = bmp_file->bmpHeader;
    int** matrix = bmp_file->pixels;
    RGBQUAD* quads = bmp_file->palett;

    FILE* file;
    file = fopen(test1, "wb");
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

    /// Узнаем, сколько байтов будут содержать существенные пиксели, а сколько будут
    /// дополнять до 4-х
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
        /// Запись существенных байтов
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

        /// Формирование и запист смешанного байта
        cur_byte = 0;
        int position = 128;
        for (int bit = 0; bit <= bmp_header.biWidth % 8; ++bit)
        {
            /// Перевод 8 отдельных битов в один байт
            if (matrix[row][(used_bytes - 1) * 8 + bit] == 1)
            {
                cur_byte += position;
            }
            position /= 2;
        }
        fwrite(&cur_byte, sizeof(BYTE), 1, file);

        /// Запись дополнительных байтов
        BYTE empty_byte = 0;
        fwrite(&empty_byte, sizeof(BYTE), to4_bytes, file);
    }

    
    fclose(file);
    return true;
}
/// Функция проверяет, существует ли каталог с данными именем.
bool directoryExists(const char* absolutePath) {

    if (_access(absolutePath, 0) == 0) {

        struct stat status;
        stat(absolutePath, &status);

        return (status.st_mode & S_IFDIR) != 0;
    }
    return false;
}

/// Функция осуществляет переход между левым и правым краями поля,
/// как если бы оно было в форме цилиндра. Возвращает координату.

int xadd(int i, int a, int w) {
    i += a;
    while (i < 0)
    {
        i += w;
    }
    while (i >= w)
    {
        i -= w;
    }
    return i;
}

/// Функция осуществляет переход между верхним и нижним краями поля,
/// Возвращает координату.
int yadd(int i, int a, int h) {
    i += a;
    while (i < 0)
    {
        i += h;
    }
    while (i >= h)
    {
        i -= h;
    }
    return i;
}

/// Функция считает живые клетки, соседние с данной, возвращает их число.
int adjacent_to(int** matrix, int w, int h, int i, int j)
{
    int count = 0;
    for (int k = -1; k <= 1; k++)
    {
        for (int l = -1; l <= 1; l++)
        {
            /// k или l не ноль - не проверяем саму текущую клетку.
            if (k || l)
                if (!matrix[yadd(i, k, h)][xadd(j, l, w)])
                    count++;
        }
    }
    return count;
}

/// Функция по правилам игры рассчитывает из current_matrix новую итерацию
/// и записывает её в new_matrix.
void step(int** current_matrix, int** new_matrix, int w, int h)
{
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            int a = adjacent_to(current_matrix, w, h, i, j);
            if (a == 2) new_matrix[i][j] = current_matrix[i][j];
            if (a == 3) new_matrix[i][j] = 0;
            if (a < 2) new_matrix[i][j] = 1;
            if (a > 3) new_matrix[i][j] = 1;
        }
    }

}

/// Функция производит max_iter * dump_freq итераций, начиная с начальной

bool game_life(BMPfile* bmp_file, int max_iter, int dump_freq, const char* output_dir)
{
    /// Заводим буффер для хранения новой итерации.
    int** current_matrix = bmp_file->pixels;
    int** new_matrix = (int**)malloc(sizeof(int*) * bmp_file->bmpHeader.biHeight);
    for (int i = 0; i < bmp_file->bmpHeader.biHeight; ++i)
    {
        new_matrix[i] = (int*)malloc(sizeof(int) * bmp_file->bmpHeader.biWidth);
    }

    /// Подготовка имени, под которым будут сохраняться итерации.
    int count = 0;
    char rezult[100];
    int length = sprintf(rezult, "%s/", output_dir);

    /// Итерации.
    for (int i = 0; i < max_iter * dump_freq; ++i)
    {
        /// производим одну итерацию
        step(current_matrix, new_matrix, bmp_file->bmpHeader.biWidth, bmp_file->bmpHeader.biHeight);

        /// Записываем новую итерацию вместо текущей.
        for (int i = 0; i < bmp_file->bmpHeader.biHeight; ++i)
        {
            for (int j = 0; j < bmp_file->bmpHeader.biWidth; ++j)
            {
                bmp_file->pixels[i][j] = new_matrix[i][j];
            }
        }

        /// Сохраняем итерацию в соответствии с частотой сохранения.
        if (i % dump_freq == 0)
        {
            count++;
            sprintf(rezult + length, "%d.bmp", count);
            if (!saveBMP(bmp_file, rezult))
            {
                for (int i = 0; i < bmp_file->bmpHeader.biHeight; ++i)
                {
                    free(new_matrix[i]);
                }
                free(new_matrix);
                return false;
            }
        }
    }

    /// Освобождаем память буффера.
    for (int i = 0; i < bmp_file->bmpHeader.biHeight; ++i)
    {
        free(new_matrix[i]);
    }
    free(new_matrix);

    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Wrong: seed file name and output directory are missed.\n");
        return -1;
    }

    /// считываем входные данные
    const char* test1 = argv[1];
    if (!test1)
    {
        printf("Wrong: seed file name.\n");
        return -1;
    }

    const char* output_dir = argv[2];
    if (!directoryExists(output_dir))
    {
        printf("Wrong: output directory does not exist.\n");
        return -1;
    }

    int max_iter, dump_freq;
    if (argc < 5)
    {
        dump_freq = 1;
    }
    if (argc < 4)
    {
        max_iter = INT_MAX;
    }
    if (argc == 5)
    {
        max_iter = atoi(argv[3]);
        dump_freq = atoi(argv[4]);
    }

    /// Считываем картинку начальной итерации.
    BMPfile* bmp_file = readBMP(test1);
    if (bmp_file == NULL)
    {
        return -1;
    }
   
    /// запуск игры
    if (!game_life(bmp_file, max_iter, dump_freq, output_dir))
        return -1;

    /// освобождение памяти
    for (int i = 0; i < bmp_file->bmpHeader.biHeight; ++i)
    {
        free(bmp_file->pixels[i]);
    }
    free(bmp_file->pixels);
    free(bmp_file->palett);
    free(bmp_file);

    return 0;
    
}