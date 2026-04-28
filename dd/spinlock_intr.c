#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

static spinlock_t lock;
static unsigned long flags;

static int shared_counter=0;

static struct timer_list my_timer;
static struct task_struct *thread1;

void timer_callback(struct timer_list *t)
{
spin_lock_irqsave(&lock,flags);
shared_counter++;
printk("Interrupt : counter=%d\n",shared_counter);
spin_unlock_irqrestore(&lock,flags);
mod_timer(&my_timer,jiffies+msecs_to_jiffies(2000));
}

static int thread_fn(void *data)
{
while(!kthread_should_stop())
{
spin_lock_irqsave(&lock,flags);
printk("Thread: counter Read =%d\n",shared_counter);
spin_unlock_irqrestore(&lock,flags);
msleep(3000);
}
return 0;
}

static int __init irq_spinlock_init(void)
{
printk("IRQ spinlock loaded\n");
spin_lock_init(&lock);
timer_setup(&my_timer,timer_callback,0);
mod_timer(&my_timer,jiffies+msecs_to_jiffies(2000));

thread1=kthread_run(thread_fn,NULL,"counter_thread");
return 0;
}

static void __exit irq_spinlock_exit(void)
{
del_timer(&my_timer);
kthread_stop(thread1);
printk("IRQ spinlock unloaded\n");
}

module_init(irq_spinlock_init);
module_exit(irq_spinlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ANVITHA");
MODULE_DESCRIPTION("Spinlock+irq program");
