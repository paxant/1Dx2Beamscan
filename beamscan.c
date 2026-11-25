#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "shared_memory.h"
#include "math.h"
#define TIME_NUMBER_STEPS (int)((datad.modeling.T_end - datad.modeling.T_start) / datad.modeling.T_step)

float Thetta_range[THETTA_NUMBER_POINT];
float Phi_range[PHI_NUMBER_POINT];
double complex V[NUMBER_X * NUMBER_Y][NUMBER_SIGNALS];
double complex Rxx_ux[NUMBER_X][NUMBER_X];
double complex Rxx_uy[NUMBER_Y][NUMBER_Y];
double complex Rxx[NUMBER_X * NUMBER_Y][NUMBER_X * NUMBER_Y];
double complex Vnm_x[NUMBER_X][THETTA_NUMBER_POINT];
double complex Vnm_y[NUMBER_Y][PHI_NUMBER_POINT];
double complex BEAMscan_x[THETTA_NUMBER_POINT];
double complex BEAMscan_y[PHI_NUMBER_POINT];
double complex DigrArray[PHI_NUMBER_POINT][THETTA_NUMBER_POINT][NUMBER_Y][NUMBER_X];
double complex BEAMscan[PHI_NUMBER_POINT][THETTA_NUMBER_POINT];

struct BeamscanData_double datad;

int T = 0;
SharedSignalArray *shm;
void computing_Rxx1D();
void computing_Rxx2D();
void InitScanScaleZone();
void Init_1DArrayPattern();
void Init_2DArrayPattern();
void compute_BEAMscan_2D();
void compute_BEAMscan_1D();
void plot_BEAMscan_x();
void plot_BEAMscan_y();
void plot_BEAMscan_2D();

void copyStruct(const struct BeamscanData *src)
{
    int i, j;

    datad.estimation.dx = (double)src->estimation.dx / 1000.0;
    datad.estimation.dy = (double)src->estimation.dy / 1000.0;
    datad.estimation.Tx = (double)src->estimation.Tx / 1000.0;
    datad.estimation.Ty = (double)src->estimation.Ty / 1000.0;

    datad.estimation.Tx_min = (double)src->estimation.Tx_min / 1000.0;
    datad.estimation.Tx_max = (double)src->estimation.Tx_max / 1000.0;
    datad.estimation.Ty_min = (double)src->estimation.Ty_min / 1000.0;
    datad.estimation.Ty_max = (double)src->estimation.Ty_max / 1000.0;

    datad.estimation.Nx = src->estimation.Nx;
    datad.estimation.Ny = src->estimation.Ny;

    for (i = 0; i < datad.estimation.Nx; i++)
        for (j = 0; j < datad.estimation.Ny; j++)
            datad.estimation.Antenna[i][j] =
                src->estimation.Antenna[i][j];

    memcpy(datad.modeling.SNR,
           src->modeling.SNR,
           sizeof(datad.modeling.SNR));

    datad.modeling.T_start = (double)src->modeling.T_start / 1000.0;
    datad.modeling.T_step = (double)src->modeling.T_step / 1000.0;
    datad.modeling.T_end = (double)src->modeling.T_end / 1000.0;

    datad.modeling.N_Signal = src->modeling.N_Signal;

    for (i = 0; i < 255; i++)
        datad.modeling.TimeSimul[i] =
            (double)src->modeling.TimeSimul[i] / 1000.0;

    for (i = 0; i < datad.modeling.N_Signal; i++)
    {
        datad.modeling.F[i] = (double)src->modeling.F[i] / 1000.0;
        datad.modeling.x[i] = (double)src->modeling.x[i] / 1000.0;
        datad.modeling.y[i] = (double)src->modeling.y[i] / 1000.0;
        datad.modeling.A[i] = (double)src->modeling.A[i] / 1000.0;
    }
}

