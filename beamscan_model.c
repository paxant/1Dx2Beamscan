#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <complex.h>
#include <math.h>
#include "shared_memory.h"

#define TIME_NUMBER_STEPS (int)((datad.modeling.T_end - datad.modeling.T_start) / datad.modeling.T_step)

struct BeamscanData_double datad;
int M = 0;

void copyStruct(const struct BeamscanData *src)
{
    int i, j;

    datad.estimation.dx     = (double)src->estimation.dx / 1000.0;
    datad.estimation.dy     = (double)src->estimation.dy / 1000.0;
    datad.estimation.Tx     = (double)src->estimation.Tx / 1000.0;
    datad.estimation.Ty     = (double)src->estimation.Ty / 1000.0;

    datad.estimation.Tx_min = (double)src->estimation.Tx_min / 1000.0;
    datad.estimation.Tx_max = (double)src->estimation.Tx_max / 1000.0;
    datad.estimation.Ty_min = (double)src->estimation.Ty_min / 1000.0;
    datad.estimation.Ty_max = (double)src->estimation.Ty_max / 1000.0;

    datad.estimation.Nx = src->estimation.Nx;
    datad.estimation.Ny = src->estimation.Ny;

    for (i = 0; i < NUMBER_X; i++)
        for (j = 0; j < NUMBER_Y; j++)
            datad.estimation.Antenna[i][j] =
                src->estimation.Antenna[i][j];

    memcpy(datad.modeling.SNR,
           src->modeling.SNR,
           sizeof(datad.modeling.SNR));

    datad.modeling.T_start = (double)src->modeling.T_start / 1000.0;
    datad.modeling.T_step  = (double)src->modeling.T_step  / 1000.0;
    datad.modeling.T_end   = (double)src->modeling.T_end   / 1000.0;

    datad.modeling.N_Signal = src->modeling.N_Signal;

    for (i = 0; i < 255; i++)
        datad.modeling.TimeSimul[i] =
            (double)src->modeling.TimeSimul[i] / 1000.0;

    for (i = 0; i < 64; i++) {
        datad.modeling.F[i] = (double)src->modeling.F[i] / 1000.0;
        datad.modeling.x[i] = (double)src->modeling.x[i] / 1000.0;
        datad.modeling.y[i] = (double)src->modeling.y[i] / 1000.0;
        datad.modeling.A[i] = (double)src->modeling.A[i] / 1000.0;
    }
}

void printStruct(const struct BeamscanData_double *d)
{
    
    int i, j;

    printf("\n=========== Modeling ============\n");

    printf("SNR      = %s\n", d->modeling.SNR);
    printf("N_Signal = %d\n", d->modeling.N_Signal);

    printf("T_start  = %.3f\n", d->modeling.T_start);
    printf("T_step   = %.3f\n", d->modeling.T_step);
    printf("T_end    = %.3f\n", d->modeling.T_end);

    printf("\nTimeSimul:\n");
    for (i = 0; i < 255; i++)
        printf("[%3d] %.3f\n", i, d->modeling.TimeSimul[i]);

    printf("\nF:\n");
    for (i = 0; i < d->modeling.N_Signal; i++)
        printf("[%2d] %.3f\n", i, d->modeling.F[i]);

    printf("\nx:\n");
    for (i = 0; i < d->modeling.N_Signal; i++)
        printf("[%2d] %.3f\n", i, d->modeling.x[i]);

    printf("\ny:\n");
    for (i = 0; i < d->modeling.N_Signal; i++)
        printf("[%2d] %.3f\n", i, d->modeling.y[i]);

    printf("\nA:\n");
    for (i = 0; i < d->modeling.N_Signal; i++)
        printf("[%2d] %.3f\n", i, d->modeling.A[i]);
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
        else{
            printf("Пустые аргументы");
            return 0;
        }
    }
    
    fd = open("/dev/beamscan", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    n = read(fd, &data, sizeof(data));
    if (n < 0) {
        perror("read");
        return 1;
    }

    if (n != sizeof(data)) {
        printf("partial read: %zd bytes\n", n);
    }
    printf("n = %zd\n", n);
    
    close(fd);

    copyStruct(&data);

    printStruct(&datad);

    // Моделируем сигнал плоского фронта электромагнитной волны
    InitSimulationSignal();

    
    return 0;
}

/**
 * @brief Инициализирует сигналы для симуляции.
 *
 * @details
 * Функция выполняет настройку необходимых обработчиков сигналов,
 * которые используются в процессе моделирования. Это может включать
 * регистрацию пользовательских функций для обработки прерываний,
 * остановки симуляции или корректного завершения программы.
 *
 * @note
 * Должна вызываться один раз в начале работы программы,
 * перед запуском основных вычислений или потоков.
 *
 * @warning
 * Неправильная инициализация сигналов может привести к некорректному
 * завершению программы или утечкам ресурсов.
 */
