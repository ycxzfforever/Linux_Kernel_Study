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
}


int timer_init()
{
	init_timer(&timer);
	timer.data = 5;
	timer.expires = jiffies + (20 * HZ);
	timer.function = timer_function;
	add_timer(&timer);
	
	return 0;
}


void timer_exit()
{
	del_timer( &timer );
}

module_init(timer_init);
module_exit(timer_exit);
