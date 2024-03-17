#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_SEGMENTS 3

int main() {
    int num_segments;
    std::cout << "Enter the number of shared memory segments: ";
    std::cin >> num_segments;

    if (num_segments <= 0 || num_segments > MAX_SEGMENTS) {
        std::cout << "Invalid number of segments. Exiting.\n";
        return 1;
    }

    int shmid[MAX_SEGMENTS];
    char *shm[MAX_SEGMENTS];

    for (int i = 0; i < num_segments; ++i) {
        size_t shm_size_bytes;
        std::cout << "Enter the size of shared memory segment " << i + 1 << " in bytes: ";
        std::cin >> shm_size_bytes;

        if ((shmid[i] = shmget(IPC_PRIVATE, shm_size_bytes, IPC_CREAT | 0666)) < 0) {
            perror("shmget");
            exit(1);
        }

        if ((shm[i] = (char *)shmat(shmid[i], 0, 0)) == (char *)-1) {
            perror("shmat");
            exit(1);
        }

        // Инициализация содержимого сегмента
        char fill_char = '0' + i + 1;
        for (size_t j = 0; j < shm_size_bytes; ++j) {
            if (j <= 1022) {
                shm[i][j] = fill_char;
            } else {
                shm[i][j] = 'A' + (j - 1023) % 26;
            }
        }

        // Отображение диапазона инициализированной памяти
        std::cout << "Segment " << i + 1 << ": Initialized Memory Range: " << (void*)shm[i] << " to " << (void*)(shm[i] + 1022) << std::endl;
        // Отображение диапазона неинициализированной памяти
        std::cout << "Segment " << i + 1 << ": Uninitialized Memory Range: " << (void*)(shm[i] + 1023) << " to " << (void*)(shm[i] + shm_size_bytes - 1) << std::endl;
    }

    std::cout << "\nAddresses and contents of shared memory segments:\n";
    for (int i = 0; i < num_segments; ++i) {
        std::cout << "Segment " << i + 1 << ": " << (void*)shm[i] << " - Content: " << shm[i] << std::endl;
    }

    // Модификация определенного байта в определенном сегменте
    int segment_to_modify, byte_to_modify;
    char value_to_write;

    std::cout << "\nEnter the segment number to modify: ";
    std::cin >> segment_to_modify;

    if (segment_to_modify < 1 || segment_to_modify > num_segments) {
        std::cout << "Invalid segment number. Exiting.\n";
        return 1;
    }

    std::cout << "Enter the byte number to modify: ";
    std::cin >> byte_to_modify;

    // Проверка, выходит ли номер байта за пределы исходного размера
    size_t original_size = std::strlen(shm[segment_to_modify - 1]);
    if (byte_to_modify >= original_size) {
        // Изменение размера сегмента разделяемой памяти
        size_t new_size = byte_to_modify + 1;
        if (shmctl(shmid[segment_to_modify - 1], IPC_RMID, NULL) == -1) {
            perror("shmctl");
            exit(1);
        }
        if ((shmid[segment_to_modify - 1] = shmget(IPC_PRIVATE, new_size, IPC_CREAT | 0666)) < 0) {
            perror("shmget");
            exit(1);
        }
        if ((shm[segment_to_modify - 1] = (char *)shmat(shmid[segment_to_modify - 1], 0, 0)) == (char *)-1) {
            perror("shmat");
            exit(1);
        }
        // Копирование оригинального содержимого в измененный сегмент
        std::strcpy(shm[segment_to_modify - 1], shm[segment_to_modify - 1]);
    }

    // Очистка символа новой строки из буфера ввода
    while (std::cin.get() != '\n');

    std::cout << "Enter the value to write: ";
    value_to_write = std::cin.get();

    if (value_to_write == '\n') {
        // Пользователь нажал Enter, удаляем байт
        std::cout << "Removing byte at position " << byte_to_modify << " in segment " << segment_to_modify << ".\n";
        shm[segment_to_modify - 1][byte_to_modify] = '\0';
    } else {
        // Запись нового значения в указанный байт
        std::cout << "Writing '" << value_to_write << "' to byte at position " << byte_to_modify << " in segment " << segment_to_modify << ".\n";
        shm[segment_to_modify - 1][byte_to_modify] = value_to_write;
    }

    std::cout << "\nAddresses and contents of shared memory segments after modification:\n";
    for (int i = 0; i < num_segments; ++i) {
        std::cout << "Segment " << i + 1 << ": " << (void*)shm[i] << " - Content: " << shm[i] << std::endl;
    }

    // Отсоединение и удаление сегментов
    for (int i = 0; i < num_segments; ++i) {
        shmdt(shm[i]);
    }

    for (int i = 0; i < num_segments; ++i) {
        shmctl(shmid[i], IPC_RMID, NULL);
    }

    return 0;
}