void InitSimulationSignal()
{   

    printf("\nРасчет модели сигнала\n");
    int counter = 0;
    M = datad.estimation.Nx * datad.estimation.Ny;
    for (double Time = datad.modeling.T_start; Time <= datad.modeling.T_end; Time += datad.modeling.T_step)
    {
        datad.modeling.TimeSimul[counter] = Time;
        counter++;
    }
    int T = TIME_NUMBER_STEPS + 1;

    double complex SinSignal[NUMBER_SIGNALS][SNAPSHOTS];

    for (int i = 0; i < datad.modeling.N_Signal; i++)
        for (int j = 0; j <= T - 1; j++)
            SinSignal[i][j] = datad.modeling.A[i] * cexp(I * 2.0 * M_PI * datad.modeling.F[i] * datad.modeling.TimeSimul[j]);

#ifdef DEBUG_OUT_MATRIX_CSV

        FILE *f = fopen("/tmp/signals.csv", "w");
        if (!f){
            return 1;
            printf("Не удалось открыть файл");
        }

        for (int i = 0; i < datad.modeling.N_Signal; i++)
        {
            for (int j = 0; j <= TIME_NUMBER_STEPS; j++)
            {
                fprintf(f, "%.15f%+.15fi",
                        creal(SinSignal[i][j]),
                        cimag(SinSignal[i][j]));

                if (j < TIME_NUMBER_STEPS)
                    fputc('\t', f);
            }
            fputc('\n', f);
        }

        fclose(f);
#endif
    double complex V[NUMBER_X * NUMBER_Y][NUMBER_SIGNALS];

    for (int i = 0; i < datad.modeling.N_Signal; i++)
    {
        int index_matrix = 0;

        for (int ny = 0; ny < datad.estimation.Ny; ny++)
            for (int nx = 0; nx < datad.estimation.Nx; nx++)
            {
                double angle_y = (ny - (double)(datad.estimation.Ny - 1) / 2.0) * datad.modeling.y[i] / 100.0 * M_PI;

                double angle_x = (nx - (double)(datad.estimation.Nx - 1) / 2.0) * datad.modeling.x[i] / 100.0 * M_PI;

                V[index_matrix][i] = cexp(I * (angle_x + angle_y));

                index_matrix++;
            }
    }

#ifdef DEBUG_OUT_MATRIX_CSV

    FILE *fV = fopen("/tmp/V.csv", "w");
    if (!f){
            return 1;
            printf("Не удалось открыть файл");
        }

    for (int row = 0; row < M; row++)
    {
        for (int col = 0; col < datad.modeling.N_Signal; col++)
        {
            fprintf(fV, "%.15f%+.15fi",
                    creal(V[row][col]),
                    cimag(V[row][col]));

            if (col < datad.modeling.N_Signal - 1)
                fputc('\t', fV);
        }
        fputc('\n', fV);
    }

    fclose(fV);

#endif
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return;
    }

    if (ftruncate(shm_fd, sizeof(SharedSignalArray)) == -1) {
        perror("ftruncate");
        close(shm_fd);
        return;
    }

    SharedSignalArray *shm = mmap(NULL,
                                sizeof(SharedSignalArray),
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED,
                                shm_fd,
                                0);

    if (shm == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return;
    }

    for (int t = 0; t < T; t++)              // по времени
    {
        for (int m = 0; m < datad.estimation.Nx * datad.estimation.Ny; m++)    // по элементам решётки
        {
            double complex sum = 0.0 + 0.0*I;

            for (int s = 0; s < datad.modeling.N_Signal; s++)   // по сигналам
            {
                sum += V[m][s] * SinSignal[s][t];
            }

            shm->SignalArray[t][m] = sum;
        }
    }
#ifdef DEBUG_OUT_MATRIX_CSV

    FILE *fSA = fopen("/tmp/SignalArray.csv", "w");
    if (!fSA) {
        perror("fopen");
        return 1;
    }

    for (int t = 0; t < T; t++)
    {
        for (int m = 0; m < datad.estimation.Nx * datad.estimation.Ny; m++)
        {
            fprintf(f, "%.15f%+.15fi",
                    creal(shm->SignalArray[t][m]),
                    cimag(shm->SignalArray[t][m]));

            if (m < datad.estimation.Nx * datad.estimation.Ny - 1)
                fputc('\t', fSA);
        }

        fputc('\n', fSA);
    }

    fclose(fSA);

#endif
    shm->ready = 1;
    usleep(1000);
    shm->ready = 0;

    munmap(shm, sizeof(SharedSignalArray));
    close(shm_fd);
}