#ifndef __SERIAL_H
#define __SERIAL_H

#include <termios.h>

typedef struct st_SerialPort SerialPort;

typedef struct st_SerialPortInfo{
	int speed;		     //baudrate
	int data_bits;		//data bits, 5, 6, 7, 8
	int	fctl;			//flow control, 0: none, 1: hardware, 2: software
	char parity;		//parity 0: none, 1: odd, 2: even
	int stop_bits;		//stop bits, 1, 2
	// char cName[32];	    //tty: 0, 1, 2, 3, 4, 5, 6, 7
} SerialPortInfo;

struct SerialOps {
	int (*open) (SerialPort *self, const char *path);
	int (*close) (SerialPort *self);
	ssize_t (*read) (SerialPort *self, void *buf, size_t count);
	ssize_t (*write) (SerialPort *self, const void *buf, size_t count);
	int (*configure) (SerialPort *self, const SerialPortInfo *info);
} ;

struct st_SerialPort {
	const struct SerialOps *ops;
	int fd;
	struct termios origin_tios;
	int is_open;
	pthread_mutex_t lock;
} ;

typedef struct st_UartPort {
	SerialPort base;
} UartPort;

typedef struct st_RS485Port {
	SerialPort base;
	int rts_pin;
} RS485Port;

UartPort *uart_port_create(void);
RS485Port *rs485_port_create(int rts_pin);

#endif
