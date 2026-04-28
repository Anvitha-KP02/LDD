#include<linux/module.h>  // Required for loadable kernel modules
#include<linux/kernel.h>  //For pr_info() and kernel macros
#include<linux/kthread.h>  //For kernel thread APIs
#include<linux/delay.h>   // For ssleep()
#include<linux/init.h>    // For __init and __exit macros

#define NUM_THREADS 3     // creating 3 kernel threads

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("Multiple Kernel Threads Exaple");

static struct task_struct *thread[NUM_THREADS];   // task_struct represents a process/thread, array stores reference to all created threads

struct my_thread_info{
int id;
};

// kernel thread function

static int thread_fn(void *data)    // function executed by each kernel thread
{
struct my_thread_info *info=(struct my_thread_info*)data;
int count=0;

pr_info("kthread: thread %d started(PID=%d)\n",info->id,current->pid); // current is the pointer to current running task

while(!kthread_should_stop())  // chaecks if thread must exit, when this returns true kthread_stop will be called
{
pr_info("kthread: Thread %d running... count=%d\n",info->id,count++);
ssleep(1);
}

pr_info("kthread: Thread %d existing\n",info->id);
kfree(info);
return 0;
}

// Module_init

static int __init multi_kthread_init(void)
{
int i;
pr_info("kthread: Module loading with %d threads\n",NUM_THREADS);
for(i=0;i<NUM_THREADS;i++)
{
struct my_thread_info *info;
info=kmalloc(sizeof(*info),GFP_KERNEL);
if(!info)
return -ENOMEM;
info->id=i;  // each threads gets different ids

thread[i]=kthread_run(thread_fn,info,"kthread_%d",i);  // creates kernel thread
if(IS_ERR(thread[i]))   // if thread creation fails
{
pr_err("kthread: Failed to create thread %d\n",i);
kfree(info);
thread[i]=NULL;
}
}
return 0;
}

// Module_exit

static void __exit multi_kthread_exit(void)
{
int i;
pr_info("kthread: Module unloding\n");

for(i=0;i<NUM_THREADS;i++)
{
if(thread[i])
{
kthread_stop(thread[i]);   // sets stop flag, wakes thread if sleeping, waits until thread exits
pr_info("kthread: Thread %d stopped\n",i);
}
}

pr_info("kthread: All threads exited\n");
}

module_init(multi_kthread_init);
module_exit(multi_kthread_exit);
