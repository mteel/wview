/*---------------------------------------------------------------------------
 
  FILENAME:
        usbcompat.h
 
  PURPOSE:
        Provide a quick and dirty compatibility layer for libusb-0.1 to
        libusb-1.0 interface API and utilities.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        02/13/2011      M.S. Teel       0               Original
 
  NOTES:
        Based on source code for the libusb-compat-0.1 library. See the
        copyright for that library below this header.
 
  LICENSE:
        Copyright (c) 2011, Mark S. Teel (mteel2005@gmail.com)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/
/*
 * Copyright (c) 2000-2003 Johannes Erdfelt <johannes@erdfelt.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * This file (and only this file) may alternatively be licensed under the
 * BSD license. See the LICENSE file shipped with the libusb-compat-0.1 source
 * distribution for details.
 */

#ifndef INC_usbcompath
#define INC_usbcompath

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include <dirent.h>
#include <libusb-1.0/libusb.h>


/*
 * USB spec information
 *
 * This is all stuff grabbed from various USB specs and is pretty much
 * not subject to change
 */

/*
 * Device and/or Interface Class codes
 */
#define USBCOMPAT_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USBCOMPAT_CLASS_AUDIO			1
#define USBCOMPAT_CLASS_COMM			2
#define USBCOMPAT_CLASS_HID			3
#define USBCOMPAT_CLASS_PRINTER		7
#define USBCOMPAT_CLASS_PTP			6
#define USBCOMPAT_CLASS_MASS_STORAGE		8
#define USBCOMPAT_CLASS_HUB			9
#define USBCOMPAT_CLASS_DATA			10
#define USBCOMPAT_CLASS_VENDOR_SPEC		0xff

/*
 * Descriptor types
 */
#define USBCOMPAT_DT_DEVICE			0x01
#define USBCOMPAT_DT_CONFIG			0x02
#define USBCOMPAT_DT_STRING			0x03
#define USBCOMPAT_DT_INTERFACE		0x04
#define USBCOMPAT_DT_ENDPOINT			0x05

#define USBCOMPAT_DT_HID			0x21
#define USBCOMPAT_DT_REPORT			0x22
#define USBCOMPAT_DT_PHYSICAL			0x23
#define USBCOMPAT_DT_HUB			0x29

/*
 * Descriptor sizes per descriptor type
 */
#define USBCOMPAT_DT_DEVICE_SIZE		18
#define USBCOMPAT_DT_CONFIG_SIZE		9
#define USBCOMPAT_DT_INTERFACE_SIZE		9
#define USBCOMPAT_DT_ENDPOINT_SIZE		7
#define USBCOMPAT_DT_ENDPOINT_AUDIO_SIZE	9	/* Audio extension */
#define USBCOMPAT_DT_HUB_NONVAR_SIZE		7

/* All standard descriptors have these 2 fields in common */
struct usbcompat_descriptor_header {
	u_int8_t  bLength;
	u_int8_t  bDescriptorType;
};

/* String descriptor */
struct usbcompat_string_descriptor {
	u_int8_t  bLength;
	u_int8_t  bDescriptorType;
	u_int16_t wData[1];
};

/* HID descriptor */
struct usbcompat_hid_descriptor {
	u_int8_t  bLength;
	u_int8_t  bDescriptorType;
	u_int16_t bcdHID;
	u_int8_t  bCountryCode;
	u_int8_t  bNumDescriptors;
	/* u_int8_t  bReportDescriptorType; */
	/* u_int16_t wDescriptorLength; */
	/* ... */
};

/* Endpoint descriptor */
#define USBCOMPAT_MAXENDPOINTS	32
struct usbcompat_endpoint_descriptor {
	u_int8_t  bLength;
	u_int8_t  bDescriptorType;
	u_int8_t  bEndpointAddress;
	u_int8_t  bmAttributes;
	u_int16_t wMaxPacketSize;
	u_int8_t  bInterval;
	u_int8_t  bRefresh;
	u_int8_t  bSynchAddress;

	unsigned char *extra;	/* Extra descriptors */
	int extralen;
};

#define USBCOMPAT_ENDPOINT_ADDRESS_MASK	0x0f    /* in bEndpointAddress */
#define USBCOMPAT_ENDPOINT_DIR_MASK		0x80

#define USBCOMPAT_ENDPOINT_TYPE_MASK		0x03    /* in bmAttributes */
#define USBCOMPAT_ENDPOINT_TYPE_CONTROL	0
#define USBCOMPAT_ENDPOINT_TYPE_ISOCHRONOUS	1
#define USBCOMPAT_ENDPOINT_TYPE_BULK		2
#define USBCOMPAT_ENDPOINT_TYPE_INTERRUPT	3

