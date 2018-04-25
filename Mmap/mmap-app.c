#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main()
{
	int fd;
	char *start;
	char buf[100]={' '};
	int DATA_LEN = sizeof(buf);
	/*打开文件*/
	fd = open("testfile",O_CREAT|O_RDWR,S_IRWXU);	
	write(fd,"18188889999",DATA_LEN);	     
	start=mmap(NULL,DATA_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	
	/* 读出数据 */
	strcpy(buf,start);
	printf("start addr= %X\n",(unsigned int)start);	
	printf("old buf = %s\n",buf);

	/* 写入数据 */
	strcpy(start,"Buf Is Not Null!");
	//msync(start, DATA_LEN, MS_SYNC);//告诉kernel将数据写入磁盘
	printf("new buf = %s\n",buf);
	munmap(start,DATA_LEN); /*解除映射*/
	close(fd);  
	
	return 0;	
}
