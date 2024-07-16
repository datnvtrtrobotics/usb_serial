// serial.h

#ifndef SERIAL_H
#define SERIAL_H

#include <pthread.h>
#include <termios.h>

int configure_serial_port(int fd, int baud_rate);

void* receive_data(void* arg);

void* send_data(void* arg);

int start_serial_communication(pthread_t *recv_thread, pthread_t *send_thread, int fd);

#endif // SERIAL_H
