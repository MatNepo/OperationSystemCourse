#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

void signal_handler(int signo) {
    printf("Received signal: %d\n", signo);
}

int main() {
    pid_t pid;
    int parent_signal, child_signal;

    // Инициализация генератора случайных чисел
    srand((unsigned int)time(NULL));

    // Устанавливаем обработчик сигнала для родительского и потомков
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);

    // Создаем процесс-потомок
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Код для процесса-потомка
        printf("Child process (PID=%d) is running.\n", getpid());
        sleep(2);

        // Ввод сигнала для отправки родителю
        printf("Enter signal number to send to the parent: ");
        scanf("%d", &parent_signal);
        printf("Child process (PID=%d) sends signal %d to the parent.\n", getpid(), parent_signal);
        kill(getppid(), parent_signal);
        sleep(2);  // Задержка для визуального контроля

        // Завершение процесса-потомка
        printf("Child process is exiting.\n");
    } else {
        // Код для родительского процесса
        printf("Parent process (PID=%d) is running.\n", getpid());

        // Ждем сигнала от потомка
        printf("Parent process is waiting for a signal from the child.\n");
        pause();  // Ждем сигнала
        sleep(2);  // Задержка для визуального контроля

        // Ввод сигнала для отправки потомку
        printf("Enter signal number to send to the child: ");
        scanf("%d", &child_signal);
        printf("Parent process (PID=%d) sends signal %d to the child.\n", getpid(), child_signal);
        kill(pid, child_signal);
        sleep(2);  // Задержка для визуального контроля

        // Ждем завершения потомка
        wait(NULL);
        printf("Parent process is exiting.\n");
    }

    return 0;
}