void printStruct(const struct BeamscanData_double *d)
{
    int i, j;

    printf("=========== Estimation ===========\n");

    printf("dx      = %.3f\n", d->estimation.dx);
    printf("dy      = %.3f\n", d->estimation.dy);
    printf("Tx      = %.3f\n", d->estimation.Tx);
    printf("Ty      = %.3f\n", d->estimation.Ty);

    printf("Tx_min  = %.3f\n", d->estimation.Tx_min);
    printf("Tx_max  = %.3f\n", d->estimation.Tx_max);
    printf("Ty_min  = %.3f\n", d->estimation.Ty_min);
    printf("Ty_max  = %.3f\n", d->estimation.Ty_max);

    printf("Nx      = %d\n", d->estimation.Nx);
    printf("Ny      = %d\n", d->estimation.Ny);

    printf("\nAntenna:\n");
    for (i = 0; i < d->estimation.Nx; i++)
    {
        for (j = 0; j < d->estimation.Ny; j++)
            printf("%3d ", d->estimation.Antenna[i][j]);
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    bool D1 = false;
    bool D2 = false;
    bool Model = false;
    int fd;
    struct BeamscanData data;
    ssize_t n;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "D1") == 0)
            D1 = true;
        else if (strcmp(argv[i], "D2") == 0)
            D2 = true;
        else if (strcmp(argv[i], "MODEL") == 0)
            Model = true;
    }
    printf("D1=%d D2=%d MODEL=%d\n", D1, D2, Model);
    fd = open("/dev/beamscan", O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }
    n = read(fd, &data, sizeof(data));
    if (n < 0)
    {
        perror("read");
        return 1;
    }

    if (n != sizeof(data))
    {
        printf("partial read: %zd bytes\n", n);
    }
    printf("n = %zd\n", n);

    close(fd);

    copyStruct(&data);

    printStruct(&datad);

    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        return 1;
    }

    shm = mmap(NULL,
               sizeof(SharedSignalArray),
               PROT_READ,
               MAP_SHARED,
               shm_fd,
               0);

    if (shm == MAP_FAILED)
    {
        perror("mmap");
        close(shm_fd);
        return 1;
    }

    T = TIME_NUMBER_STEPS + 1;
    InitScanScaleZone();
    // while (1)
    {
        // while (!shm->ready)
        //     usleep(100);

        if (D1)
        {
            Init_1DArrayPattern();
            computing_Rxx1D();
            compute_BEAMscan_1D();
#ifdef GRAFICS
            plot_BEAMscan_x();
            plot_BEAMscan_y();
#endif
        }
        if (D2)
        {
            Init_2DArrayPattern();
            computing_Rxx2D();
            compute_BEAMscan_2D();
#ifdef GRAFICS
            plot_BEAMscan_2D();
#endif

            // perror("Нет реализации 2D пространтвенного спектра");
        }
        // shm->ready = 0;
    }

    return 0;
}

/**
 * @brief Вычисляет одномерную автокорреляционную функцию Rxx для всей области моделирования.
 *
 * @details
 * Функция объединяет вычисления одномерных автокорреляционных функций
 * вдоль обеих осей (X и Y).
 * Обычно используется как универсальный вызов, который автоматически
 * выполняет анализ данных в обоих направлениях и сохраняет/выводит результаты.
 *
 * @note
 * Должна вызываться после инициализации параметров сетки (Nx, Ny, dx, dy)
 * и заполнения массивов исходных данных.
 *
 * @warning
 * Если входные данные не подготовлены или параметры сетки заданы неверно,
 * результаты корреляционного анализа будут недостоверны.
 *
 * @see computing_Rxx1D_X(), computing_Rxx1D_Y()
 */
