/*
** identified_echo_server.cpp -- echo server with client identification
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#define SOCK_PATH "echo_socket"

// Структура для передачи данных в поток
struct ThreadData {
    int client_id;
    int client_socket;
};

// Обработка клиента в отдельном потоке
void *handle_client(void *arg);

int main(void)
{
    int server_socket, client_socket, len;
    struct sockaddr_un local, remote;
    char str[100];

    if ((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);

    if (bind(server_socket, (struct sockaddr *)&local, len) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(server_socket, 5) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Server is ready to accept connections.\n");

    int client_id = 1;  // Идентификатор клиента

    while (1) {
        int thread_data_size = sizeof(struct ThreadData);
        struct ThreadData *data = (struct ThreadData *)malloc(thread_data_size);

        len = sizeof(remote);

        if ((client_socket = accept(server_socket, (struct sockaddr *)&remote, (socklen_t *)&len)) == -1) {
            perror("accept");
            exit(1);
        }

        // Передача данных в новый поток для обработки клиента
        data->client_id = client_id++;
        data->client_socket = client_socket;

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, (void *)data) != 0) {
            perror("pthread_create");
            close(client_socket);
            free(data);
            continue;
        }

        // Отсоединение потока, чтобы избежать утечек ресурсов
        pthread_detach(thread);
    }

    return 0;
}

// Обработка клиента в отдельном потоке
void *handle_client(void *arg) {
    struct ThreadData *data = (struct ThreadData *)arg;
    int client_socket = data->client_socket;
    int client_id = data->client_id;
    free(data);  // Освобождение памяти после передачи данных в поток

    printf("New client connected. (Client ID: %d)\n", client_id);

    char str[100];
    int n;

    do {
        // Прием данных от клиента
        n = recv(client_socket, str, 100, 0);
        if (n <= 0) {
            if (n < 0) perror("recv");
            break;
        }

        // Отправка данных обратно клиенту
        if (send(client_socket, str, n, 0) < 0) {
            perror("send");
            break;
        }

        // Вывод в окно сервера с указанием идентификатора клиента
        printf("Received and echoed from client %d (Client ID: %d): %.*s\n", client_socket, client_id, n, str);
    } while (1);

    // Вывод сообщения о закрытии соединения клиента
    printf("Connection closed by client %d (Client ID: %d).\n", client_socket, client_id);

    close(client_socket);
    pthread_exit(NULL);
}
