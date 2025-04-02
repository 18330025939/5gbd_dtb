#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <event2/event.h>
#include "serial.h"
#include "can.h"
#include "cloud_comm.h"


void signal_handler(evutil_socket_t fd, short events, void *arg)
{
    struct event_base *base = (struct event_base *)arg;
    printf("收到退出信号，准备退出...\n");
    event_base_loopexit(base, NULL);
}

int main()
{
#if 0
    UartPort *uart = create_uart_port();
    uart->base.ops->open((SerialPort*)uart, "/dev/ttyS0");
    const SerialPortInfo g_UartPortInfo = {
        .speed = 9600,
        .data_bits = 8, 
        .fctl = 0,
        .parity = 'N',
        .stop_bits = 1
    };
    uart->base.ops->configure((SerialPort*)uart, &g_UartPortInfo);

    RS485Port *rs485 = create_rs485_port(17);
    rs485->base.ops->open((SerialPort*)rs485, "/dev/ttyUSB0");
    const SerialPortInfo g_RS485PortInfo = {
        .speed = 9600,
        .data_bits = 8, 
        .fctl = 0,
        .parity = 'N',
        .stop_bits = 1
    };
    rs485->base.ops->configure((SerialPort*)rs485, &g_RS485PortInfo);


    char buf[128];
    uart->base.ops->write((SerialPort*)uart, "AT\r\n", 4);
    rs485->base.ops->read((SerialPort*)rs485, buf, sizeof(buf));
    
    // 资源清理
    uart->base.ops->close((SerialPort*)uart);
    rs485->base.ops->close((SerialPort*)rs485);
    free(uart);
    free(rs485);
#endif
#if 0
    CANDevice *can_dev = can_device_create();
    struct can_filter rfilter = {
        .can_id = 0x12,
        .can_mask = CAN_SFF_MASK
    };
    can_dev->ops->set_filter(can_dev, &rfilter);
    can_dev->ops->open(can_dev, "can0");
    can_dev->ops->register_callback(can_dev, can_rx_para);
    can_dev->ops->close(can_dev);
#endif

    CloundCommContext *cloud_ctx = NULL;

    cloud_ctx = (CloundCommContext*)malloc(sizeof(CloundCommContext));
    if (cloud_ctx == NULL) {
        exit(1);
    }
    memset(cloud_ctx, 0, sizeof(CloundCommContext));
    clound_comm_init(cloud_ctx);

    struct event_base *base = event_base_new();
    if (!base) {
        perror("event_base_new");
        exit(1);
    }
    struct event *signal = event_new(base, SIGINT, 0, signal_handler, base);
    if (!signal) {
        perror("event_new");
        event_base_free(base);
        exit(1);
    }
    event_add(signal, NULL);

    event_base_dispatch(base);

    event_free(signal);
    event_base_free(base);
    clound_comm_init(cloud_ctx);

    return 0;
}