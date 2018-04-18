#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>

char *buf1 = NULL;
char *buf2 = NULL;
int order;

int __init alloc_init(void)
{
	buf1 = kmalloc(100,GFP_KERNEL);
	memset(buf1,0,100);
	strcpy(buf1,"<<< --- Kmalloc Mem OK! --- >>>");
	printk("BUF 1 : %s\n",buf1);
	
	order = get_order(8192);
	buf2 = (char *)__get_free_pages(GFP_KERNEL,order);
	memset(buf2,0,8192);
	strcpy(buf2,"<<<--- Get Free Page OK! --- >>>");
	printk("BUF 2 : %s\n",buf2);
	return 0;
}


void __exit alloc_exit(void)
{
	kfree(buf1);
	free_pages((unsigned long)buf2,order);
	printk("<<< --- Module Exit! --->>>\n");
}

module_init(alloc_init);
module_exit(alloc_exit);

MODULE_LICENSE("GPL");
