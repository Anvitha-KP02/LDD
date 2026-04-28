#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/kthread.h>
#include<linux/delay.h>
#include<linux/spinlock.h>

static spinlock_t lock;
static int shared_counter=0;

static struct task_struct *thread1;
static struct task_struct *thread2;

static int thread_fn(void *data)
{
int i;
for(i=0;i<5;i++)
{
spin_lock(&lock);
shared_counter++;
printk("Thread %s: counter=%d\n",current->comm, shared_counter);
spin_unlock(&lock);
msleep(1000);
}
return 0;
}

static int __init spinlock_init(void)
{
printk("Spinlock module loaded\n");
spin_lock_init(&lock);
thread1=kthread_run(thread_fn,NULL,"thread1");
thread2=kthread_run(thread_fn,NULL,"thread2");

return 0;
}

static void __exit spinlock_exit(void)
{
printk("Spinlock module unloaded\n");
}

module_init(spinlock_init);
module_exit(spinlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ANVITHA");
MODULE_DESCRIPTION("Spinlock program");

/* Here we used only spin_lock because No interrupt handler involved, only process context(kernel threads), so interrupt disabling not required*/
