#include <linux/init.h> /* Необходим для макросов */
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kmod.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kernel.h> /* необходим для pr_info() */
#include <linux/module.h> /* необходим для всех модулей */


#define BUF_SIZE 4096
#define SCALE 1000


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

struct Estimation
{
    s64 dx;
    s64 dy;
    s64 Tx;
    s64 Ty;
    s64 Tx_min;
    s64 Tx_max;
    s64 Ty_min;
    s64 Ty_max;
    int Nx;
    int Ny;
    int Antenna[NUMBER_X][NUMBER_Y];
};

struct Modeling 
{
    char SNR[32];
    s64 T_start;
    s64 T_step; 
    s64 T_end;
    s64 TimeSimul[255];
    int N_Signal;
    s64 F[64];
    s64 x[64];
    s64 y[64];
    s64 A[64];
};

struct BeamscanData {
    struct Estimation estimation;
    struct Modeling modeling;
};

// чтение файла конфигураций
int read_params_from_file(const char *filename);
int read_array_from_file(const char *filename);