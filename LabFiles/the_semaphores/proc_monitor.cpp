
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main()
{
    printf("Type 'e' + Enter to exit or any other key + Enter to refresh monitor\n");
    while (getchar() != 'e')
    {
        key_t key;
        int semid;
        int result;
        if ((key = ftok(".", 'J')) == -1)
        {
            perror("ftok");
            exit(1);
        }
        if ((key = ftok(".", 'J')) == -1)
        {
            perror("ftok");
            exit(1);
        }
        printf("Waiting semaphores: %d\n", result);
        printf("Type 'e' + Enter to exit or any other key + Enter to refresh monitor\n");
    }
    return 0;
}