/* Interface descriptor */
#define USBCOMPAT_MAXINTERFACES	32
struct usbcompat_interface_descriptor {
	u_int8_t  bLength;
	u_int8_t  bDescriptorType;
	u_int8_t  bInterfaceNumber;
	u_int8_t  bAlternateSetting;
	u_int8_t  bNumEndpoints;
	u_int8_t  bInterfaceClass;
	u_int8_t  bInterfaceSubClass;
	u_int8_t  bInterfaceProtocol;
	u_int8_t  iInterface;

	struct usbcompat_endpoint_descriptor *endpoint;

	unsigned char *extra;	/* Extra descriptors */
	int extralen;
};

#define USBCOMPAT_MAXALTSETTING	128	/* Hard limit */
struct usbcompat_interface {
	struct usbcompat_interface_descriptor *altsetting;

	int num_altsetting;
};

/* Configuration descriptor information.. */
#define USBCOMPAT_MAXCONFIG		8
struct usbcompat_config_descriptor {
	u_int8_t  bLength;
	u_int8_t  bDescriptorType;
	u_int16_t wTotalLength;
	u_int8_t  bNumInterfaces;
	u_int8_t  bConfigurationValue;
	u_int8_t  iConfiguration;
	u_int8_t  bmAttributes;
	u_int8_t  MaxPower;

	struct usbcompat_interface *interface;

	unsigned char *extra;	/* Extra descriptors */
	int extralen;
};

/* Device descriptor */
struct usbcompat_device_descriptor {
	u_int8_t  bLength;
	u_int8_t  bDescriptorType;
	u_int16_t bcdUSB;
	u_int8_t  bDeviceClass;
	u_int8_t  bDeviceSubClass;
	u_int8_t  bDeviceProtocol;
	u_int8_t  bMaxPacketSize0;
	u_int16_t idVendor;
	u_int16_t idProduct;
	u_int16_t bcdDevice;
	u_int8_t  iManufacturer;
	u_int8_t  iProduct;
	u_int8_t  iSerialNumber;
	u_int8_t  bNumConfigurations;
};

struct usbcompat_ctrl_setup {
	u_int8_t  bRequestType;
	u_int8_t  bRequest;
	u_int16_t wValue;
	u_int16_t wIndex;
	u_int16_t wLength;
};

/*
 * Standard requests
 */
#define USBCOMPAT_REQ_GET_STATUS		0x00
#define USBCOMPAT_REQ_CLEAR_FEATURE		0x01
/* 0x02 is reserved */
#define USBCOMPAT_REQ_SET_FEATURE		0x03
/* 0x04 is reserved */
#define USBCOMPAT_REQ_SET_ADDRESS		0x05
#define USBCOMPAT_REQ_GET_DESCRIPTOR		0x06
#define USBCOMPAT_REQ_SET_DESCRIPTOR		0x07
#define USBCOMPAT_REQ_GET_CONFIGURATION	0x08
#define USBCOMPAT_REQ_SET_CONFIGURATION	0x09
#define USBCOMPAT_REQ_GET_INTERFACE		0x0A
#define USBCOMPAT_REQ_SET_INTERFACE		0x0B
#define USBCOMPAT_REQ_SYNCH_FRAME		0x0C

#define USBCOMPAT_TYPE_STANDARD		(0x00 << 5)
#define USBCOMPAT_TYPE_CLASS			(0x01 << 5)
#define USBCOMPAT_TYPE_VENDOR			(0x02 << 5)
#define USBCOMPAT_TYPE_RESERVED		(0x03 << 5)

#define USBCOMPAT_RECIP_DEVICE		0x00
#define USBCOMPAT_RECIP_INTERFACE		0x01
#define USBCOMPAT_RECIP_ENDPOINT		0x02
#define USBCOMPAT_RECIP_OTHER			0x03

/*
 * Various libusb API related stuff
 */

#define USBCOMPAT_ENDPOINT_IN			0x80
#define USBCOMPAT_ENDPOINT_OUT		0x00

/* Error codes */
#define USBCOMPAT_ERROR_BEGIN			500000

/* Data types */
struct usbcompat_device;
struct usbcompat_bus;

/*
 * To maintain compatibility with applications already built with libusb,
 * we must only add entries to the end of this structure. NEVER delete or
 * move members and only change types if you really know what you're doing.
 */
struct usbcompat_device {
  struct usbcompat_device *next, *prev;

  char filename[PATH_MAX + 1];

  struct usbcompat_bus *bus;

  struct usbcompat_device_descriptor descriptor;
  struct usbcompat_config_descriptor *config;

