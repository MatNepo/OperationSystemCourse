#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main(void)
{
    key_t key;
    int semid;

    // Создаем ключ для семафора
    if ((key = ftok(".", 'J')) == -1) {
        perror("ftok");
        exit(1);
    }

    // Создаем семафор или получаем доступ к существующему
    if ((semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        perror("semget");
        exit(1);
    }

    // Устанавливаем начальное значение семафора (например, 1)
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } argument;

    argument.val = 1;
    if (semctl(semid, 0, SETVAL, argument) == -1) {
        perror("semctl");
        exit(1);
    }

    printf("Semaphore created with key 0x%lx\n", (long)key);

    return 0;
}