void computing_Rxx1D()
{

/*  
    for (int i = 0; i < 10; i++)
    {
  
        printf("%2d %.12f%+.12fi\n", i, creal(shm->SignalArray[0][i]), cimag(shm->SignalArray[0][i]));
    }
*/

    int Nx = datad.estimation.Nx;
    int Ny = datad.estimation.Ny;

#ifdef CLEAR_MATRIX
    // Обнуляем матрицы
    for (int i = 0; i < Nx; i++)
        for (int j = 0; j < Nx; j++) {
           Rxx_ux[i][j] = 0.0 + 0.0 * I;
        }

     for (int i = 0; i < Ny; i++)
        for (int j = 0; j < Ny; j++) {
             Rxx_uy[i][j] = 0.0 + 0.0 * I;
        }
#endif

    // Основной цикл по времени
    for (int t = 0; t < T; t++)
    {
        /* -------- Rxx по X -------- */

        for (int x1 = 0; x1 < Nx; x1++)
        {
            for (int x2 = 0; x2 < Nx; x2++)
            {
                double complex sum = 0.0 + 0.0 * I;

                for (int y = 0; y < Ny; y++)
                {
                    double complex v1 = shm->SignalArray[t][y * Nx + x1];
                    double complex v2 = shm->SignalArray[t][y * Nx + x2];

                    sum += (v1) * conj(v2);
                }

                Rxx_ux[x1][x2] += sum;
            }
        }

        /* -------- Rxx по Y -------- */

        for (int y1 = 0; y1 < Ny; y1++)
        {
            for (int y2 = 0; y2 < Ny; y2++)
            {
                double complex sum = 0.0 + 0.0 * I;

                for (int x = 0; x < Nx; x++)
                {
                    double complex v1 = shm->SignalArray[t][y1 * Nx + x];
                    double complex v2 = shm->SignalArray[t][y2 * Nx + x];

                    sum += conj(v1) * v2;
                }

                Rxx_uy[y1][y2] += sum;
            }
        }
    }

#if 0
    // Нормировка
    double normX = (double)(T * Nx);
    double normY = (double)(T * Ny);

    for (int i = 0; i < Nx; i++)
        for (int j = 0; j < Nx; j++) {
            Rxx_ux[i][j] /= normX;
        }

    for (int i = 0; i < Ny; i++)
        for (int j = 0; j < Ny; j++) {
            Rxx_uy[i][j] /= normY;
        }
#endif

#ifdef DEBUG_OUT_MATRIX_CSV

    FILE *fux = fopen("/tmp/Rxx_ux.csv", "w");

    for (int i = 0; i < Nx; i++)
    {
        for (int j = 0; j < Nx; j++)
        {
            fprintf(fux, "%.15f%+.15fi",
                    creal(Rxx_ux[i][j]),
                    cimag(Rxx_ux[i][j]));

            if (j < Nx - 1)
                fputc('\t', fux);
        }
        fputc('\n', fux);
    }

    fclose(fux);

    FILE *fuy = fopen("/tmp/Rxx_uy.csv", "w");

    for (int i = 0; i < Ny; i++)
    {
        for (int j = 0; j < Ny; j++)
        {
            fprintf(fuy, "%.15f%+.15fi",
                    creal(Rxx_uy[i][j]),
                    cimag(Rxx_uy[i][j]));

            if (j < Ny - 1)
                fputc('\t', fuy);
        }
        fputc('\n', fuy);
    }

    fclose(fuy);

#endif
}

/**
 * @brief Вычисляет двумерную автокорреляционную функцию Rxx.
 *
 * @details
 * Функция реализует расчёт двумерной автокорреляции моделируемого поля/сигнала
 * по пространственным координатам X и Y. Такой анализ позволяет оценивать
 * пространственные зависимости и статистические свойства системы сразу в двух измерениях,
 * что важно для комплексных моделей и многомерных сигналов.
 *
 * @note
 * Перед вызовом функции необходимо корректно задать параметры сетки
 * (Nx, Ny, dx, dy) и подготовить массив данных для анализа.
 *
 * @warning
 * Некорректная инициализация сетки или отсутствие исходных данных
 * приведёт к неверным результатам вычислений.
 *
 * @see computing_Rxx1D(), computing_Rxx1D_X(), computing_Rxx1D_Y()
 */
void computing_Rxx2D(void)
{
    int M = datad.estimation.Nx * datad.estimation.Ny;

    /* Первая транспозиция: B_T = SignalArray.' */
    double complex B_T[M][T];

    for (int i = 0; i < M; i++)
    {
        for (int t = 0; t < T; t++)
        {
            B_T[i][t] = shm->SignalArray[t][i];
        }
    }

    /* Корреляционная матрица */
    double complex Rxx[M][M];

    for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++)
            Rxx[i][j] = 0.0 + 0.0 * I;

    /* Rxx = B_T * B_T^H */
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < M; j++)
        {
            double complex sum = 0.0 + 0.0 * I;

            for (int t = 0; t < T; t++)
            {
                sum += B_T[i][t] * conj(B_T[j][t]);
            }

            Rxx[i][j] = sum / (double)T;
        }
    }

