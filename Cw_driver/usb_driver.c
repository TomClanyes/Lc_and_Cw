#include <linux/kernel.h> 
#include <linux/slab.h> 
#include <linux/module.h> 
#include <linux/init.h> 
#include <linux/usb/input.h> 
#include <linux/hid.h> 
#include <linux/fs.h> 
#include <asm/segment.h> 
#include <asm/uaccess.h> 
#include <linux/buffer_head.h> 
#include <linux/device.h> 
#include <linux/cdev.h> 
  
#define DRIVER_VERSION "v0.1"  
#define DRIVER_AUTHOR "<TomClanyes>"  
#define DRIVER_DESC "USB driver"  
#define DRIVER_LICENSE "GPL"  
  
MODULE_AUTHOR(DRIVER_AUTHOR);  
MODULE_DESCRIPTION(DRIVER_DESC);  
MODULE_LICENSE(DRIVER_LICENSE);  
  
static char current_data = 0;  
static int registered = 0;  
  
struct usb_mouse {  
 char name[128];  
 char phys[64];  
 struct usb_device *usbdev;  
 struct input_dev *input;  
 struct urb *irq;  
 signed char *data;  
 dma_addr_t data_dma;  
};  
  
static void usb_mouse_irq(struct urb *urb)  
{  
 struct usb_mouse *mouse = urb->context;  
 signed char *data = mouse->data;  
 struct input_dev *input = mouse->input;  
 int status;  
  
 switch (urb->status) {  
 case 0:  
  break;  
 case -ECONNRESET:  
 case -ENOENT:  
 case -ESHUTDOWN:  
  return;  
 default:  
  goto resubmit;  
 }  
  
 input_report_key(input, BTN_LEFT, data[0] & 0x01);  
 input_report_key(input, BTN_RIGHT, data[0] & 0x02);  
 input_report_key(input, BTN_MIDDLE, data[0] & 0x04);  
 input_report_rel(input, REL_X, data[1]);  
 input_report_rel(input, REL_Y, data[2]);  
 input_report_rel(input, REL_WHEEL, data[3]);  
 input_sync(input);  
  
resubmit:  
 status = usb_submit_urb(urb, GFP_ATOMIC);  
 if (status)  
  dev_err(&mouse->usbdev->dev, "urb resubmit failed: %d\n", status);  
  
 current_data = data[0];  
  
 if (!(data[0] & 0x01) && !(data[0] & 0x02)) {  
  pr_info("No button pressed!\n");  
  return;  
 }  
  
 if (data[0] & 0x01)  
  pr_info("Left mouse button clicked!\n");  
 else if (data[0] & 0x02)  
  pr_info("Right mouse button clicked!\n");  
}  
  
static int usb_mouse_open(struct input_dev *dev)  
{  
 struct usb_mouse *mouse = input_get_drvdata(dev);  
  
 mouse->irq->dev = mouse->usbdev;  
 return usb_submit_urb(mouse->irq, GFP_KERNEL);  
}  
  
static void usb_mouse_close(struct input_dev *dev)  
{  
 struct usb_mouse *mouse = input_get_drvdata(dev);  
  
 usb_kill_urb(mouse->irq);  
}  
  
static int my_open(struct inode *i, struct file *f)  
{  
 pr_info("Driver: open()\n");  
 return 0;  
}  
  
static int my_close(struct inode *i, struct file *f)  
{  
 pr_info("Driver: close()\n");  
 return 0;  
}  
  
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)  
{  
 pr_info("Driver: read()\n");  
 return copy_to_user(buf, &current_data, 1) ? -EFAULT : 1;  
}  
  
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)  
{  
 pr_info("Driver: write()\n");  
 return len;  
}  
  
static struct file_operations fops = {  
 .owner = THIS_MODULE,  
 .open = my_open,  
 .release = my_close,  
 .read = my_read,  
 .write = my_write,  
};  
  
