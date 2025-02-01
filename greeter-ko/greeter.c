#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tao Jin");
MODULE_DESCRIPTION("A kernel module with a kernel thread that prints periodically");

static struct task_struct *my_thread = NULL;
static int thread_data = 10; // Example data to be used by the thread
static int print_delay_seconds = 1;

module_param(print_delay_seconds, int, 0644); // Allow changing delay via module parameter
MODULE_PARM_DESC(print_delay_seconds, "Delay in seconds between each print");

static int my_kernel_thread(void *data) {
    int *val = (int *)data;

    while (!kthread_should_stop()) {
        printk(KERN_INFO "Hello from kernel thread! Data: %d\n", *val);
        (*val)++; // Increment data

        // Use msleep() (for smaller delays) or ssleep() (for longer delays)
        ssleep(print_delay_seconds);  // Sleep for the specified number of seconds

        // If you need to do something that involves waiting on other events
        // or responding to specific conditions, you would use proper
        // synchronization mechanisms here instead of a simple sleep.
    }

    printk(KERN_INFO "Kernel thread stopping.\n");
    return 0;
}

static int __init my_module_init(void) {
    printk(KERN_INFO "Module loading...\n");

    // Create the kernel thread
    my_thread = kthread_run(my_kernel_thread, &thread_data, "my_kthread");

    if (IS_ERR(my_thread)) {
        printk(KERN_ERR "Failed to create kernel thread\n");
        return PTR_ERR(my_thread);
    }

    return 0;
}

static void __exit my_module_exit(void) {
    if (my_thread) {
        kthread_stop(my_thread);
        printk(KERN_INFO "Kernel thread stopped\n");
    }

    printk(KERN_INFO "Module unloaded.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);