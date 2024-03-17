#include "p.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>

int main()
{
    int publicfifo, privatefifo;
    struct message msg;
    static char buffer[PIPE_BUF];

    /* Создание очереди типа public FIFO */
    mknod(PUBLIC, S_IFIFO | 0666, 0);

    /* Открыть public FIFO на чтение */
    if ((publicfifo = open(PUBLIC, O_RDONLY)) == -1)
    {
        perror(PUBLIC);
        exit(1);
    }

    while (1)
    {
        /* Чтение сообщения от клиента */
        if (read(publicfifo, (char *)&msg, sizeof(msg)) <= 0)
        {
            continue; // Пропускаем пустые сообщения
        }

        /* Создание дочернего процесса */
        pid_t child_pid = fork();

        if (child_pid == -1)
        {
            perror("fork");
        }
        else if (child_pid == 0)
        {
            /* Дочерний процесс - выполняет команду */
            close(publicfifo); // Закрываем public FIFO в дочернем процессе

            // Открываем приватное FIFO клиента для записи
            if ((privatefifo = open(msg.fifo_name, O_WRONLY)) == -1)
            {
                perror(msg.fifo_name);
                exit(1);
            }

            // Выполняем команду и отправляем результат в приватное FIFO
            FILE *fin = popen(msg.cmd_line, "r");
            write(privatefifo, "\n", 1);

            while (fgets(buffer, sizeof(buffer), fin) != NULL)
            {
                write(privatefifo, buffer, strlen(buffer));
            }

            pclose(fin);
            close(privatefifo);
            exit(0); // Завершаем дочерний процесс
        }
        else
        {
            /* Родительский процесс - продолжает ожидать новых сообщений */
            // Ничего не делаем в этом месте
        }
    }

    // После завершения основного цикла, ожидаем завершения всех дочерних процессов
    while (wait(NULL) > 0) {
        // Пустое тело цикла
    }

    return 0;
}
