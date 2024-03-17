#include "pipe_local_mod.h"
#include <signal.h>

volatile sig_atomic_t clientRunning = 1;

void clientSigIntHandler(int signo) {
    if (signo == SIGINT) {
        clientRunning = 0;
    }
}

void handleErrorsAndTryStartServer() {
    if (fork() == 0) {
        execl("./pipe_server_mod", "./pipe_server_mod", NULL);
        _exit(EXIT_SUCCESS);
    }
    sleep(1);
}

int main(void) {
    signal(SIGINT, clientSigIntHandler);

    int privatefifo, publicfifo;
    struct message msg;
    static char buffer[PIPE_BUF];

    snprintf(msg.fifo_name, sizeof(msg.fifo_name), "/tmp/fifo%d", getpid());

    while (clientRunning) {
        // Ожидаем, пока сервер создаст private FIFO
        while (access(msg.fifo_name, F_OK) == -1) {
            usleep(100000); // Пауза 0.1 секунда
        }

        if ((publicfifo = open(PUBLIC, O_WRONLY | O_NDELAY)) == -1) {
            perror(PUBLIC);
            handleErrorsAndTryStartServer();
        } else {
            write(publicfifo, (char *)&msg, sizeof(msg));

            if ((privatefifo = open(msg.fifo_name, O_RDONLY)) == -1) {
                perror(msg.fifo_name);
                exit(EXIT_FAILURE);
            }

            int n;
            while ((n = read(privatefifo, buffer, PIPE_BUF)) > 0) {
                // Пишем вывод на stdout
                write(fileno(stdout), buffer, n);
            }

            close(privatefifo);
            close(publicfifo);
            // Нет необходимости unlink для private FIFO
        }
    }

    return 0;
}
