#include "pipe_local.h"
#include <signal.h>

volatile sig_atomic_t serverRunning = 1;

void serverSigIntHandler(int signo) {
    if (signo == SIGINT) {
        serverRunning = 0;
        unlink(SERVER_FIFO_NAME);
        exit(EXIT_SUCCESS);
    }
}

int main(void) {
    signal(SIGINT, serverSigIntHandler);

    int publicfifo, dummyfifo;
    struct message msg;
    static char buffer[PIPE_BUF];  // Включаем определение буфера

    if (mkfifo(PUBLIC, 0666) == -1) {
        perror("Error creating public FIFO");
        exit(EXIT_FAILURE);
    }

    if ((publicfifo = open(PUBLIC, O_RDONLY)) == -1) {
        perror("Error opening public FIFO");
        exit(EXIT_FAILURE);
    }

    if ((dummyfifo = open(PUBLIC, O_WRONLY | O_NDELAY)) == -1) {
        perror("Error opening dummy FIFO");
        exit(EXIT_FAILURE);
    }

    while (serverRunning && read(publicfifo, (char *)&msg, sizeof(msg)) > 0) {
        int n, done = 0;
        int privatefifo;

        do {
            if ((privatefifo = open(msg.fifo_name, O_WRONLY | O_NDELAY)) == -1)
                sleep(3);
            else {
                FILE *fin = popen(msg.cmd_line, "r");

                write(privatefifo, "\n", 1);

                while ((n = read(fileno(fin), buffer, PIPE_BUF)) > 0) {
                    write(privatefifo, buffer, n);
                    memset(buffer, 0x0, PIPE_BUF);
                }

                pclose(fin);
                close(privatefifo);
                done = 1;
            }
        } while (++n < 5 && !done);

        if (!done)
            write(fileno(stderr), "\nNOTE: SERVER ** NEVER ** accessed private FIFO\n", 48);
    }

    close(publicfifo);
    unlink(PUBLIC);

    return 0;
}
