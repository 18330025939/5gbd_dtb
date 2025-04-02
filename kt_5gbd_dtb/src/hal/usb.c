#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

static int usb_open(USBDevice *self) 
{
    if (self == NULL) {
        return LIBUSB_ERROR_INVALID_PARAM;
    }

    self->handle = libusb_open_device_with_vid_pid(NULL, self->vid, self->pid);
    if (!self->handle) {
        return LIBUSB_ERROR_NO_DEVICE;
    }

    int ret = libusb_claim_interface(self->handle, self->bulk_interface);
    if (ret != LIBUSB_SUCCESS) {
        libusb_close(self->handle);
        self->handle = NULL;
    }
    return ret;
}

static int usb_close(USBDevice *self) 
{
    if (!self || !self->handle) {
        return LIBUSB_ERROR_INVALID_PARAM;
    }

    libusb_release_interface(self->handle, self->bulk_interface);
    libusb_close(self->handle);
    self->handle = NULL;

    return LIBUSB_SUCCESS;
}

// static int usb_control_transfer(USBDevice *self, 
//                                uint8_t bmRequestType, uint8_t bRequest,
//                                uint16_t wValue, uint16_t wIndex,
//                                uint8_t *data, uint16_t wLength,
//                                unsigned int timeout) 
// {
//     return libusb_control_transfer(self->handle, bmRequestType, bRequest,
//                                   wValue, wIndex, data, wLength, timeout);
// }

static int usb_bulk_read(USBDevice *self, uint8_t *buffer, int length, uint32_t timeout) 
{
    int recv_len;

    if (buffer == NULL || length <= 0) {
        return -1;
    }

    int ret = libusb_bulk_transfer(self->handle, self->in_ep, buffer, length, &recv_len, timeout);
    if (ret < 0) {
        fprintf(stderr, "Bulk read failed: %s\n", libusb_error_name(ret));
        return ret;
    }

    return recv_len;
}

static int usb_bulk_write(USBDevice *self, uint8_t *buffer, int length, uint32_t timeout) 
{
    int transferred;
    uint32_t packages;
    uint8_t send_len;

    if (buffer == NULL || length <= 0) {
        return -1;
    }

    packages = length / 64;
    if (length & 64) {
        packages += 1;
    }

    for (uint32_t i = 0; i < packages; i++) {
        send_len = (length - (i * 64)) > 64 ? 64 : (length - (i * 64));
        int ret = libusb_bulk_transfer(self->handle, self->out_ep, buffer, length, &transferred, timeout);
        if (ret < 0) {
            fprintf(stderr, "Bulk write failed: %s\n", libusb_error_name(ret));
            return ret;
        }
        buffer += send_len;
    }
    return 0;
}

static void usb_register_callback(USBDevice *self, UsbDataCallback cb)
{
    self->data_cb = cb;
}

static USBDeviceOps bulk_ops = {
    .open = usb_open,
    .close = usb_close,
    .bulk_write = usb_bulk_write,
    .bulk_read = usb_bulk_read,
    .register_cb = usb_register_callback
};
} ;

USBDevice* usb_device_create(uint16_t vid, uint16_t pid) 
{
    USBDevice *dev = calloc(1, sizeof(USBDevice));
    if (!dev) {
        return NULL;
    }
    
    dev->vid = vid;
    dev->pid = pid;

    libusb_init(NULL);

    dev->handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
    if (!dev->handle) {
        fprintf(stderr, "Failed to open device\n");
        exit(EXIT_FAILURE);
    }

    libusb_device *usb_dev = libusb_get_device(dev->handle);
    struct libusb_device_descriptor desc;
    int ret = libusb_get_device_descriptor(usb_dev, &desc);
    if (ret < 0) {
        fprintf(stderr, "Failed to get device descriptor\n");
        exit(EXIT_FAILURE);
    }

    struct libusb_config_descriptor *config;
    ret = libusb_get_config_descriptor(usb_dev, 0, &config);
    if (ret < 0) {
        fprintf(stderr, "Failed to get config descriptor\n");
        exit(EXIT_FAILURE);
    }

    dev->bulk_interface = 0;
    for (int i = 0; i < config->interface[0].num_altsetting; i++) {
        const struct libusb_interface_descriptor *alt = &config->interface[0].altsetting[i];
        for (int j = 0; j < alt->bNumEndpoints; j++) {
            const struct libusb_endpoint_descriptor *ep = &alt->endpoint[j];
            if ((ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK) {
                if (ep->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
                    dev->in_ep = ep->bEndpointAddress;
                } else {
                    dev->out_ep = ep->bEndpointAddress;
                }
                dev->bulk_interface = alt->bInterfaceNumber;
            }
        }
    }

    libusb_free_config_descriptor(config);
    libusb_close(dev->handle);
    dev->ops = &bulk_ops;
    return dev;
}

void usb_device_destroy(USBDevice *dev) 
{
    if (dev) {
        if (dev->handle) {
            dev->ops->close(dev);
        }
        libusb_exit(NULL);
        free(dev);
    }
}