#ifdef DEBUG_OUT_MATRIX_CSV

    FILE *fRxx = fopen("/tmp/Rxx.csv", "w");
    if (!fRxx)
    {
        perror("fopen");
        return;
    }

    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < M; j++)
        {
            fprintf(fRxx,
                    "%.15f%+.15fi",
                    creal(Rxx[i][j]),
                    cimag(Rxx[i][j]));

            if (j < M - 1)
                fputc('\t', fRxx);
        }
        fputc('\n', fRxx);
    }

    fclose(fRxx);

#endif
}

/**
 * @brief Инициализирует область сканирования и масштабирования.
 *
 * @details
 * Функция выполняет настройку параметров области моделирования,
 * в пределах которой будут выполняться вычисления или анализ сигнала.
 * Обычно используется для задания размеров зоны, шагов сетки и границ
 * по координатам X и Y. Также может подготавливать внутренние массивы
 * и структуры данных для последующего сканирования.
 *
 * @note
 * Должна вызываться перед запуском вычислительных функций
 * (например, computing_Rxx1D() или computing_Rxx2D()).
 *
 * @warning
 * Если параметры зоны заданы некорректно, вычисления в дальнейшем
 * будут давать неверные результаты или могут привести к выходу за пределы массивов.
 */
void InitScanScaleZone()
{
    int counter = 0;
    for (float i = datad.estimation.Tx_min; i <= datad.estimation.Tx_max; i = i + datad.estimation.Tx)
    {
        Thetta_range[counter] = i;
        counter++;
    }
    counter = 0;
    for (float i = datad.estimation.Ty_min; i <= datad.estimation.Ty_max; i += datad.estimation.Ty)
    {
        Phi_range[counter] = i;
        counter++;
    }
    // return STATE_OK;
}

/**
 * @brief Инициализирует диаграмму направленности двумерной антенной решётки.
 *
 * @details
 * Функция подготавливает двумерный массив, описывающий пространственное
 * распределение амплитудно-фазовых характеристик антенной решётки.
 * Используется для моделирования диаграммы направленности (pattern)
 * и расчёта её параметров, таких как уровень боковых лепестков, ширина
 * главного лепестка и усиление в заданных направлениях.
 *
 * @note
 * Вызывать эту функцию необходимо до выполнения процедур анализа или
 * вычисления корреляционных функций, так как диаграмма направленности
 * является исходными данными для симуляции.
 *
 * @warning
 * Некорректная инициализация может привести к ошибкам в моделировании
 * радиолокационных или коммуникационных систем, использующих данную решётку.
 */
void Init_2DArrayPattern(void)
{
    int Nx = datad.estimation.Nx;
    int Ny = datad.estimation.Ny;

    int lenTheta = (datad.estimation.Tx_max - datad.estimation.Tx_min) /
                       datad.estimation.Tx + 1;

    int lenPhi = (datad.estimation.Ty_max - datad.estimation.Ty_min) /
                     datad.estimation.Ty + 1;

    for (int i = 0; i < lenTheta; i++)
    {
        double ux = Thetta_range[i] / 100.0;

        for (int j = 0; j < lenPhi; j++)
        {
            double uy = Phi_range[j] / 100.0;

            for (int n = 0; n < Nx; n++)
            {
                for (int m = 0; m < Ny; m++)
                {
                    double phase_n = n - (Nx - 1.0) / 2.0;
                    double phase_m = m - (Ny - 1.0) / 2.0;

                    double angle =
                        M_PI * (phase_n * ux + phase_m * uy);

                    DigrArray[j][i][m][n] = cexp(I * angle);
                }
            }
        }
    }

#ifdef DEBUG_OUT_MATRIX_CSV

    FILE *f = fopen("/tmp/DigrArray.csv", "w");
    if (!f)
    {
        perror("fopen");
        return;
    }

    for (int jp = 0; jp < lenPhi; jp++)
    {
        for (int it = 0; it < lenTheta; it++)
        {
            for (int ny = 0; ny < Ny; ny++)
            {
                for (int nx = 0; nx < Nx; nx++)
                {
                    fprintf(f,
                            "%.15f%+.15fi\n",
                            creal(DigrArray[jp][it][ny][nx]),
                            cimag(DigrArray[jp][it][ny][nx]));
                }
            }
        }
    }

    fclose(f);

#endif
}

