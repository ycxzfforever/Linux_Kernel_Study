
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
   unsigned long remain_size;
} Mem_Dev;

#endif /* _MEMDEV_H_ */
