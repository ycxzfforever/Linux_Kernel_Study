#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/timer.h>  /*timer*/
#include <asm/uaccess.h>  /*jiffies*/

MODULE_LICENSE("GPL");

struct timer_list timer;

void timer_function(int para)
{
    printk("Timer Expired and para is %d !!\n",para);	
	timer.data = jiffies;
	timer.expires = jiffies + (3 * HZ);
	timer.function = timer_function;
	add_timer(&timer);
}


int __init timer_init(void)
{
	init_timer(&timer);
	timer.data = jiffies;
	timer.expires = jiffies + (3 * HZ);
	timer.function = timer_function;
	add_timer(&timer);
	
	return 0;
}


void __exit timer_exit(void)
{
	del_timer( &timer );
}

module_init(timer_init);
module_exit(timer_exit);