/**
 * @brief Инициализирует диаграмму направленности одномерной антенной решётки.
 *
 * @details
 * Функция формирует одномерный массив, описывающий диаграмму направленности (pattern)
 * линейной антенной решётки. Используется для моделирования характеристик излучения
 * или приёма вдоль одного пространственного направления.
 * Может включать задание амплитудно-фазового распределения, шага элементов
 * и количества излучателей.
 *
 * @note
 * Должна быть вызвана до вычисления характеристик решётки или анализа её
 * корреляционных свойств.
 *
 * @warning
 * Если параметры решётки заданы неверно (например, шаг элементов или количество
 * антенн), диаграмма направленности будет искажена, что приведёт к ошибкам
 * в дальнейших расчётах.
 */
void Init_1DArrayPattern(void)
{
    int Nx = datad.estimation.Nx;
    int Ny = datad.estimation.Ny;

    int lenTheta = (datad.estimation.Tx_max - datad.estimation.Tx_min) /
                       datad.estimation.Tx +
                   1;

    int lenPhi = (datad.estimation.Ty_max - datad.estimation.Ty_min) /
                     datad.estimation.Ty +
                 1;
    /* диаграмма по X */

    for (int i = 0; i < lenTheta; i++)
    {
        double theta = Thetta_range[i] / 100.0;

        for (int n = 0; n < Nx; n++)
        {
            double phase = (n - (Nx - 1.0) / 2.0) * theta * M_PI;

            Vnm_x[n][i] = cexp(I * phase);
        }
    }

#ifdef DEBUG_OUT_MATRIX_CSV

    FILE *fx = fopen("/tmp/DigrArray_X.csv", "w");
    if (!fx)
    {
        perror("fopen");
        return;
    }

    for (int it = 0; it < lenTheta; it++)
    {
        for (int nx = 0; nx < Nx; nx++)
        {
            fprintf(fx,
                    "%.15f%+.15fi\n",
                    creal(Vnm_x[nx][it]),
                    cimag(Vnm_x[nx][it]));
        }
    }

    fclose(fx);

#endif

    /* диаграмма по Y */

    for (int j = 0; j < lenPhi; j++)
    {
        double phi = Phi_range[j] / 100.0;

        for (int m = 0; m < Ny; m++)
        {
            double phase = (m - (Ny - 1.0) / 2.0) * phi * M_PI;

            Vnm_y[m][j] = cexp(I * phase);
        }
    }

#ifdef DEBUG_OUT_MATRIX_CSV

    FILE *fy = fopen("/tmp/DigrArray_Y.csv", "w");
    if (!fy)
    {
        perror("fopen");
        return;
    }

    for (int ny = 0; ny < Ny; ny++)
{
    for (int it = 0; it < lenPhi; it++)
    {
        fprintf(fy,
                "%.15f%+.15fi",
                creal(Vnm_y[ny][it]),
                cimag(Vnm_y[ny][it]));

        if (it < lenPhi - 1)
            fputc('\t', fy);
    }
    fputc('\n', fy);
}

    fclose(fy);

#endif
}

/**
 * @brief Выполняет двумерное сканирование луча антенной решётки.
 *
 * @details
 * Функция моделирует процесс сканирования диаграммы направленности
 * двумерной антенной решётки по обоим пространственным осям (X и Y).
 * Результатом является полная двумерная диаграмма направленности,
 * отражающая распределение усиления или чувствительности в пространстве.
 * Используется для анализа характеристик антенн в радиолокационных
 * и телекоммуникационных системах.
 *
 * @note
 * Перед вызовом необходимо:
 * - инициализировать диаграмму направленности (см. Init_2DArrayPattern());
 * - корректно задать размеры решётки, шаг дискретизации и параметры сканирования.
 *
 * @warning
 * Ошибки в параметрах сетки или шагах сканирования приведут к искажению
 * диаграммы направленности или к неверной оценке боковых лепестков.
 *
 * @see compute_BEAMscan_1D(), BeamScanX(), BeamScanY()
 */
