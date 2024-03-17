#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_SEGMENTS 3
#define SEGMENT_SIZE 1023

void displaySegments(char *shm[], int num_segments) {
    printf("\nAddresses and contents of shared memory segments:\n");
    for (int i = 0; i < num_segments; ++i) {
        printf("Segment %d: %p - Content: %s\n", i + 1, shm[i], shm[i]);
    }
}

int main(void) {
    int num_segments;
    printf("Enter the number of shared memory segments: ");
    scanf("%d", &num_segments);

    if (num_segments <= 0 || num_segments > MAX_SEGMENTS) {
        printf("Invalid number of segments. Exiting.\n");
        return 1;
    }

    int shmid[MAX_SEGMENTS];
    char *shm[MAX_SEGMENTS];

    for (int i = 0; i < num_segments; ++i) {
        if ((shmid[i] = shmget(IPC_PRIVATE, SEGMENT_SIZE, IPC_CREAT | 0666)) < 0) {
            perror("shmget");
            exit(1);
        }

        if ((shm[i] = (char *)shmat(shmid[i], 0, 0)) == (char *)-1) {
            perror("shmat");
            exit(1);
        }

        char fill_char = '0' + i + 1;
        for (size_t j = 0; j < SEGMENT_SIZE; ++j) {
            if (j <= 1021) {
                shm[i][j] = fill_char;
            } else {
                shm[i][j] = 'A' + (j - 1022) % 26;
            }
        }
    }

    // Создаем FIFO для обмена информацией между процессами
    if (mkfifo("segment_fifo", 0666) == -1) {
        perror("mkfifo");
        exit(1);
    }

    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(1);
    }

    if (child_pid == 0) {
        // Процесс-потомок отвечает за изменение сегментов
        execl("./modify_segment", "./modify_segment", NULL);
        perror("execl");
        exit(1);
    } else {
        // Процесс-родитель ожидает завершения процесса-потомка
        waitpid(child_pid, NULL, 0);

        // Чтение результатов из FIFO
        FILE *fifo = fopen("segment_fifo", "r");
        if (fifo == NULL) {
            perror("fopen");
            exit(1);
        }

        int modifiedSegment;
        int modifiedByte;
        char modifiedContent[SEGMENT_SIZE];

        fscanf(fifo, "%d %d %s", &modifiedSegment, &modifiedByte, modifiedContent);
        fclose(fifo);

        // Вывод изменений
        printf("\nModified Segment %d at Byte %d:\n", modifiedSegment, modifiedByte);
        printf("Previous Content: %s\n", shm[modifiedSegment - 1]);
        printf("New Content: %s\n", modifiedContent);

        // Обновление сегментов
        strcpy(shm[modifiedSegment - 1], modifiedContent);

        // Отображение обновленных сегментов
        displaySegments(shm, num_segments);

        // Удаление FIFO
        unlink("segment_fifo");

        // Отключение и удаление сегментов
        for (int i = 0; i < num_segments; ++i) {
            shmdt(shm[i]);
            shmctl(shmid[i], IPC_RMID, NULL);
        }
    }

    return 0;
}
