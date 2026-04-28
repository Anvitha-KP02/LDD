#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/kthread.h>
#include <linux/wait.h>                 // Required for the wait queues
#include <linux/err.h>

#define DEVICE_NAME "waitq_demo"

static DECLARE_WAIT_QUEUE_HEAD(my_wait_queue);  // processes waiting for specific condition
static int condition=0;
static int major;

// Read Function: Process will sleep here if condition is 0
static ssize_t dev_read(struct file *filp,char *buf,size_t len,loff_t *off)
{
pr_info("Read function called\n");
wait_event_interruptible(my_wait_queue,condition!=0);  // Put process to sleep until 'condition' becomes non-zero
pr_info("Read called: Woken up\n");
condition=0;   // Reset condition for next time, if we not reset here, next read() will never sleep
}

// Write Function: Wakes up the sleeping process
static ssize_t dev_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
    pr_info("Write called: Waking up reader...\n");
    condition = 1; // Set condition to true
    
    // 3. Wake up the process(es)
    wake_up_interruptible(&my_wait_queue);
    return len;
}

static struct file_operations fops = {
    .read = dev_read,
    .write = dev_write,
};

static int __init wq_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);
    pr_info("Waitqueue module loaded. Major: %d\n", major);
    return 0;
}

static void __exit wq_exit(void)
{
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("Waitqueue module unloaded.\n");
}

module_init(wq_init);
module_exit(wq_exit);
MODULE_LICENSE("GPL");



/* 
wait queue will not wake on their own.
they wake only when condition becomes true.
The wait queue doesn't know what you're waiting for, we must tell it using the condition variable*/

/*wait_event_interruptible(my_wait_queue,condition!=0); 
this line means if(condition!=0)  return immediately;
if false
current process state -> TASK_INTERRUPTIBLE
added to my_wait_queue
cpu switches to another process*/

/* TASK_INTERRUPTIBLE -> Most sleeping processes are in the TASK_INTERRUPTIBLE state, which means they are waiting for a resource but can be woken up immediately by a signal, allowing them to terminate gracefully if needed */

/* why read() sleeps?
Bcz, read means "I want data".
If no data/event is available: User process must wait and blocking read is expected behavior*/

/* what happens in write() -why wake here?
condition=1;
kernel scans my_wait_queue
All sleeping processes:  state changed to TASK_RUNNING, put back on wun queue.  They don't run immediately, scheduler decides when.*/
