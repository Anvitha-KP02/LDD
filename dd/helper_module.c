// helper_module.c
#include <linux/module.h>
#include <linux/kernel.h>

void helper_function(void)
{
    pr_info("Helper function called\n");
}

EXPORT_SYMBOL(helper_function);   // Export symbol for other modules

static int __init helper_init(void)
{
    pr_info("Helper module loaded\n");
    return 0;
}

static void __exit helper_exit(void)
{
    pr_info("Helper module removed\n");
}

module_init(helper_init);
module_exit(helper_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Helper Module");
