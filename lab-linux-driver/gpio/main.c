#include <asm/io.h>
#include <asm/irq.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>

#pragma region Configurations
MODULE_LICENSE("MIT");
MODULE_AUTHOR("UnnamedOrange");
MODULE_DESCRIPTION("GPIO driver for Raspberry Pi 3");

#define DEV_MAJOR 114   // Major number of device.
#define DEV_MINOR 514   // Minor number of device.
#define DEV_NAME "gpio" // Name of device.

#define GPIO_BASE ((void*)0x3F200000) // Base address of GPIO.
#pragma endregion

#pragma region Global Variables
static ssize_t gpio_write(struct file*, const char __user*, size_t, loff_t*);

static dev_t device_id;
static struct file_operations gpio_fops = {
    .owner = THIS_MODULE,
    .write = gpio_write,
};
static struct cdev* gpio_cdev; // Character device.

typedef struct
{
    uint32_t GPFSEL[6]; // GPIO Function Select.
    uint32_t _rev1;     // Reserved.
    uint32_t GPSET[2];  // GPIO Pin Output Set.
    uint32_t _rev2;     // Reserved.
    uint32_t GPCLR[2];  // GPIO Pin Output Clear.
    // The remaining registers are not used.
} gpio_registers_t;
static gpio_registers_t* remapped_gpio_base; // Remapped base address of GPIO.
#pragma endregion

#pragma region GPIO
static void ke_gpio_select_function(int pin, int function)
{
    // assert(0 <= pin && pin < 54);
    // assert(0 <= function && function <= 7);
    int register_index = pin / 10; // 10 pins per register.
    int bit = (pin % 10) * 3;      // 3 bits per pin. Use 1 << bit.

    uint32_t old_value = remapped_gpio_base->GPFSEL[register_index];
    uint32_t mask = ~(0b111 << bit);
    uint32_t new_value =
        (old_value & mask) | (function << bit); // Function is valid.

    // Write the new value.
    remapped_gpio_base->GPFSEL[register_index] = new_value;
    printk(KERN_INFO "GPIO: Select function for %d (register %x -> %x)\n", pin,
           old_value, new_value);
}
static void ke_gpio_set_output(int pin, bool is_set)
{
    // assert(0 <= pin && pin < 54);
    int register_index = pin / 32; // 32 pins per register.
    int bit = pin % 32;            // Use 1 << bit.

    // Write the output.
    if (is_set)
        remapped_gpio_base->GPSET[register_index] = 1ul << bit;
    else
        remapped_gpio_base->GPCLR[register_index] = 1ul << bit;
    printk(KERN_INFO "GPIO: Set output for %d (value %d)\n", pin, is_set);
}
#pragma endregion

#pragma region File Operations
static ssize_t gpio_write(struct file* file, const char __user* buf,
                          size_t count, loff_t* ppos)
{
    // Copy the data from user space.
    unsigned char* kbuf = kmalloc(count, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;
    if (0 != copy_from_user(kbuf, buf, count))
    {
        kfree(kbuf);
        return -EFAULT;
    }

    // Parse the command.
    int pin = 0;
    int value = 0;
    if (2 != sscanf(kbuf, "%d=%d", &pin, &value))
    {
        kfree(kbuf);
        return -EINVAL;
    }
    if (pin < 0 || pin >= 54)
    {
        kfree(kbuf);
        return -EINVAL;
    }
    if (value != 0 && value != 1)
    {
        kfree(kbuf);
        return -EINVAL;
    }
    ke_gpio_select_function(pin, 1);
    ke_gpio_set_output(pin, value);

    // Free the buffer.
    kfree(kbuf);

    printk(KERN_INFO "GPIO: Written.\n");
    return count;
}
#pragma endregion

#pragma region Driver Entry
/**
 * @brief In entry, register the character device.
 */
static int __init gpio_module_entry(void)
{
    int last_error = 0;

    device_id = MKDEV(DEV_MAJOR, DEV_MINOR);
    printk(KERN_INFO "GPIO: Device ID %d, %d.\n", DEV_MAJOR, DEV_MINOR);

    // Step 1: Allocate a device number.
    last_error = register_chrdev_region(device_id, 1, DEV_NAME);
    if (last_error < 0)
    {
        printk(KERN_ERR "GPIO: Failed to register device %d, %d.\n", DEV_MAJOR,
               DEV_MINOR);
        goto fail_register_chrdev_region;
    }

    // Step 2: Create a character device and initialize it.
    gpio_cdev = cdev_alloc();
    gpio_cdev->ops = &gpio_fops;

    // Step 3: Add the device to the kernel.
    last_error = cdev_add(gpio_cdev, device_id, 1);
    if (last_error < 0)
    {
        printk(KERN_ERR "GPIO: Failed to add device %d, %d.\n", DEV_MAJOR,
               DEV_MINOR);
        goto fail_cdev_add;
    }

    // Another step: ioremap.
    remapped_gpio_base =
        (gpio_registers_t*)ioremap(GPIO_BASE, sizeof(gpio_registers_t));

    printk(KERN_INFO "GPIO: Loaded.\n");
    goto success;

fail_cdev_add:
    unregister_chrdev_region(device_id, 1);
fail_register_chrdev_region:
    return last_error;
success:
    return 0;
}

static void __exit gpio_module_exit(void)
{
    iounmap(remapped_gpio_base);
    cdev_del(gpio_cdev);
    unregister_chrdev_region(device_id, 1);
}

module_init(gpio_module_entry);
module_exit(gpio_module_exit);
#pragma endregion
