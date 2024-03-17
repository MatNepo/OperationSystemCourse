#ifndef PIPE_LOCAL_H
#define PIPE_LOCAL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PUBLIC "/tmp/PUBLIC"
#define SERVER_FIFO_NAME "/tmp/server_fifo"
#define B_SIZ (PIPE_BUF / 2)

struct message {
    char fifo_name[B_SIZ];
    char cmd_line[B_SIZ];
};

#endif // PIPE_LOCAL_H
