#ifndef __USB_H
#define __USB_H

typedef struct st_USBDevice USBDevice;

typedef struct {
    int (*open)(USBDevice *self);
    int (*close)(USBDevice *self);
    // int (*control_transfer)(struct USBDevice *self, 
    //                        uint8_t bmRequestType, uint8_t bRequest,
    //                        uint16_t wValue, uint16_t wIndex,
    //                        uint8_t *data, uint16_t wLength, 
    //                        unsigned int timeout);
    int (*bulk_write)(USBDevice *self, uint8_t *buffer, int length, uint32_t timeout);
    int (*bulk_read)(USBDevice *self, uint8_t *buffer, int length, uint32_t timeout);
    // void (*print_info)(struct USBDevice *self);
} USBDeviceOps;

typedef void (*UsbDataCallback)(uint8_t*, size_t);

/*---------------------- USB 设备类定义 ----------------------*/
struct st_USBDevice {
    // 基础信息
    libusb_device_handle *handle;
    uint16_t vid;
    uint16_t pid;
    uint8_t in_ep;      // 输入端点地址
    uint8_t out_ep;     // 输出端点地址
    uint8_t bulk_interface;
    UsbDataCallback data_cb;
    USBDeviceOps *ops;
} ;


#endif