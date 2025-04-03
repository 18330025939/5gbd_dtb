#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include "serial.h"

int serial_open(SerialPort *self, const char *path)
{
    self->fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (-1 == self->fd) 
        return -1;

    if (tcgetattr(self->fd, &self->origin_tios) < 0) {
        close(self->fd);
        return -2;
    }

    self->is_open = 1;
    return 0;
}

int serial_close(SerialPort *self)
{
    if (self->is_open) {
        tcsetattr(self->fd, TCSANOW, &self->origin_tios);
        close(self->fd);
        self->is_open = 0;
    }

    return 0;
}

ssize_t serial_write(SerialPort *self, const void *buf, size_t count)
{
    return write(self->fd, buf, count);
}

ssize_t serial_read(SerialPort *self, void *buf, size_t count)
{

    return read(self->fd, buf, count);
}

static int conv_baud(unsigned long int baudrate)
{
	switch(baudrate){
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
			return B57600;
		case 115200:
			return B115200;
        case 1000000:
			return B1000000;
		default:
			return B9600;
	}
}

static int uart_configure(SerialPort *self, const SerialPortInfo *info)//int nSpeed, int nBits, char nEvent, nt nStop)
{
    struct termios tio;
    int baudrate = 0;

    if (info == NULL || self == NULL)
        return -1;

    // if (tcgetattr(self->fd, &tio) != 0)
    //     return -1;

    bzero(&tio, sizeof(tio));
    cfmakeraw(&tio);
    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_cflag &= ~CSIZE;
    tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tio.c_oflag &= ~OPOST;

    switch (info->fctl)
    {
        case 0:
            tio.c_cflag &= ~CRTSCTS;
        break;
        case 1:
            tio.c_cflag |= CRTSCTS;
        break;
        case 2:
            tio.c_cflag |= IXON | IXOFF |IXANY;
        break;
    }

    switch (info->data_bits)
    {
        case 5:
            tio.c_cflag |= CS5;
        break;
        case 6:
            tio.c_cflag |= CS6;
        break;
        case 7:
            tio.c_cflag |= CS7;
        break;
        case 8:
            tio.c_cflag |= CS8;
        break;
        default:
        return -1;
    }

    switch(info->parity)
    {
        case 'O': //Odd
            tio.c_cflag |= PARENB | PARODD;
            tio.c_iflag |= INPCK | ISTRIP; 
        break;
        case 'E': //Even
            tio.c_iflag |= INPCK | ISTRIP;
            tio.c_cflag |= PARENB;
            tio.c_cflag &= ~PARODD;
        break;
        case 'N': //None
            tio.c_cflag &= ~PARENB;
        break;
        default:
        return -1;
    }

    baudrate = conv_baud(info->speed);
    cfsetispeed(&tio, baudrate);
    cfsetospeed(&tio, baudrate);

    if (info->stop_bits == 1)
        tio.c_cflag &= ~CSTOPB;
    else if (info->stop_bits == 2)
        tio.c_cflag |= CSTOPB;
    else 
        return -1;

    tio.c_cc[VMIN] = 1;
    tio.c_cc[VTIME] = 0;

    tcflush(self->fd, TCOFLUSH);
    if (tcsetattr(self->fd , TCSANOW, &tio) != 0) {
        perror("com set error");
        return -1;
    }

    return 0;
}

static ssize_t rs485_write(SerialPort *self, const void *buf, size_t count)
{
    RS485Port *rs485 = (RS485Port*)self;

    // gpio_set(rs485->rts_pin, HIGH);
    ssize_t ret = write (rs485->base.fd, buf, count);
    // gpio_set(rs485->rts_pin, LOW);

    return ret;
}

static int rs485_configure(SerialPort *self, const SerialPortInfo *info)
{
    struct serial_rs485 rs485_conf = {0};
    RS485Port *rs485 = (RS485Port*)self;

    rs485_conf.flags |= SER_RS485_ENABLED;
    if (rs485->rts_pin > 0) {
        rs485_conf.flags |= SER_RS485_RTS_ON_SEND;
        rs485_conf.delay_rts_before_send = 1;
    }

    if (ioctl(rs485->base.fd, TIOCSRS485, &rs485_conf) < 0) {
        return -1;
    }

    return uart_configure(self, info);
}

static struct SerialOps uart_ops = {
    .open = serial_open,
    .close = serial_close,
    .read = serial_read,
    .write = serial_write,
    .configure = uart_configure
};


static struct SerialOps rs485_ops = {
    .open = serial_open,
    .close = serial_close,
    .read = serial_read,
    .write = rs485_write,
    .configure = rs485_configure
};

UartPort *uart_port_create(void)
{
    UartPort *port = NULL;
    
    port = malloc(sizeof(UartPort));
    if (port) { 
        memset(port, 0, sizeof(UartPort));
        port->base.ops = &uart_ops;
        port->base.is_open = 0;
        // uart_configure(&port->base, info);
    }
    return port;
}

RS485Port *rs485_port_create(int rts_pin)
{
    RS485Port *port = NULL;

    port = malloc(sizeof(RS485Port));
    if (port) {
        memset(port, 0, sizeof(UartPort));
        port->base.ops = &rs485_ops;
        port->base.is_open = 0;
        port->rts_pin = rts_pin;
        // rs485_configure(&port->base, info);
    }
    return port;
}

