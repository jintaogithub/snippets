#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tao Jin");
MODULE_DESCRIPTION("A simple Hello World kernel module");

static int __init hello_init(void) {
    printk(KERN_INFO "Hello, world! from the kernel module (cross-compiled)\n");
    return 0;
}

static void __exit hello_exit(void) {
    printk(KERN_INFO "Goodbye, world! from the kernel module (cross-compiled)\n");
}

module_init(hello_init);
module_exit(hello_exit);