#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/tracepoint.h>
#include <trace/events/exceptions.h>

/*
 * die tracepoint handler
 * Replaces die_notifier in kernel >= 6.6
 */
static void my_die_handler(void *ignore,
                           struct pt_regs *regs,
                           const char *str,
                           long err)
{
    pr_alert("Custom DIE handler invoked!\n");
    pr_alert("Reason: %s | Error code: %ld\n", str, err);
}

/*
 * Module init
 */
static int __init exception_module_init(void)
{
    int ret;

    pr_info("Registering die tracepoint...\n");

    ret = register_trace_die(my_die_handler, NULL);
    if (ret) {
        pr_err("Failed to register die tracepoint\n");
        return ret;
    }

    pr_info("Triggering exception using WARN_ON...\n");

    /* Safe exception trigger for demo */
    WARN_ON(1); //equilvalent to any exception we create

    return 0;
}

/*
 * Module exit
 */
static void __exit exception_module_exit(void)
{
    pr_info("Unregistering die tracepoint...\n");
    unregister_trace_die(my_die_handler, NULL);
}

module_init(exception_module_init);
module_exit(exception_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba - Demo");
MODULE_DESCRIPTION("Kernel 6.8 Exception Handler using die tracepoint");