void compute_BEAMscan_2D(void)
{
    int Nx = datad.estimation.Nx;
    int Ny = datad.estimation.Ny;

    int lenTheta = (datad.estimation.Tx_max - datad.estimation.Tx_min) /
                    datad.estimation.Tx + 1;

    int lenPhi = (datad.estimation.Ty_max - datad.estimation.Ty_min) /
                  datad.estimation.Ty + 1;

    int M = Nx * Ny;

    double max_mag = 0.0;

#pragma GCC ivdep
    for (int it = 0; it < lenTheta; it++)
    {
        for (int jp = 0; jp < lenPhi; jp++)
        {
            double complex a[M];

            int idx = 0;

            for (int m = 0; m < Ny; m++)
            {
                for (int n = 0; n < Nx; n++)
                {
                    a[idx++] = DigrArray[jp][it][m][n];
                }
            }

            double complex sum = 0.0 + 0.0 * I;

            for (int i = 0; i < M; i++)
            {
                double complex ai = conj(a[i]);

                for (int j = 0; j < M; j++)
                {
                    sum += ai * Rxx[i][j] * a[j];
                }
            }

            BEAMscan[jp][it] = sum;

            double mag = cabs(sum);

            if (mag > max_mag)
                max_mag = mag;
        }
    }

    /* Нормировка */

    if (max_mag > 0.0)
    {
        for (int jp = 0; jp < lenPhi; jp++)
        {
            for (int it = 0; it < lenTheta; it++)
            {
                BEAMscan[jp][it] /= max_mag;
            }
        }
    }

#ifdef DEBUG_OUT_MATRIX_CSV

    FILE *fB = fopen("/tmp/BEAMscan.csv", "w");
    if (!fB)
    {
        perror("fopen");
        return;
    }

    for (int jp = 0; jp < lenPhi; jp++)
    {
        for (int it = 0; it < lenTheta; it++)
        {
            fprintf(fB,
                    "%.15f%+.15fi",
                    creal(BEAMscan[jp][it]),
                    cimag(BEAMscan[jp][it]));

            if (it < lenTheta - 1)
                fputc('\t', fB);
        }

        fputc('\n', fB);
    }

    fclose(fB);

#endif
}


/**
 * @brief Выполняет одномерное сканирование луча антенной решётки.
 *
 * @details
 * Функция рассчитывает диаграмму направленности (pattern) при сканировании
 * луча вдоль одного измерения (X или Y). Используется для анализа свойств
 * линейных и двумерных антенных решёток в сечении, определения положения
 * главного лепестка, ширины диаграммы и уровней боковых лепестков.
 *
 * @note
 * Перед вызовом необходимо инициализировать массив диаграммы направленности
 * (см. Init_1DArrayPattern() или Init_2DArrayPattern()) и задать параметры
 * сканирования (шаг угла, количество элементов решётки и др.).
 *
 * @warning
 * Ошибки в параметрах (например, шаге сканирования или числе элементов)
 * приведут к искажённым результатам или некорректному положению главного лепестка.
 *
 * @see compute_BEAMscan_2D(), BeamScanX(), BeamScanY()
 */
