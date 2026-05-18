#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/select.h>

#define BUFFER_SIZE 256
#define TIMEOUT_SEC 5

int main(int argc, char *argv[]) {
    const char *device;

    if (argc > 1) {
        device = argv[1];
    } else {
        device = "/dev/ttyS0";
    }

    printf("Opening UART device: %s\n", device);

    int fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "ERROR: Cannot open %s - %s\n",
                device, strerror(errno));
        return EXIT_FAILURE;
    }

    printf("UART device opened successfully!\n");

    struct termios tty;

    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "WARNING: tcgetattr failed - %s\n",
                strerror(errno));
    } else {
        cfsetispeed(&tty, B9600);
        cfsetospeed(&tty, B9600);

        /* 8N1 config */
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |=  CS8;
        tty.c_cflag |=  CREAD | CLOCAL;
        tty.c_cflag &= ~CRTSCTS;

        /* Raw mode */
        tty.c_lflag = 0;
        tty.c_oflag = 0;

        /* No software flow control, clean input flags */
        tty.c_iflag &= ~(IXON | IXOFF | IXANY |
                         IGNBRK | BRKINT | PARMRK |
                         ISTRIP | INLCR | IGNCR | ICRNL);

        tty.c_cc[VMIN]  = 0;
        tty.c_cc[VTIME] = 0;

        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
            fprintf(stderr, "WARNING: tcsetattr failed - %s\n",
                    strerror(errno));
        } else {
            printf("UART configured: 9600 baud | 8N1\n");
        }
    }

    /* Transmit test message */
    const char *message = "Hello from UART - RISC-V ACT Challenge!\n";
    ssize_t bytes_written = write(fd, message, strlen(message));
    if (bytes_written < 0) {
        fprintf(stderr, "ERROR: Write failed - %s\n",
                strerror(errno));
    } else {
        printf("Sent (%zd bytes): %s", bytes_written, message);
    }

    /* Wait for response using select() */
    fd_set read_fds;
    struct timeval timeout;

    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    timeout.tv_sec  = TIMEOUT_SEC;
    timeout.tv_usec = 0;

    printf("Waiting for incoming data (%d sec timeout)...\n", TIMEOUT_SEC);

    int activity = select(fd + 1, &read_fds, NULL, NULL, &timeout);

    if (activity < 0) {
        fprintf(stderr, "ERROR: select() failed - %s\n",
                strerror(errno));
    } else if (activity == 0) {
        printf("Timeout: No data received within %d seconds.\n",
               TIMEOUT_SEC);
    } else {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0) {
            fprintf(stderr, "ERROR: Read failed - %s\n",
                    strerror(errno));
        } else {
            buffer[bytes_read] = '\0';
            printf("Received (%zd bytes): %s\n",
                   bytes_read, buffer);
        }
    }

    close(fd);
    printf("UART device closed. Done.\n");
    return EXIT_SUCCESS;
}
