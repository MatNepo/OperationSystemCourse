#include "pipe_local_mod.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <csignal>

char publicfifo_name[B_SIZ];
int publicfifo;

void signal_int(int the_sig) {
    close(publicfifo);
    printf("Public FIFO closed\n");
    unlink(PUBLIC);
    printf("Public FIFO removed\n");
    exit(0);
}

int main(void) {
    int done, dummyfifo, privatefifo;
    static char buffer[PIPE_BUF];
    std::signal(SIGINT, signal_int);
    struct message msg;

    // Generate the public FIFO
    mknod(PUBLIC, S_IFIFO | 0666, 0);

    // OPEN the public FIFO for reading and writing
    if ((publicfifo = open(PUBLIC, O_RDONLY)) == -1 || (dummyfifo = open(PUBLIC, O_WRONLY | O_NDELAY)) == -1) {
        perror(PUBLIC);
        exit(1);
    }

    // Message can be read from the PUBLIC pipe
    while (read(publicfifo, (char *)&msg, sizeof(msg)) > 0) {
        if (fork() == 0) {
            std::signal(SIGINT, SIG_DFL);

            // Try OPEN of private FIFO
            if ((privatefifo = open(msg.fifo_name, O_WRONLY | O_NDELAY)) == -1)
                sleep(3);
            else { // OPEN successful
                FILE *fin = popen(msg.cmd_line, "r");
                write(privatefifo, "\n", 1);

                int n;
                while ((n = read(fileno(fin), buffer, PIPE_BUF)) > 0) {
                    write(privatefifo, buffer, n); // to private FIFO
                    memset(buffer, 0x0, PIPE_BUF); // Clear it out
                }

                pclose(fin);
                close(privatefifo);
            }

            // Record success
            exit(0);
        }
    }

    return 0;
}