void compute_BEAMscan_1D(void)
{
    int Nx = datad.estimation.Nx;
    int Ny = datad.estimation.Ny;

    int lenTheta = (datad.estimation.Tx_max - datad.estimation.Tx_min) /
                    datad.estimation.Tx + 1;

    int lenPhi = (datad.estimation.Ty_max - datad.estimation.Ty_min) /
                  datad.estimation.Ty + 1;

    /* ---------- BEAMscan_x ---------- */

    for (int i = 0; i < lenTheta; i++)
    {
        double complex sum = 0.0 + 0.0 * I;

        for (int m = 0; m < Nx; m++)
        {
            for (int n = 0; n < Nx; n++)
            {
                sum += conj(Vnm_x[n][i]) *
                       Rxx_ux[n][m] *
                       Vnm_x[m][i];
            }
        }

        BEAMscan_x[i] = sum;
    }

    /* ---------- BEAMscan_y ---------- */

    for (int j = 0; j < lenPhi; j++)
    {
        double complex sum = 0.0 + 0.0 * I;

        for (int p = 0; p < Ny; p++)
        {
            for (int q = 0; q < Ny; q++)
            {
                sum += conj(Vnm_y[q][j]) *
                       Rxx_uy[q][p] *
                       Vnm_y[p][j];
            }
        }

        BEAMscan_y[j] = sum;
    }

#ifdef DEBUG_OUT_MATRIX_CSV

    FILE *fx = fopen("/tmp/BEAMscan_x.csv", "w");
    if (fx)
    {
        for (int i = 0; i < lenTheta; i++)
        {
            fprintf(fx,
                    "%.15f%+.15fi\n",
                    creal(BEAMscan_x[i]),
                    cimag(BEAMscan_x[i]));
        }
        fclose(fx);
    }

    FILE *fy = fopen("/tmp/BEAMscan_y.csv", "w");
    if (fy)
    {
        for (int j = 0; j < lenPhi; j++)
        {
            fprintf(fy,
                    "%.15f%+.15fi\n",
                    creal(BEAMscan_y[j]),
                    cimag(BEAMscan_y[j]));
        }
        fclose(fy);
    }

#endif
}


#ifdef GRAFICS

/**
 * @brief Строит график диаграммы направленности при сканировании по оси Y.
 *
 * @details
 * Функция визуализирует результаты двумерного или одномерного сканирования
 * луча антенной решётки в направлении оси Y. Обычно отображает зависимость
 * усиления (или нормированной мощности) от угла отклонения.
 * Используется для анализа характеристик диаграммы направленности:
 * ширины главного лепестка, уровня боковых лепестков, положения максимума.
 *
 * @note
 * Перед вызовом должны быть рассчитаны данные сканирования
 * (см. compute_BEAMscan_1D() или compute_BEAMscan_2D()).
 *
 * @warning
 * Если данные для оси Y не были предварительно вычислены или массив
 * результатов не инициализирован, функция построит некорректный график
 * либо вызовет ошибку доступа к памяти.
 *
 * @see plot_BEAMscan_x(), compute_BEAMscan_1D(), compute_BEAMscan_2D()
 */
void plot_BEAMscan_y()
{
    int lenPhi = (datad.estimation.Ty_max - datad.estimation.Ty_min) /
                     datad.estimation.Ty + 1;

    double maxMag = 0.0;
    for (int j = 0; j < lenPhi; j++) {
        double mag = cabs(BEAMscan_y[j]);
        if (mag > maxMag) maxMag = mag;
    }

    FILE *f = fopen("beam_y.dat", "w");
    for (int j = 0; j < lenPhi; j++)
    {
        double mag = 10 * log10(BEAMscan_y[j]/ maxMag)  ;
        fprintf(f, "%f %f\n", Phi_range[j], mag);
    }
    fclose(f);

    // создаем скрипт для gnuplot
    FILE *gp = popen("gnuplot -persist", "w");
    if (gp)
    {
        fprintf(gp, "set title 'BEAMscan_y'\n");
        fprintf(gp, "set xlabel 'Phi (градусы)'\n");
        fprintf(gp, "set ylabel 'Amplitude'\n");
        fprintf(gp, "plot 'beam_y.dat' with lines title 'BEAMscan_y'\n");
        fflush(gp);
        // pclose(gp); // если хочешь закрыть окно автоматически
    }
}

/**
 * @brief Строит график диаграммы направленности при сканировании по оси X.
 *
 * @details
 * Функция визуализирует результаты сканирования луча антенной решётки
 * в направлении оси X. Отображает зависимость усиления или нормированной
 * мощности от угла отклонения по горизонтальной оси.
 * Используется для анализа характеристик диаграммы направленности:
 * положения главного лепестка, ширины диаграммы, уровней боковых лепестков
 * и симметрии распределения.
 *
 * @note
 * Перед вызовом должны быть рассчитаны данные для оси X
 * (см. compute_BEAMscan_1D() или compute_BEAMscan_2D()).
 *
 * @warning
 * Если массив данных для оси X не инициализирован или повреждён,
 * построенный график будет некорректным либо произойдёт ошибка доступа к памяти.
 *
 * @see plot_BEAMscan_y(), compute_BEAMscan_1D(), compute_BEAMscan_2D()
 */
