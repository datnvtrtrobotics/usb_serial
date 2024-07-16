#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>

#define SERIAL_PORT "/dev/ttyUSB0"
#define BAUD_RATE B115200

// Cấu hình cổng serial
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

// Hàm cho luồng nhận dữ liệu
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

// Hàm cho luồng truyền dữ liệu
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

int main() {
    int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_SYNC);

    if (fd < 0) {
        perror("Không thể mở cổng serial");
        return 1;
    }

    if (configure_serial_port(fd, BAUD_RATE) != 0) {
        close(fd);
        return 1;
    }

    pthread_t recv_thread, send_thread;

    // Tạo luồng nhận dữ liệu
    if (pthread_create(&recv_thread, NULL, receive_data, &fd) != 0) {
        perror("Không thể tạo luồng nhận dữ liệu");
        close(fd);
        return 1;
    }

    // Tạo luồng truyền dữ liệu
    if (pthread_create(&send_thread, NULL, send_data, &fd) != 0) {
        perror("Không thể tạo luồng truyền dữ liệu");
        close(fd);
        return 1;
    }

    // Đợi các luồng kết thúc
    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);

    close(fd);
    return 0;
}

