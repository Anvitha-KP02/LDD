#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/kallsyms.h>
#include<linux/kdebug.h>

static int my_die_handler(struct notifier_block *nb, unsigned long val, void *data)
{
    pr_alert("Custom Exception Handler Invoked! Reason code: %lu\n", val);
    return NOTIFY_DONE;
}

// nb-> notifier block
// val-> reason code of exception
// data-> exception specific info

static struct notifier_block my_nb = {
    .notifier_call = my_die_handler,
    .next = NULL,
    .priority = 0,
};

static int __init exception_module_init(void)
{
    int a = 1, b = 0, c;

    pr_info("Registering die notifier...\n");
    register_die_notifier(&my_nb);

    pr_info("Simulating exception: division by zero...\n");

    c = a / b; // This causes a divide-by-zero exception
    pr_info("Result: %d\n", c);

    return 0;
}

static void __exit exception_module_exit(void)
{
    pr_info("Unregistering die notifier...\n");
    unregister_die_notifier(&my_nb);
}

module_init(exception_module_init);
module_exit(exception_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba - Demo");
MODULE_DESCRIPTION("Kernel Exception Handler Example (Divide by Zero)");


/*An exception is a CPU-generated error like divide-by-zero or page fault. In this program, we register a die notifier using register_die_notifier(). When a divide-by-zero occurs, the CPU generates a divide error exception, and the kernel calls notify_die(), which invokes our custom handler. The handler prints an alert message. This mechanism is mainly used for debugging and monitoring kernel exceptions.*/
