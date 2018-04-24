#ifndef _MEMDEV_H_
#define _MEMDEV_H_

#include <linux/ioctl.h>

#ifndef MEMDEV_MAJOR
#define MEMDEV_MAJOR 0   
#endif

#ifndef MEMDEV_NR_DEVS
#define MEMDEV_NR_DEVS 3    
#endif

#ifndef MEMDEV_SIZE
#define MEMDEV_SIZE 4096
#endif


typedef struct Mem_Dev {
   char *data;
   struct Mem_Dev *next;   /* next listitem */
   unsigned long size;  
   unsigned long wp, rp;  /* 读写数据的位置 */
   struct semaphore sem;    /* 定义信号量 */
   wait_queue_head_t inq;  /* 不要使用指针方式，否则要用kmalloc先为其分配空间 */
} Mem_Dev;

/* 定义幻数 */
#define MEMDEV_IOC_MAGIC  'k'


/* 定义命令 */
#define MEMDEV_IOCPRINT   _IO(MEMDEV_IOC_MAGIC, 1)
#define MEMDEV_IOCGETDATA _IOR(MEMDEV_IOC_MAGIC, 2, int)
#define MEMDEV_IOCSETDATA _IOW(MEMDEV_IOC_MAGIC, 3, int)


#define MEMDEV_IOC_MAXNR 3

#endif /* _MEMDEV_H_ */
