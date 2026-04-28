#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("IRQ invoking Workqueue example");

/*
 * Using keyboard IRQ (1) for demo purposes
 * In real drivers, this will be a device IRQ
 */
static int irq = 1;
module_param(irq, int, 0444);

static struct workqueue_struct *my_wq;
static struct work_struct irq_work;

/* ================= Workqueue Handler ================= */
static void work_handler(struct work_struct *work)
{
    pr_info("work_handler: running in process context\n");

    /* Heavy work is allowed */
    msleep(1000);

    pr_info("work_handler: completed\n");
}

/* ================= IRQ Handler ================= */
static irqreturn_t irq_handler(int irq, void *dev_id)
{
    pr_info("irq_handler: IRQ %d received\n", irq);

    /*
     * Defer heavy processing
     * queue_work() is IRQ-safe
     */
    queue_work(my_wq, &irq_work);

    return IRQ_HANDLED;
}

/* ================= Module Init ================= */
static int __init irq_wq_init(void)
{
    int ret;

    pr_info("irq_wq: init\n");

    /* Create single-threaded workqueue */
    my_wq = create_singlethread_workqueue("irq_wq");
    if (!my_wq)
        return -ENOMEM;

    INIT_WORK(&irq_work, work_handler);

    /* Register IRQ */
    ret = request_irq(irq,
                      irq_handler,
                      IRQF_SHARED,
                      "irq_wq_demo",
                      (void *)&irq);
    if (ret) {
        destroy_workqueue(my_wq);
        return ret;
    }

    pr_info("irq_wq: IRQ %d registered\n", irq);
    return 0;
}

/* ================= Module Exit ================= */
static void __exit irq_wq_exit(void)
{
    pr_info("irq_wq: exit\n");

    free_irq(irq, (void *)&irq);
    flush_workqueue(my_wq);
    destroy_workqueue(my_wq);
}

module_init(irq_wq_init);
module_exit(irq_wq_exit);
