#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "led.h"

// 初始化GPIO控制器
static int GpioController_Init(GpioController *controller, int gpio_num) 
{
    char path[64];
    int fd_export, ret;

    controller->gpio_num = gpio_num;
    controller->fd_value = -1;
    controller->fd_dir = -1;

    // 导出GPIO
    fd_export = open("/sys/class/gpio/export", O_WRONLY);
    if (fd_export < 0) {
        perror("Failed to open export");
        return -1;
    }

    ret = write(fd_export, &gpio_num, sizeof(gpio_num));
    if (ret < 0) {
        perror("Failed to export GPIO");
        close(fd_export);
        return -1;
    }
    close(fd_export);

    // 打开方向文件
    snprintf(path, sizeof(path), "%s%s%d%s", GPIO_SYSFS_PATH, "gpio", gpio_num, "/direction");
    controller->fd_dir = open(path, O_WRONLY);
    if (controller->fd_dir < 0) {
        perror("Failed to open direction");
        return -1;
    }

    // 打开值文件
    snprintf(path, sizeof(path), "%s%s%d%s", GPIO_SYSFS_PATH, "gpio", gpio_num, "/value");
    controller->fd_value = open(path, O_RDWR);
    if (controller->fd_value < 0) {
        perror("Failed to open value");
        close(controller->fd_dir);
        return -1;
    }

    return 0;
}

// 设置GPIO方向
static int GpioController_SetDirection(GpioController *controller, const char *direction) 
{
    if (controller->fd_dir < 0) {
        perror("Direction file not opened");
        return -1;
    }

    if (write(controller->fd_dir, direction, strlen(direction) + 1) < 0) {
        perror("Failed to set direction");
        return -1;
    }

    return 0;
}

// 设置GPIO值
static int GpioController_SetValue(GpioController *controller, int value) 
{
    char buf[2];
    if (controller->fd_value < 0) {
        perror("Value file not opened");
        return -1;
    }

    buf[0] = value ? '1' : '0';
    buf[1] = '\0';

    if (write(controller->fd_value, buf, sizeof(buf)) < 0) {
        perror("Failed to set value");
        return -1;
    }

    return 0;
}

// 获取GPIO值
static int GpioController_GetValue(GpioController *controller, int *value) 
{
    char buf[32];
    int ret;

    if (controller->fd_value < 0) {
        perror("Value file not opened");
        return -1;
    }

    ret = read(controller->fd_value, buf, sizeof(buf));
    if (ret < 0) {
        perror("Failed to read value");
        return -1;
    }

    *value = atoi(buf);
    lseek(controller->fd_value, 0, SEEK_SET); // 重置文件指针位置

    return 0;
}

// 清理GPIO控制器
static void GpioController_Cleanup(GpioController *controller) 
{
    char path[64];
    int fd_unexport;

    if (controller->fd_value >= 0) {
        close(controller->fd_value);
        controller->fd_value = -1;
    }

    if (controller->fd_dir >= 0) {
        close(controller->fd_dir);
        controller->fd_dir = -1;
    }

    // 取消导出GPIO
    fd_unexport = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd_unexport >= 0) {
        snprintf(path, sizeof(path), "%d", controller->gpio_num);
        int ret = write(fd_unexport, path, strlen(path) + 1);
        close(fd_unexport);
    }
}

int led_init(LedController *controller, int gpio_num) 
{
    // 初始化GPIO控制器
    if (GpioController_Init(&controller->gpio_controller, gpio_num) < 0) {
        return -1;
    }

    // 设置GPIO为输出方向
    if (GpioController_SetDirection(&controller->gpio_controller, "out") < 0) {
        GpioController_Cleanup(&controller->gpio_controller);
        return -1;
    }

    return 0;
}

// 打开LED
int led_set_high(LedController *controller) 
{
    return GpioController_SetValue(&controller->gpio_controller, 1);
}

// 关闭LED
int led_set_low(LedController *controller) 
{
    return GpioController_SetValue(&controller->gpio_controller, 0);
}

// 切换LED状态
int led_toggle(LedController *controller) 
{
    int state;
    if (GpioController_GetValue(&controller->gpio_controller, &state) < 0) {
        return -1;
    }
    return GpioController_SetValue(&controller->gpio_controller, !state);
}

// 获取LED状态
int led_get_state(LedController *controller, int *state) 
{
    return GpioController_GetValue(&controller->gpio_controller, state);
}

// 清理LED控制器
void led_destroy(LedController *controller) 
{
    GpioController_Cleanup(&controller->gpio_controller);
}
