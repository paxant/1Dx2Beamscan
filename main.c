#include "main.h"
#include <linux/init.h> /* Необходим для макросов */

/*
Макрос __init приводит к отбрасыванию функции инициализации и освобождению
занятой ей памяти по завершении её выполнения для встроенных драйверов, но не
загружаемых модулей. И это вполне разумно, если учесть, когда эта функция вызывается.
Также есть __initdata, которая работает аналогично __init, но для переменных
инициализации, а не для функций.
Макрос __exit приводит к пропуску функции, если модуль встроен в ядро, то есть
аналогично __init не влияет на загружаемые модули.
*/

/*
rmmod main
insmod main.ko

*/

/* module_param_array(name, type, num, perm);
* Первым аргументом идёт имя параметра (в данном случае массива).
* Второй аргумент – это тип элементов массива.
* Третий – это указатель на переменную, которая будет хранить количество элементов
массива, инициализированных пользователем при загрузке модуля.
* Четвёртый аргумент – это биты разрешения.
*/

// Включение параметров
static bool D1 = false;
static bool D2 = false;
static bool Model = false;

// Переменные

extern struct Estimation estimation;
extern struct Modeling modeling;

/* Символьное устройство */
static dev_t beamscan_dev;
static struct cdev beamscan_cdev;
static struct class *beamscan_class;

static int beamscan_open(struct inode *inode, struct file *file)
{
    return 0;
}
static int beamscan_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t beamscan_read(struct file *file,
                             char __user *buf,
                             size_t len,
                             loff_t *off)
{
    struct BeamscanData data;

    if (*off != 0)
        return 0;

    data.estimation = estimation;
    data.modeling = modeling;

    if (len < sizeof(data))
        return -EINVAL;

    if (copy_to_user(buf, &data, sizeof(data)))
        return -EFAULT;

    *off += sizeof(data);

    return sizeof(data);
}

static const struct file_operations beamscan_fops = {
    .owner   = THIS_MODULE,
    .open    = beamscan_open,
    .read    = beamscan_read,
    .release = beamscan_release,
};

static int start_beamscan(void)
{
    char *argv[5];
    int argc = 0;

    argv[argc++] = "/usr/local/bin/beamscan";

    if (D1)
        argv[argc++] = "D1";

    if (D2)
        argv[argc++] = "D2";

    if (Model)
        argv[argc++] = "MODEL";

    argv[argc] = NULL;

    static char *envp[] = {
        "HOME=/",
        "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
        NULL
    };

    return call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
}

static int start_beamscan_model(void)
{
    char *argv[5];
    int argc = 0;

    argv[argc++] = "/usr/local/bin/beamscan_model";

    if (D1)
        argv[argc++] = "D1";

    if (D2)
        argv[argc++] = "D2";

    if (Model)
        argv[argc++] = "MODEL";

    argv[argc] = NULL;

    static char *envp[] = {
        "HOME=/",
        "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
        NULL
    };

    return call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
}

static int __init hello_init(void)
{
    int ret;

    pr_info("Mode 1D: %d\n", D1);
    pr_info("Mode 2D: %d\n", D2);
    pr_info("Model: %d\n", Model);
    pr_info("Init BeamScan driver\n");

    ret = read_params_from_file("/etc/configs/Beam.params");
    if (ret)
        return ret;

    ret = read_array_from_file("/etc/configs/ArrayStruct.params");
    if (ret)
        return ret;

    /* Регистрация устройства */

    ret = alloc_chrdev_region(&beamscan_dev, 0, 1, "beamscan");
    if (ret) {
        pr_err("alloc_chrdev_region failed\n");
        return ret;
    }

    cdev_init(&beamscan_cdev, &beamscan_fops);
    beamscan_cdev.owner = THIS_MODULE;

    ret = cdev_add(&beamscan_cdev, beamscan_dev, 1);
    if (ret) {
        pr_err("cdev_add failed\n");
        unregister_chrdev_region(beamscan_dev, 1);
        return ret;
    }

    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
        beamscan_class = class_create("beamscan");
    #else
        beamscan_class = class_create(THIS_MODULE, "beamscan");
    #endif
    if (IS_ERR(beamscan_class)) {
        ret = PTR_ERR(beamscan_class);

        cdev_del(&beamscan_cdev);
        unregister_chrdev_region(beamscan_dev, 1);

        return ret;
    }

    if (IS_ERR(device_create(beamscan_class,
                             NULL,
                             beamscan_dev,
                             NULL,
                             "beamscan"))) {

        class_destroy(beamscan_class);
        cdev_del(&beamscan_cdev);
        unregister_chrdev_region(beamscan_dev, 1);

        return -ENOMEM;
    }

    pr_info("/dev/beamscan created\n");

    /* Запуск пользовательской программы */

    ret = start_beamscan_model();
    if (ret)
        pr_err("Cannot start beamscan_model (%d)\n", ret);
    else
        pr_info("beamscan_model userspace started\n");

    ret = start_beamscan();
    if (ret)
        pr_err("Cannot start beamscan (%d)\n", ret);
    else
        pr_info("beamscan userspace started\n");

    return 0;
}

static void __exit hello_exit(void)
{
    device_destroy(beamscan_class, beamscan_dev);
    class_destroy(beamscan_class);

    cdev_del(&beamscan_cdev);

    unregister_chrdev_region(beamscan_dev, 1);

    pr_info("BeamScan driver unloaded\n");
}

module_param(D1, bool, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(D1, "A bool");
module_param(D2, bool, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(D2, "A bool");
module_param(Model, bool, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(Model, "A bool");

module_init(hello_init);
module_exit(hello_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paxant");
MODULE_DESCRIPTION("A Beamscan driver");