#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "common.h"

#define PROC_NAME "proc_basic"
#define BUF_LEN   256

static char proc_buffer[BUF_LEN];
static size_t proc_buffer_len;

	struct data d;
	struct result r;

static ssize_t proc_read(struct file *file,
                         char __user *user_buf,
                         size_t count,
                         loff_t *ppos)
{
    return simple_read_from_buffer(user_buf,
                                   count,
                                   ppos,
                                   proc_buffer,
                                   proc_buffer_len);
	
	if(copy_to_user(&r,proc_buffer,count))
		return -EINVAL;

	printk(KERN_INFO "Kernel sent the result\n");
}

static ssize_t proc_write(struct file *file,
                          const char __user *user_buf,
                          size_t count,
                          loff_t *ppos)
{
    if (count > BUF_LEN - 1){
	pr_info("error1\n");
        count = BUF_LEN - 1;
    }
    if (copy_from_user(proc_buffer, &d, count))
    {   
	pr_info("error2\n");
	    return -EFAULT;}


    proc_buffer[count] = '\0';

    /*if(proc_buffer[count-1]=='\n')
	    proc_buffer[count-1]='\0';*/
   // proc_buffer_len = count;

    pr_info("proc_basic: received  %s ", proc_buffer);

    switch(d.op)
    {
	    case '+': r.res=d.a + d.b;
		      break;
	    case '-': r.res=d.a - d.b;
		      break;
	    case '*': r.res=d.a * d.b;
		      break;
	    case '/': r.res=d.a / d.b;
		      break;
	    default: printk("Invalid option\n");
		     return -EINVAL;
    }
    //proc_buffer=result;
    pr_info("\n data is: %d %c %d\n",d.a,d.op,d.b);
    printk("proc_basic: result=%d %d\n",r.res, d.b);
    return count;
}

static const struct proc_ops proc_fops = {
    .proc_read  = proc_read,
    .proc_write = proc_write,
};

static int __init proc_basic_init(void)
{
    proc_create(PROC_NAME, 0666, NULL, &proc_fops);
    pr_info("proc_basic: loaded\n");
    return 0;
}

static void __exit proc_basic_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("proc_basic: unloaded\n");
}

module_init(proc_basic_init);
module_exit(proc_basic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("Basic procfs driver example");
