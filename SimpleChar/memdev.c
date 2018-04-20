#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef MODULE
#define MODULE
#endif

//#include <linux/config.h>
#include <linux/module.h>

#include <linux/kernel.h>   
#include <linux/init.h>     
#include <linux/slab.h>   
#include <linux/fs.h>       
#include <linux/errno.h>    
#include <linux/types.h>    
#include <linux/proc_fs.h>

#include <asm/system.h>  
#include <asm/uaccess.h>   /*使用copy_to_user,copy_from_user必须要包含该文件*/

#include "memdev.h"

MODULE_LICENSE("GPL");

Mem_Dev *mem_devices; 
int memdev_major = MEMDEV_MAJOR;

/*设备打开*/
int memdev_open(struct inode *inode, struct file *filp)
{
    Mem_Dev *dev;
    
    /*获取次设备号*/
    int num = MINOR(inode->i_rdev);

    dev = (Mem_Dev *)filp->private_data;
    if (!dev) 
    {
        if (num >= MEMDEV_NR_DEVS) 
            return -ENODEV;
        dev = &mem_devices[num];
        filp->private_data = dev; 
    }
    
    /*增加模块引用计数*/
    //MOD_INC_USE_COUNT; 
	try_module_get(THIS_MODULE);
    return 0;          
}

/*设备关闭*/
int memdev_release(struct inode *inode, struct file *filp)
{
    //MOD_DEC_USE_COUNT;
	module_put(THIS_MODULE);
    return 0;
}

/*设备读操作*/
ssize_t memdev_read(struct file *filp, char *buf, size_t count,
                loff_t *f_pos)
{
    Mem_Dev *dev = filp->private_data; 
    int pos = *f_pos;
    ssize_t ret = 0;
    
    /*判断读位置是否有效*/
    if (pos >= dev->size)
        goto out;
    if (pos + count > dev->size)
        count = dev->size - pos;
        
    if (!dev->data)
        goto out;
    
    /*读数据到用户空间*/
    if (copy_to_user(buf, &(dev->data[pos]), count)) 
    {
        ret = -EFAULT;
	goto out;
    }
    
    *f_pos += count;
    ret = count;
    
 out:
    return ret;
}

/*文件写操作*/
ssize_t memdev_write(struct file *filp, const char *buf, size_t count,
                loff_t *f_pos)
{
    Mem_Dev *dev = filp->private_data;
    int pos = *f_pos;
    ssize_t ret = -ENOMEM;

    /*判断写位置是否有效*/
    if (pos >= dev->size)
        goto out;
    if (pos + count > dev->size)
        count = dev->size - pos;
    
    /*从用户空间写入数据*/
    if (copy_from_user(&(dev->data[pos]), buf, count)) 
    {
        ret = -EFAULT;
	goto out;
    }

    *f_pos += count;
    ret = count;

  out:
    return ret;
}


/*文件定位*/
loff_t memdev_llseek(struct file *filp, loff_t off, int whence)
{
    Mem_Dev *dev = filp->private_data;
    loff_t newpos;

    switch(whence) {
      case 0: /* SEEK_SET */
        newpos = off;
        break;

      case 1: /* SEEK_CUR */
        newpos = filp->f_pos + off;
        break;

      case 2: /* SEEK_END */
        newpos = dev->size -1 + off;
        break;

      default: /* can't happen */
        return -EINVAL;
    }
    if (newpos<0) 
    	return -EINVAL;
    	
    filp->f_pos = newpos;
    return newpos;
}


/*
 * The following wrappers are meant to make things work with 2.0 kernels
 */

struct file_operations memdev_fops = {
    .llseek =     memdev_llseek,
    .read =       memdev_read,
    .write =      memdev_write,
    //.ioctl =      NULL,
    .open =       memdev_open,
    .release =    memdev_release,
};


/*卸载函数*/
void memdev_cleanup_module(void)
{
    int i;
    
    /*注销字符设备*/
    unregister_chrdev(memdev_major, "memdev");

    /*释放内存*/
    if (mem_devices) 
    {
        for (i=0; i<MEMDEV_NR_DEVS; i++)
	    kfree(mem_devices[i].data);
        kfree(mem_devices);
    }

}

/*加载函数*/
int memdev_init_module(void)
{
    int result, i;
    
    /*设置模块owner*/
    //SET_MODULE_OWNER(&memdev_fops);
 
    /*注册字符设备*/
    result = register_chrdev(memdev_major, "memdev", &memdev_fops);
    if (result < 0) 
    {
        printk(KERN_WARNING "mem: can't get major %d\n",memdev_major);
        return result;
    }
    if (memdev_major == 0) 
    	memdev_major = result; 

    /*为设备描述结构分配内存*/
    mem_devices = kmalloc(MEMDEV_NR_DEVS * sizeof(Mem_Dev), GFP_KERNEL);
    if (!mem_devices) 
    {
        result = -ENOMEM;
        goto fail;
    }
    memset(mem_devices, 0, MEMDEV_NR_DEVS * sizeof(Mem_Dev));
    
    /*为设备分配内存*/
    for (i=0; i < MEMDEV_NR_DEVS; i++) 
    {
        mem_devices[i].size = MEMDEV_SIZE;
        mem_devices[i].data = kmalloc(MEMDEV_SIZE, GFP_KERNEL);
        memset(mem_devices[i].data, 0, MEMDEV_SIZE);
    }
    
    return 0;

  fail:
    memdev_cleanup_module();
    return result;
}


module_init(memdev_init_module);
module_exit(memdev_cleanup_module);
