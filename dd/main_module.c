// main_module.c
#include <linux/module.h>
#include <linux/kernel.h>

extern void helper_function(void);   // Function from helper module

static int __init main_init(void)
{
    pr_info("Main module loaded\n");

    helper_function();   // Calling function from helper module

    return 0;
}

static void __exit main_exit(void)
{
    pr_info("Main module removed\n");
}

module_init(main_init);
module_exit(main_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Main Module");
