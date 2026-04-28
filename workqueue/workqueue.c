#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

#define WQ_NAME "wq_example"

/* Dedicated workqueue */
static struct workqueue_struct *wq;

/* Work items */
static struct work_struct immediate_work;
static struct delayed_work delayed_work_item;

/* -------------------------------------------------- */
/* Immediate work handler */
static void immediate_work_fn(struct work_struct *work)
{
    pr_info("wq_example: Immediate work executed (PID=%d)\n",
            current->pid);
}

/* -------------------------------------------------- */
/* Delayed work handler */
static void delayed_work_fn(struct work_struct *work)
{
    pr_info("wq_example: Delayed work executed after 3 seconds\n");
}

/* -------------------------------------------------- */
static int __init wq_example_init(void)
{
    pr_info("wq_example: Module loaded\n");

    /* Create a single-threaded workqueue */
    wq = alloc_workqueue(WQ_NAME,
                          WQ_UNBOUND | WQ_MEM_RECLAIM,
                          1);
    if (!wq) {
        pr_err("wq_example: Failed to create workqueue\n");
        return -ENOMEM;
    }

    /* Initialize work items */
    INIT_WORK(&immediate_work, immediate_work_fn);
    INIT_DELAYED_WORK(&delayed_work_item, delayed_work_fn);

    /* Queue work */
    queue_work(wq, &immediate_work);
    queue_delayed_work(wq, &delayed_work_item,
                       msecs_to_jiffies(3000));

    pr_info("wq_example: Work queued successfully\n");
    return 0;
}

/* -------------------------------------------------- */
static void __exit wq_example_exit(void)
{
    pr_info("wq_example: Module unloading\n");

    /* Ensure all work is finished */
    cancel_delayed_work_sync(&delayed_work_item);
    flush_workqueue(wq);
    destroy_workqueue(wq);

    pr_info("wq_example: Workqueue destroyed\n");
}

module_init(wq_example_init);
module_exit(wq_example_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("Linux Kernel 6.8 Friendly Workqueue Example");

/*#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

static struct work_struct my_work;

void my_work_handler(struct work_struct *my_work)
{
pr_info("Workqueue example: INside the work queue handler\n");
msleep(100);
pr_info("WOrk queue handler finshed\n");
}

static int __init my_module_init(void)
{
pr_info("Module loaded\n");
INIT_WORK(&my_work,my_work_handler);

schedule_work(&my_work);
pr_info("Work scheduled\n");
return 0;
}

static void __exit my_module_exit(void)
{
cancel_work_sync(&my_work);
pr_info("Module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("A simple workqueue example");
*/
