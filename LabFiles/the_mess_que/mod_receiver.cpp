#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
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
    long msgtype; // Переменная для выбора типа сообщения, который будет приниматься

    // Генерация ключа для очереди сообщений
    if ((key = ftok(".", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }

    // Подключение к очереди сообщений
    if ((msqid = msgget(key, 0644)) == -1) {
        perror("msgget");
        exit(1);
    }

    // Запрос у пользователя типа сообщений для приема
    printf("Enter the message type to receive (1 or 2): ");
    scanf("%ld", &msgtype);

    printf("Receiver: ready to receive messages of type %ld.\n", msgtype);

    for (;;) {
        // Принятие сообщения заданного типа
        if (msgrcv(msqid, &buf, sizeof(buf.mtext), msgtype, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        printf("Receiver: \"%s\"", buf.mtext);
    }

    return 0;
}
