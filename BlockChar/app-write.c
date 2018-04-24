#include <stdio.h>

int main()
{
	FILE *fp = NULL;
	char Buf[128];
	
	
	/*打开设备文件*/
	fp = fopen("/dev/memdev","r+");
	if (fp == NULL)
	{
		printf("Open Dev memdev Error!\n");
		return -1;
	}
	
	/*写入设备*/
	strcpy(Buf,"memdev is char dev!");
	printf("Write BUF: %s\n",Buf);
	fwrite(Buf, sizeof(Buf), 1, fp);
	
	sleep(5);
	fclose(fp);
	
	return 0;	

}
