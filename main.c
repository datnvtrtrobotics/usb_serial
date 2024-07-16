
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "serial.h"

#define SERIAL_PORT "/dev/ttyUSB0"
#define BAUD_RATE B115200

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

    if (start_serial_communication(&recv_thread, &send_thread, fd) != 0) {
        close(fd);
        return 1;
    }

    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);
    
    close(fd);
    return 0;
}
