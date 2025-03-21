#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include "serial.h"
#include "can.h"


const SerialPortInfo g_UartPortInfo = {
    .speed = 9600,
    .data_bits = 8, 
    .fctl = 0,
    .parity = 'N',
    .stop_bits = 1
};

const SerialPortInfo g_RS485PortInfo = {
    .speed = 9600,
    .data_bits = 8, 
    .fctl = 0,
    .parity = 'N',
    .stop_bits = 1
};

void can_rx_para(struct can_frame* frame)
{
    if (frame->can_dlc > 0) 
    {


    } 
}

int main()
{
#if 0
    UartPort *uart = create_uart_port();
    uart->base.ops->open((SerialPort*)uart, "/dev/ttyS0");
    uart->base.ops->configure((SerialPort*)uart, &g_UartPortInfo);

    RS485Port *rs485 = create_rs485_port(17);
    rs485->base.ops->open((SerialPort*)rs485, "/dev/ttyUSB0");
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
#if 1
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


    return 0;
}