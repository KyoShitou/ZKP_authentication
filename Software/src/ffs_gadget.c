#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/usb/functionfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <endian.h>
#include <errno.h>

#define USB_REQ_COMMIT "1"
#define USB_CHA_0 "2"
#define USB_CHA_1 "3"
#define USB_END "4"


// Binary USB descriptor definition
static struct {
    struct usb_functionfs_descs_head_v2 header;
    __le32 fs_count;
    struct usb_interface_descriptor intf;
    struct usb_endpoint_descriptor_no_audio ep_out;
    struct usb_endpoint_descriptor_no_audio ep_in;
} __attribute__((packed)) descriptors;

void setup_descriptors() {
    descriptors.header.magic = htole32(FUNCTIONFS_DESCRIPTORS_MAGIC_V2);
    descriptors.header.length = htole32(sizeof(descriptors));
    descriptors.header.flags = FUNCTIONFS_HAS_FS_DESC;

    descriptors.fs_count = htole32(3);

    descriptors.intf.bLength = sizeof(descriptors.intf);
    descriptors.intf.bDescriptorType = USB_DT_INTERFACE;
    descriptors.intf.bInterfaceNumber = 0;
    descriptors.intf.bAlternateSetting = 0;
    descriptors.intf.bNumEndpoints = 2;
    descriptors.intf.bInterfaceClass = USB_CLASS_VENDOR_SPEC;
    descriptors.intf.bInterfaceSubClass = 0;
    descriptors.intf.bInterfaceProtocol = 0;
    descriptors.intf.iInterface = 1;

    descriptors.ep_out.bLength = sizeof(descriptors.ep_out);
    descriptors.ep_out.bDescriptorType = USB_DT_ENDPOINT;
    descriptors.ep_out.bEndpointAddress = 0x01;
    descriptors.ep_out.bmAttributes = USB_ENDPOINT_XFER_BULK;
    descriptors.ep_out.wMaxPacketSize = htole16(64);
    descriptors.ep_out.bInterval = 0;

    descriptors.ep_in.bLength = sizeof(descriptors.ep_in);
    descriptors.ep_in.bDescriptorType = USB_DT_ENDPOINT;
    descriptors.ep_in.bEndpointAddress = 0x81;
    descriptors.ep_in.bmAttributes = USB_ENDPOINT_XFER_BULK;
    descriptors.ep_in.wMaxPacketSize = htole16(64);
    descriptors.ep_in.bInterval = 0;
}


int main() {
    setup_descriptors();

    int control_fd = open("/dev/ffs-myfunc", O_RDWR);
    if (control_fd < 0) {
        perror("open control");
        return 1;
    }

    // Write descriptors to control endpoint
    if (write(control_fd, &descriptors, sizeof(descriptors)) != sizeof(descriptors)) {
        perror("write descriptors");
        return 1;
    }

    close(control_fd);

    // Open endpoint files
    int ep_out = open("/dev/ffs-myfunc/ep1", O_RDONLY);
    if (ep_out < 0) {
        perror("open ep_out");
        return 1;
    }

    int ep_in = open("/dev/ffs-myfunc/ep2", O_WRONLY);
    if (ep_in < 0) {
        perror("open ep_in");
        return 1;
    }

    // Main loop
    char inbuf[64];
    char outbuf[] = "42";

    while (1) {
        ssize_t r = read(ep_out, inbuf, sizeof(inbuf));
        if (r < 0) {
            perror("read ep_out");
            break;
        }

        if (r == 1 && strcmp(inbuf, USB_REQ_COMMIT) == 0) {
            
        }
        if (r == 1 && inbuf[0] == '1') {
            if (write(ep_in, outbuf, strlen(outbuf)) < 0) {
                perror("write ep_in");
                break;
            }
        }
    }

    close(ep_out);
    close(ep_in);
    return 0;
}
