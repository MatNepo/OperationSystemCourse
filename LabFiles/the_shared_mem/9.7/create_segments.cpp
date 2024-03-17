#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1023

extern int etext, edata, end;

main(void) 
{
	int shmid[3];
	char *data1, *data2, *data3;
	
	printf("Addresses in parent\n");
	printf("program text(etext): \t\t%10p\ninitialized data (edata):\t%10p\nuninitialized data (end): \t%10p\n", &etext, &edata, &end);
	for (int i = 0; i < 3; i++)
	{
		if ((shmid[i] = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT|0666)) < 0)
		{
			perror("shmget fail");
			exit(1);
		}
		switch (i)
		{
			case 0:
				data1 = (char*) shmat(shmid[i], 0, 0);
				printf("shared mem %d: \t\t\t%10p\n", i, data1);
				break;
			case 1:
				data2 = (char*) shmat(shmid[i], 0, 0);
				printf("shared mem %d: \t\t\t%10p\n", i, data2);
				break;
			case 2:
				data3 = (char*) shmat(shmid[i], 0, 0);
				printf("shared mem %d: \t\t\t%10p\n", i, data3);
				break;
		}
	}
	for (int i = 0; i < 1023; i++) 
	{
		data1[i] = '1';
		data2[i] = '2';
		data3[i] = '3';
	}
	data1[1023] = 'I';
	data1[1024] = 'N';
	data1[1025] = 'F';
	printf("1: %s\n", data1);
	printf("2: %s\n", data2);
	printf("3: %s\n", data3);
} 	  
