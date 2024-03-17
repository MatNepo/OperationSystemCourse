#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " command1 command2 ... commandN" << std::endl;
        return 1;
    }

    std::vector<int> pipes;
    pipes.push_back(STDIN_FILENO);

    for (int i = 1; i < argc; ++i) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return 1;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            // Child process
            close(pipefd[0]);  // Close the read end of the pipe
            if (i < argc - 1) {
                // If not the last command, redirect stdout to the write end of the pipe
                dup2(pipefd[1], STDOUT_FILENO);
            }

            close(pipefd[1]);  // Close the write end of the pipe

            // Execute the command
            const char* command = argv[i];
            char* args[] = {const_cast<char*>(command), nullptr};
            execvp(command, args);
            perror("execvp");
            _exit(1);
        } else {
            // Parent process
            close(pipefd[1]);  // Close the write end of the pipe
            pipes.push_back(pipefd[0]);
        }
    }

    // Wait for all child processes to finish
    for (int i = 1; i < argc; ++i) {
        int status;
        wait(nullptr);
    }

    // Close all open pipe file descriptors
    for (int fd : pipes) {
        close(fd);
    }

    return 0;
}
