#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

int main() {
    key_t key;
    int semid;

    char u_char = 'J';

    if ((key = ftok(".", u_char)) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((semid = semget(key, 1, 0)) == -1) {
        perror("semget");
        exit(1);
    }

    int ch;
    while (1) {
        printf("Press 'r' and Enter to update, or press 'q' to quit: ");
        ch = getchar();

        if (ch == 'q') {
            break; // выход из цикла при вводе 'q'
        } else if (ch == 'r') {
            int waitingProcesses = semctl(semid, 0, GETNCNT, 0);
            printf("Number of processes waiting: %d\n", waitingProcesses);
        }
        // Очистка буфера ввода
        while ((getchar()) != '\n');
    }

    return 0;
}
