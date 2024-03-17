#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_SEGMENTS 3

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
        size_t shm_size_bytes;
        printf("Enter the size of shared memory segment %d in bytes: ", i + 1);
        scanf("%zu", &shm_size_bytes);

        if ((shmid[i] = shmget(IPC_PRIVATE, shm_size_bytes, IPC_CREAT | 0666)) < 0) {
            perror("shmget");
            exit(1);
        }

        if ((shm[i] = (char *)shmat(shmid[i], 0, 0)) == (char *)-1) {
            perror("shmat");
            exit(1);
        }

        // Заполнение сегмента определенным образом в зависимости от размера
        char fill_char = '0' + i + 1;
        for (size_t j = 0; j < shm_size_bytes; ++j) {
            if (j <= 1022) {
                shm[i][j] = fill_char;
            } else {
                shm[i][j] = 'A' + (j - 1023) % 26;
            }
        }
    }

    printf("\nAddresses and contents of shared memory segments:\n");
    for (int i = 0; i < num_segments; ++i) {
        printf("Segment %d: %p - Content: %s\n", i + 1, shm[i], shm[i]);
    }

    for (int i = 0; i < num_segments; ++i) {
        shmdt(shm[i]);
    }

    for (int i = 0; i < num_segments; ++i) {
        shmctl(shmid[i], IPC_RMID, NULL);
    }

    return 0;
}
