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


// 定义每个灯的 GPIO 引脚编号
#define SYS_RUN_LED_PIN_NUM "104"  //GPIO3_B0 高亮
#define SYS_FAULT_LED_PIN_NUM "103"  //GPIO3_A7 高灭 3*32+(1-1)*8+7

// 定义每个灯的控制器实例
LedController run_led;
LedController fault_led;

// 初始化运行灯
#define RUN_LED_INIT() \
    do { \
        led_init(&run_led, SYS_RUN_LED_PIN_NUM); \
        led_set_low(&run_led); \
    } while (0)

// 初始化故障灯
#define FAULT_LED_INIT() \
    do { \
        led_init(&fault_led, SYS_FAULT_LED_PIN_NUM); \
        led_set_high(&fault_led); \
    } while (0)

// 打开运行灯
#define RUN_LED_ON() \
    do { \
        led_set_high(&run_led); \
    } while (0)

// 关闭运行灯
#define RUN_LED_OFF() \
    do { \
        led_set_low(&run_led); \
    } while (0)

// 切换运行灯状态
#define RUN_LED_TOGGLE() \
    do { \
        led_toggle(&run_led); \
    } while (0)

// 获取运行灯状态
#define RUN_LED_GET_STATE(state) \
    do { \
        led_get_state(&run_led, state); \
    } while (0)

// 打开故障灯
#define FAULT_LED_ON() \
    do { \
        led_set_low(&fault_led); \
    } while (0)

// 关闭故障灯
#define FAULT_LED_OFF() \
    do { \
        led_set_high(&fault_led); \
    } while (0)

// 获取故障灯状态
#define FAULT_LED_GET_STATE(state) \
    do { \
        led_get_state(&fault_led, state); \
    } while (0)

#endif