  void *dev;		/* Darwin support */

  u_int8_t devnum;

  unsigned char num_children;
  struct usbcompat_device **children;
};

struct usbcompat_bus {
  struct usbcompat_bus *next, *prev;

  char dirname[PATH_MAX + 1];

  struct usbcompat_device *devices;
  u_int32_t location;

  struct usbcompat_device *root_dev;
};

struct usbcompat_dev_handle;
typedef struct usbcompat_dev_handle usbcompat_dev_handle;

/* Variables */
extern struct usbcompat_bus *usbcompat_busses;

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */

/* usb.c */
usbcompat_dev_handle *usbcompat_open(struct usbcompat_device *dev);
int usbcompat_close(usbcompat_dev_handle *dev);
int usbcompat_get_string(usbcompat_dev_handle *dev, int index, int langid, char *buf,
	size_t buflen);
int usbcompat_get_string_simple(usbcompat_dev_handle *dev, int index, char *buf,
	size_t buflen);

/* descriptors.c */
int usbcompat_get_descriptor_by_endpoint(usbcompat_dev_handle *udev, int ep,
	unsigned char type, unsigned char index, void *buf, int size);
int usbcompat_get_descriptor(usbcompat_dev_handle *udev, unsigned char type,
	unsigned char index, void *buf, int size);

/* <arch>.c */
int usbcompat_bulk_write(usbcompat_dev_handle *dev, int ep, char *bytes, int size,
	int timeout);
int usbcompat_bulk_read(usbcompat_dev_handle *dev, int ep, char *bytes, int size,
	int timeout);
int usbcompat_interrupt_write(usbcompat_dev_handle *dev, int ep, char *bytes, int size,
        int timeout);
int usbcompat_interrupt_read(usbcompat_dev_handle *dev, int ep, char *bytes, int size,
        int timeout);
int usbcompat_control_msg(usbcompat_dev_handle *dev, int requesttype, int request,
	int value, int index, char *bytes, int size, int timeout);
int usbcompat_set_configuration(usbcompat_dev_handle *dev, int configuration);
int usbcompat_claim_interface(usbcompat_dev_handle *dev, int interface);
int usbcompat_release_interface(usbcompat_dev_handle *dev, int interface);
int usbcompat_set_altinterface(usbcompat_dev_handle *dev, int alternate);
int usbcompat_resetep(usbcompat_dev_handle *dev, unsigned int ep);
int usbcompat_clear_halt(usbcompat_dev_handle *dev, unsigned int ep);
int usbcompat_reset(usbcompat_dev_handle *dev);

#define LIBUSBCOMPAT_HAS_GET_DRIVER_NP 1
int usbcompat_get_driver_np(usbcompat_dev_handle *dev, int interface, char *name,
	unsigned int namelen);
#define LIBUSBCOMPAT_HAS_DETACH_KERNEL_DRIVER_NP 1
int usbcompat_detach_kernel_driver_np(usbcompat_dev_handle *dev, int interface);

char *usbcompat_strerror(void);

void usbcompat_init(void);
void usbcompat_set_debug(int level);
int usbcompat_find_busses(void);
int usbcompat_find_devices(void);
struct usbcompat_device *usbcompat_device(usbcompat_dev_handle *dev);
struct usbcompat_bus *usbcompat_get_busses(void);

/* *** BEGIN added from libusb-compat-0.1/usbi.h *** */
/* Some quick and generic macros for the simple kind of lists we use */
#define LIST_ADD(begin, ent) \
	do { \
	  if (begin) { \
	    ent->next = begin; \
	    ent->next->prev = ent; \
	  } else \
	    ent->next = NULL; \
	  ent->prev = NULL; \
	  begin = ent; \
	} while(0)

#define LIST_DEL(begin, ent) \
	do { \
	  if (ent->prev) \
	    ent->prev->next = ent->next; \
	  else \
	    begin = ent->next; \
	  if (ent->next) \
	    ent->next->prev = ent->prev; \
	  ent->prev = NULL; \
	  ent->next = NULL; \
	} while (0)

struct usbcompat_dev_handle {
	libusb_device_handle *handle;
	struct usbcompat_device *device;

	/* libusb-0.1 is buggy w.r.t. interface claiming. it allows you to claim
	 * multiple interfaces but only tracks the most recently claimed one,
	 * which is used for usbcompat_set_altinterface(). we clone the buggy behaviour
	 * here. */
	int last_claimed_interface;
};
/* *** END added from libusb-compat-0.1/usbi.h *** */

#ifdef __cplusplus
}
#endif

#endif /* __USBCOMPAT_H__ */

