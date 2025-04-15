#ifndef __LED_H
#define __LED_H

#define GPIO_SYSFS_PATH "/sys/class/gpio/"

typedef struct {
    char *gpio_num;         // GPIO编号
    int fd_value;         // 值文件描述符
    int fd_dir;           // 方向文件描述符
} GpioController;

typedef struct {
    GpioController gpio_controller; // GPIO控制器
} LedController;

int led_init(LedController *controller, char *gpio_num);
int led_set_high(LedController *controller);
int led_set_low(LedController *controller);
int led_toggle(LedController *controller);
int led_get_state(LedController *controller, int *state);
void led_destroy(LedController *controller);
#endif