void plot_BEAMscan_x()
{   
    
    int lenTheta = (datad.estimation.Tx_max - datad.estimation.Tx_min) /
                       datad.estimation.Tx + 1;

    double maxMag = 0.0;
    for (int j = 0; j < lenTheta; j++) {
        double mag = cabs(BEAMscan_x[j]);
        if (mag > maxMag) maxMag = mag;
    }

    FILE *f = fopen("beam_x.dat", "w");
    for (int j = 0; j < lenTheta; j++)
    {
        double mag = 10 * log10(BEAMscan_x[j]/ maxMag)  ;
        fprintf(f, "%f %f\n", Thetta_range[j], mag);
    }
    fclose(f);

    // скрипт для gnuplot
    FILE *gp = popen("gnuplot -persist", "w");
    if (gp)
    {
        fprintf(gp, "set title 'BEAMscan_x'\n");
        fprintf(gp, "set xlabel 'Thetta (градусы)'\n");
        fprintf(gp, "set ylabel 'Amplitude'\n");
        fprintf(gp, "plot 'beam_x.dat' with lines title 'BEAMscan_x'\n");
        fflush(gp);
        // pclose(gp); // закрыть окно автоматически
    }
}

/**
 * @brief Строит двумерный график диаграммы направленности антенной решётки.
 *
 * @details
 * Функция визуализирует результаты сканирования луча антенной решётки
 * в двухмерной плоскости (θ и φ). Отображает зависимость амплитуды
 * (или нормированной мощности) от углового положения по горизонтальной
 * и вертикальной осям. Используется для анализа характеристик
 * диаграммы направленности: положения главного лепестка, ширины диаграммы,
 * уровней боковых лепестков и симметрии распределения.
 *
 * @note
 * Перед вызовом должны быть рассчитаны данные для двумерного сканирования
 * (см. compute_BEAMscan_2D()). Функция автоматически вычисляет модуль
 * комплексных значений BEAMscan и нормирует данные.
 *
 * @warning
 * Если массив BEAMscan не инициализирован или повреждён, построенный
 * график будет некорректным либо произойдёт ошибка доступа к памяти.
 *
 * @see plot_BEAMscan_x(), plot_BEAMscan_y(), compute_BEAMscan_2D()
 */

#if 1
void plot_BEAMscan_2D()
{
    FILE *f = fopen("beam_2D.dat", "w");
    if (!f)
    {
        perror("Cannot open file for 2D plot");
        return;
    }
    int lenPhi = (datad.estimation.Ty_max - datad.estimation.Ty_min) /
                     datad.estimation.Ty + 1;
    int lenTheta = (datad.estimation.Tx_max - datad.estimation.Tx_min) /
                       datad.estimation.Tx + 1;
    // Записываем данные: theta phi magnitude
    for (int it = 0; it < lenTheta; ++it)
    {
        for (int jp = 0; jp < lenPhi; ++jp)
        {
            double mag = creal(BEAMscan[jp][it]);
            fprintf(f, "%f %f %f\n", Thetta_range[it], Phi_range[jp], mag);
        }
        fprintf(f, "\n"); // пустая строка для корректного отображения сетки
    }
    fclose(f);

    FILE *gp = popen("gnuplot -persist", "w");
    if (gp)
    {
        fprintf(gp, "set title 'BEAMscan 2D'\n");
        fprintf(gp, "set xlabel 'Theta (градусы)'\n");
        fprintf(gp, "set ylabel 'Phi (градусы)'\n");
        fprintf(gp, "set zlabel 'Amplitude'\n");

        // Цветовая карта "jet" (синяя -> зеленая -> желтая -> красная)
        fprintf(gp, "set palette defined (0 'blue', 1 'cyan', 2 'green', 3 'yellow', 4 'red')\n");
        fprintf(gp, "set pm3d map\n");
        fprintf(gp, "splot 'beam_2D.dat' using 1:2:3 with pm3d notitle\n");

        fflush(gp);
        // pclose(gp);
    }
}
#endif

#endif