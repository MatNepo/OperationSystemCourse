#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s cmd1 [cmd2 ...]\n", argv[0]);
        exit(1);
    }

    int num_cmds = argc - 1;
    int pipes[num_cmds - 1][2];  // Массив для хранения дескрипторов конвейера

    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < num_cmds; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) { // Дочерний процесс
            // Если это не первая команда, перенаправляем стандартный ввод
            if (i > 0) {
                dup2(pipes[i - 1][0], 0);
                close(pipes[i - 1][1]);
            }
            // Если это не последняя команда, перенаправляем стандартный вывод
            if (i < num_cmds - 1) {
                dup2(pipes[i][1], 1);
                close(pipes[i][0]);
            }

            // Выполняем команду
            execlp(argv[i + 1], argv[i + 1], NULL);
            perror("execlp");
            exit(1);
        }
    }

    // Закрыть все дескрипторы конвейера в родительском процессе
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Дожидаемся завершения всех дочерних процессов
    for (int i = 0; i < num_cmds; i++) {
        wait(NULL);
    }

    return 0;
}
