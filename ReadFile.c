#include "main.h"

// Переменные

struct Estimation estimation;
struct Modeling modeling;

int T = 0; // число временных отсчётов
int M = 0;

int lenPhi = 0;
int lenTheta = 0;

static void print_fixed(const char *name, s64 value)
{
    s64 integer = value / SCALE;
    s64 frac = value % SCALE;

    if (frac < 0)
        frac = -frac;

    pr_info("%s=%lld.%03lld\n",
            name,
            (long long)integer,
            (long long)frac);
}

static s64 parse_fixed(const char *str)
{
    s64 integer = 0;
    s64 frac = 0;
    int sign = 1;
    int digits = 0;

    if (*str == '-') {
        sign = -1;
        str++;
    }

    while (*str && *str != '.') {
        integer = integer * 10 + (*str - '0');
        str++;
    }

    if (*str == '.')
        str++;

    while (*str && digits < 3) {
        frac = frac * 10 + (*str - '0');
        str++;
        digits++;
    }

    while (digits < 3) {
        frac *= 10;
        digits++;
    }

    return sign * (integer * SCALE + frac);
}

int read_params_from_file(const char *filename)
{
    struct file *f;
    char *buf;
    char *cur;
    char *line;
    loff_t pos = 0;
    ssize_t size;

    f = filp_open(filename, O_RDONLY, 0);
    if (IS_ERR(f)) {
        pr_err("Cannot open config file\n");
        return PTR_ERR(f);
    }

    buf = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!buf) {
        filp_close(f, NULL);
        return -ENOMEM;
    }

    size = kernel_read(f, buf, BUF_SIZE - 1, &pos);
    if (size < 0) {
        pr_err("Cannot read config file\n");
        kfree(buf);
        filp_close(f, NULL);
        return size;
    }

    buf[size] = '\0';

    cur = buf;

    while ((line = strsep(&cur, "\n")) != NULL) {

        char key[32];
        char value[32];

        if (sscanf(line, "%31s %31s", key, value) != 2)
            continue;

        //printk(KERN_INFO "%s = %s\n", key, value);

        if (!strcmp(key, "dx"))
            estimation.dx = parse_fixed(value);

        else if (!strcmp(key, "dy"))
            estimation.dy = parse_fixed(value);

        else if (!strcmp(key, "Nx"))
            kstrtoint(value, 10, &estimation.Nx);

        else if (!strcmp(key, "Ny"))
            kstrtoint(value, 10, &estimation.Ny);

        else if (!strcmp(key, "Tx"))
            estimation.Tx = parse_fixed(value);

        else if (!strcmp(key, "Ty"))
            estimation.Ty = parse_fixed(value);

        else if (!strcmp(key, "Tx_min"))
            estimation.Tx_min = parse_fixed(value);

        else if (!strcmp(key, "Tx_max"))
            estimation.Tx_max = parse_fixed(value);

        else if (!strcmp(key, "Ty_min"))
            estimation.Ty_min = parse_fixed(value);

        else if (!strcmp(key, "Ty_max"))
            estimation.Ty_max = parse_fixed(value);

        else if (!strcmp(key, "N_Signal"))
            kstrtoint(value, 10, &modeling.N_Signal);

        else if (!strcmp(key, "T_start"))
            modeling.T_start = parse_fixed(value);

        else if (!strcmp(key, "T_step"))
            modeling.T_step = parse_fixed(value);

        else if (!strcmp(key, "T_end"))
            modeling.T_end = parse_fixed(value);

        else if (!strcmp(key, "SNR"))
            strscpy(modeling.SNR, value, sizeof(modeling.SNR));
        else if (key[0] == 'F') {
            int idx;

            if (!kstrtoint(key + 1, 10, &idx) &&
                idx >= 1 && idx <= ARRAY_SIZE(modeling.F))
                modeling.F[idx - 1] = parse_fixed(value);
        }

        else if (key[0] == 'x') {
            int idx;

            if (!kstrtoint(key + 1, 10, &idx) &&
                idx >= 1 && idx <= ARRAY_SIZE(modeling.x))
                modeling.x[idx - 1] = parse_fixed(value);
        }

        else if (key[0] == 'y') {
            int idx;

            if (!kstrtoint(key + 1, 10, &idx) &&
                idx >= 1 && idx <= ARRAY_SIZE(modeling.y))
                modeling.y[idx - 1] = parse_fixed(value);
        }

        else if (key[0] == 'A') {
            int idx;

            if (!kstrtoint(key + 1, 10, &idx) &&
                idx >= 1 && idx <= ARRAY_SIZE(modeling.A))
                modeling.A[idx - 1] = parse_fixed(value);
        }
    }
    
    kfree(buf);
    filp_close(f, NULL);

    print_fixed("dx", estimation.dx);
    print_fixed("dy", estimation.dy);
    print_fixed("Tx", estimation.Tx);
    print_fixed("Ty", estimation.Ty);
    print_fixed("Tx_min", estimation.Tx_min);
    print_fixed("Tx_max", estimation.Tx_max);
    print_fixed("Ty_min", estimation.Ty_min);
    print_fixed("Ty_max", estimation.Ty_max);
    pr_info("Параметры моделирования сигнала");
    print_fixed("T_start", modeling.T_start);
    print_fixed("T_step", modeling.T_step);
    print_fixed("T_end", modeling.T_end);

    for (int i = 0; i < modeling.N_Signal; i++) {
        pr_info("Signal %d: F=%lld.%03lld x=%lld.%03lld y=%lld.%03lld A=%lld.%03lld\n",
        i + 1,
        (long long)(modeling.F[i] / SCALE), (long long)abs(modeling.F[i] % SCALE),
        (long long)(modeling.x[i] / SCALE), (long long)abs(modeling.x[i] % SCALE),
        (long long)(modeling.y[i] / SCALE), (long long)abs(modeling.y[i] % SCALE),
        (long long)(modeling.A[i] / SCALE), (long long)abs(modeling.A[i] % SCALE));
    }
    return 0;
}

