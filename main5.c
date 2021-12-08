#define _CRT_SECURE_NO_WARNINGS
#include "bmp_functions.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

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
/// как если бы оно было в форме цилиндра. Возвращает координату.
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
/// (хранится в bmp_file) и сохраняет итерации с частотой dump_freq.
/// В резуоьтате max_iter картинок будут сохранены в output_dir.
/// В случае проблем возвращает false.
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
    char fname[100];
    int length = sprintf(fname, "%s/", output_dir);

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
            sprintf(fname + length, "%d.bmp", count);
            if (!saveBMP(bmp_file, fname))
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
    const char* fname = argv[1];
    if (!fname)
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
    BMPfile* bmp_file = readBMP(fname);
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