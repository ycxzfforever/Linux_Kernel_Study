#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static int year=2007;
module_param(year, int, 0644);

int __init hello_init(void)
{
	
	printk(KERN_WARNING "Hello World %d!\n",year);
	return 0;
}


void __exit hello_exit(void)
{
	printk(KERN_WARNING  "Hello Exit!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bright Yang");
MODULE_DESCRIPTION("A simple Hello World Module");

