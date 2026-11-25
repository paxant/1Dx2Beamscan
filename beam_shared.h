#include <linux/kernel.h> /* необходим для pr_info() */
#include <linux/module.h> /* необходим для всех модулей */

struct Estimation
{
    int64_t dx;
    int64_t dy;
    int64_t Tx;
    int64_t Ty;
    int64_t Tx_min;
    int64_t Tx_max;
    int64_t Ty_min;
    int64_t Ty_max;
    int Nx;
    int Ny;
    int Antenna[NUMBER_X][NUMBER_Y];
};

struct Estimation_double
{
    double dx;
    double dy;
    double Tx;
    double Ty;
    double Tx_min;
    double Tx_max;
    double Ty_min;
    double Ty_max;
    int Nx;
    int Ny;
    int Antenna[NUMBER_X][NUMBER_Y];
};

struct Modeling_double
{
    char SNR[32];
    double T_start;
    double T_step; 
    double T_end;
    double TimeSimul[255];
    int N_Signal;
    double F[64];
    double x[64];
    double y[64];
    double A[64];
};

struct BeamscanData_double {
    struct Estimation_double estimation;
    struct Modeling_double modeling;
};

struct Modeling 
{
    char SNR[32];
    int64_t T_start;
    int64_t T_step; 
    int64_t T_end;
    int64_t TimeSimul[255];
    int N_Signal;
    int64_t F[64];
    int64_t x[64];
    int64_t y[64];
    int64_t A[64];
};

struct BeamscanData {
    struct Estimation estimation;
    struct Modeling modeling;
};

// чтение файла конфигураций
int read_params_from_file(const char *filename);
int read_array_from_file(const char *filename);