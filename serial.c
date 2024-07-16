
#include "serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

int configure_serial_port(int fd, int baud_rate) {
    struct termios tty;

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }

    cfsetospeed(&tty, baud_rate);
    cfsetispeed(&tty, baud_rate);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~CRTSCTS;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }

    tcflush(fd, TCIFLUSH);

    return 0;
}

void* receive_data(void* arg) {
    int fd = *((int*)arg);
    char buf[256];

    while (1) {
        int n = read(fd, buf, sizeof(buf) - 1);
        if (n < 0) {
            perror("Lỗi khi đọc dữ liệu từ cổng serial");
            break;
        }
        if (n == 0) {
            printf("Không có dữ liệu\n");
            continue;
        }
        buf[n] = '\0';
        printf("Dữ liệu nhận được: %s\n", buf);
    }

    return NULL;
}

void* send_data(void* arg) {
    int fd = *((int*)arg);
    const char *data = "Hello, serial port!";
    int len = strlen(data);

    while (1) {
        int n = write(fd, data, len);
        if (n < 0) {
            perror("Lỗi khi gửi dữ liệu");
            break;
        }
        printf("Đã gửi %d byte: %s\n", n, data);
        sleep(1);
    }

    return NULL;
}

int start_serial_communication(pthread_t *recv_thread, pthread_t *send_thread, int fd) {
    int *fd_ptr = malloc(sizeof(int));
    if (!fd_ptr) {
        perror("malloc");
        return -1;
    }
    *fd_ptr = fd;

    if (pthread_create(recv_thread, NULL, receive_data, fd_ptr) != 0) {
        perror("Không thể tạo luồng nhận dữ liệu");
        free(fd_ptr);
        return -1;
    }

    if (pthread_create(send_thread, NULL, send_data, fd_ptr) != 0) {
        perror("Không thể tạo luồng truyền dữ liệu");
        free(fd_ptr);
        return -1;
    }


    return 0;
}