static int usb_mouse_probe(struct usb_interface *intf, const struct usb_device_id *id)  
{  
 struct usb_device *dev = interface_to_usbdev(intf);  
 struct usb_host_interface *interface;  
 struct usb_endpoint_descriptor *endpoint;  
 struct usb_mouse *mouse;  
 struct input_dev *input;  
 int pipe, maxp;  
 int error = -ENOMEM;  
 int t; 
  
 interface = intf->cur_altsetting;  
 if (interface->desc.bNumEndpoints != 1)  
  return -ENODEV;  
  
 endpoint = &interface->endpoint[0].desc;  
 if (!usb_endpoint_is_int_in(endpoint))  
  return -ENODEV;  
  
 pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);  
 maxp = usb_maxpacket(dev, pipe);  // Исправлено: удален третий аргумент 
  
 mouse = kzalloc(sizeof(*mouse), GFP_KERNEL);  
 input = input_allocate_device();  
 if (!mouse || !input)  
  goto fail1;  
  
 mouse->data = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &mouse->data_dma);  
 if (!mouse->data)  
  goto fail1;  
  
 mouse->irq = usb_alloc_urb(0, GFP_KERNEL);  
 if (!mouse->irq)  
  goto fail2;  
  
 mouse->usbdev = dev;  
 mouse->input = input;  
  
 if

Osvaldo Mobrey, [19.12.2024 4:19]
(dev->manufacturer)  
  strncpy(mouse->name, dev->manufacturer, sizeof(mouse->name));  
  
 if (dev->product) {  
  if (dev->manufacturer)  
   strlcat(mouse->name, " ", sizeof(mouse->name));  
  strlcat(mouse->name, dev->product, sizeof(mouse->name));  
 }  
  
 if (!strlen(mouse->name)) 
snprintf(mouse->name, sizeof(mouse->name), "USB HIDBP Mouse %04x:%04x",  
    le16_to_cpu(dev->descriptor.idVendor),  
    le16_to_cpu(dev->descriptor.idProduct));  
  
 usb_make_path(dev, mouse->phys, sizeof(mouse->phys));  
 strlcat(mouse->phys, "/input0", sizeof(mouse->phys));  
  
 input->name = mouse->name;  
 input->phys = mouse->phys;  
 usb_to_input_id(dev, &input->id);  
 input->dev.parent = &intf->dev;  
  
 input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);  
 input->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) | BIT_MASK(BTN_RIGHT) | BIT_MASK(BTN_MIDDLE);  
 input->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);  
 input->keybit[BIT_WORD(BTN_MOUSE)] |= BIT_MASK(BTN_SIDE) | BIT_MASK(BTN_EXTRA);  
 input->relbit[0] |= BIT_MASK(REL_WHEEL);  
  
 input_set_drvdata(input, mouse);  
 input->open = usb_mouse_open;  
 input->close = usb_mouse_close;  
  
 usb_fill_int_urb(mouse->irq, dev, pipe, mouse->data, (maxp > 8 ? 8 : maxp),  
    usb_mouse_irq, mouse, endpoint->bInterval);  
 mouse->irq->transfer_dma = mouse->data_dma;  
 mouse->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;  
  
 error = input_register_device(mouse->input);  
 if (error)  
  goto fail3;  
  
 usb_set_intfdata(intf, mouse);  
  
 t = register_chrdev(91, "mymouse", &fops);  
 if (t < 0) {  
  pr_info("mymouse registration failed\n");  
  registered = 0;  
 } else {  
  pr_info("mymouse registration successful\n");  
  registered = 1;  
 }  
 return t;  
  
fail3:  
 usb_free_urb(mouse->irq);  
fail2:  
 usb_free_coherent(dev, 8, mouse->data, mouse->data_dma);  
fail1:  
 input_free_device(input);  
 kfree(mouse);  
 return error;  
}  
  
static void usb_mouse_disconnect(struct usb_interface *intf)  
{  
 struct usb_mouse *mouse = usb_get_intfdata(intf);  
  
 usb_set_intfdata(intf, NULL);  
 if (mouse) {  
  usb_kill_urb(mouse->irq);  
  input_unregister_device(mouse->input);  
  usb_free_urb(mouse->irq);  
  usb_free_coherent(interface_to_usbdev(intf), 8, mouse->data, mouse->data_dma);  
  kfree(mouse);  
 }  
  
 if (registered)  
  unregister_chrdev(91, "mymouse");  
 registered = 0;  
}  
  
static struct usb_device_id usb_mouse_id_table[] = {  
 { USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,  
  USB_INTERFACE_PROTOCOL_MOUSE) },  
 {}  
};  
MODULE_DEVICE_TABLE(usb, usb_mouse_id_table);  
static struct usb_driver usb_mouse_driver = {  
 .name = "usbmouse",  
 .probe = usb_mouse_probe,  
 .disconnect = usb_mouse_disconnect,  
 .id_table = usb_mouse_id_table,  
};  
module_usb_driver(usb_mouse_driver);
