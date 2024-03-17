#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/msg.h>

#define SHM_SIZE 1024
#define MAX 5

int main(void) {
    char buffer[PIPE_BUF];
    char u_char = 'A';
    int i, n;
    int mid[MAX];
    key_t key;
    sem_t *sem;
    char *shmaddr;

    // Создание сегмента разделяемой памяти
    int shmid = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Ошибка создания сегмента разделяемой памяти");
        exit(1);
    }

    // Получение адреса разделяемой памяти
    shmaddr = static_cast<char*>(shmat(shmid, NULL, 0));
    if (shmaddr == reinterpret_cast<char*>(-1)) {
        perror("Ошибка подключения к сегменту разделяемой памяти");
        exit(1);
    }

    // Инициализация семафора
    sem = static_cast<sem_t*>(mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0));
    if (sem == MAP_FAILED) {
        perror("Ошибка отображения семафора");
        exit(1);
    }

    if (sem_init(sem, 1, 1) == -1) {
        perror("Ошибка инициализации семафора");
        exit(1);
    }

    for (i = 0; i < MAX; ++i, ++u_char) {
        key = ftok(".", u_char);
        if ((mid[i] = msgget(key, IPC_CREAT | 0660)) == -1) {
            perror("Ошибка создания очереди сообщений");
            exit(1);
        }
    }

    // Вывод информации об IPC-объектах
    FILE *fin = popen("ipcs", "r");
    while ((n = read(fileno(fin), buffer, PIPE_BUF)) > 0) {
        write(fileno(stdout), buffer, n);
    }
    pclose(fin);

    // Отсоединение сегмента разделяемой памяти
    if (shmdt(shmaddr) == -1) {
        perror("Ошибка отсоединения сегмента разделяемой памяти");
        exit(1);
    }

    // Удаление сегмента разделяемой памяти
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Ошибка удаления сегмента разделяемой памяти");
        exit(1);
    }

    // Удаление семафора
    if (sem_destroy(sem) == -1) {
        perror("Ошибка удаления семафора");
        exit(1);
    }

    for (i = 0; i < MAX; ++i)
        msgctl(mid[i], IPC_RMID, NULL);

    exit(0);
}