int read_array_from_file(const char *filename)
{
    struct file *f;
    char *buf;
    char *cur;
    char *line;
    loff_t pos = 0;
    ssize_t size;
    int row = 0;
    int cols = 0;

    f = filp_open(filename, O_RDONLY, 0);
    if (IS_ERR(f)) {
        pr_err("Cannot open %s\n", filename);
        return PTR_ERR(f);
    }

    buf = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!buf) {
        filp_close(f, NULL);
        return -ENOMEM;
    }

    size = kernel_read(f, buf, BUF_SIZE - 1, &pos);
    if (size < 0) {
        kfree(buf);
        filp_close(f, NULL);
        return size;
    }

    buf[size] = '\0';

    cur = buf;

    while ((line = strsep(&cur, "\n")) != NULL) {

        char *token;
        int col = 0;

        /* убрать '\r', если файл создан в Windows */
        token = strchr(line, '\r');
        if (token)
            *token = '\0';

        token = line;

        while ((token = strsep(&line, ",")) != NULL) {

            int value;

            if (*token == '\0')
                continue;

            if (row >= NUMBER_X || col >= NUMBER_Y) {
                pr_err("Array is too large\n");
                goto out;
            }

            if (kstrtoint(token, 10, &value)) {
                pr_err("Bad value '%s' at [%d][%d]\n",
                       token, row, col);
                goto out;
            }

            estimation.Antenna[row][col] = value;
            col++;
        }

        /* пропускаем пустые строки */
        if (col == 0)
            continue;

        /* запоминаем число столбцов первой строки */
        if (row == 0)
            cols = col;
        else if (col != cols) {
            pr_err("Row %d has %d columns, expected %d\n",
                   row, col, cols);
            goto out;
        }

        row++;
    }

    pr_info("Loaded antenna array %d x %d\n", row, cols);
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < cols; j++)
            pr_cont("%d ", estimation.Antenna[i][j]);

    pr_cont("\n");
}
out:
    kfree(buf);
    filp_close(f, NULL);
    return 0;
}