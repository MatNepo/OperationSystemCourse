#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

int main(void)
{
    struct my_msgbuf buf;
    int msqid;
    key_t key;

    // Генерация ключа для очереди сообщений
    if ((key = ftok(".", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }

    // Создание или подключение к очереди сообщений
    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    printf("Enter lines of text, ^D to quit:\n");

    while (fgets(buf.mtext, sizeof buf.mtext, stdin) != NULL) {
        int len = strlen(buf.mtext);

        // Определение типа сообщения в зависимости от первого символа
        if (buf.mtext[0] == '!') {
            buf.mtype = 2; // Тип сообщения 2 для сообщений, начинающихся с '!'
        } else {
            buf.mtype = 1; // Тип сообщения 1 для остальных сообщений
        }

        // Отправка сообщения в очередь
        if (msgsnd(msqid, &buf, len + 1, 0) == -1) /* +1 for '\0' */
            perror("msgsnd");
    }

    // Удаление очереди сообщений
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    return 0;
}
