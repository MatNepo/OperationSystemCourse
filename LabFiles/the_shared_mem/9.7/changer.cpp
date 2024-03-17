#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define MAX_SEGMENTS 3
#define SEGMENT_SIZE 1023

void changeByte(char *segment, size_t byteIndex, char newValue) {
    segment[byteIndex] = newValue;
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

        // ... (similar to the main program, filling the shared memory segments)
    }

    printf("\nAddresses and contents of shared memory segments:\n");
    for (int i = 0; i < num_segments; ++i) {
        printf("Segment %d: %p - Content: %s\n", i + 1, shm[i], shm[i]);
    }

    // Forking to create a child process for the changer
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(1);
    } else if (child_pid == 0) {
        // Child process (changer)
        int segmentIndex;
        size_t byteIndex;
        char newValue;

        printf("\nEnter the segment index to change: ");
        scanf("%d", &segmentIndex);

        if (segmentIndex < 1 || segmentIndex > num_segments) {
            printf("Invalid segment index. Exiting.\n");
            exit(1);
        }

        printf("Enter the byte index to change: ");
        scanf("%zu", &byteIndex);

        if (byteIndex >= SEGMENT_SIZE) {
            printf("Invalid byte index. Exiting.\n");
            exit(1);
        }

        printf("Enter the new value for the byte: ");
        scanf(" %c", &newValue);

        changeByte(shm[segmentIndex - 1], byteIndex, newValue);

        // Detach and exit
        shmdt(shm[segmentIndex - 1]);
        exit(0);
    } else {
        // Parent process (main)
        wait(NULL); // Wait for the child process to finish

        // Display the updated content
        printf("\nUpdated content after changer:\n");
        for (int i = 0; i < num_segments; ++i) {
            printf("Segment %d: %p - Content: %s\n", i + 1, shm[i], shm[i]);
        }

        // Detach and delete shared memory
        for (int i = 0; i < num_segments; ++i) {
            shmdt(shm[i]);
        }

        for (int i = 0; i < num_segments; ++i) {
            shmctl(shmid[i], IPC_RMID, NULL);
        }
    }

    return 0;
}
