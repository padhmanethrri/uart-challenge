/*
 * uart.c - UART Communication Program
 * =====================================
 * Author: Padhmanethrri S
 * Challenge: RISC-V ACT Framework Enablement Mentorship - 10xEngineers
 *
 * Description:
 *   This program opens and configures a UART serial interface on Linux
 *   using the termios API. It transmits a test message and waits for
 *   incoming data using select() with a non-blocking timeout.
 *
 *   This is directly relevant to RISC-V hardware validation where UART
 *   is used to read ACT test results from M-mode firmware running on
 *   boards like VisionFive 2 and Milk-V Jupiter.
 *
 * Usage:
 *   sudo ./uart [device] [baud_rate]
 *   sudo ./uart /dev/ttyS0 9600
 *   sudo ./uart /dev/ttyUSB0 115200
 *
 * Compile:
 *   make
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/select.h>

/* ── Constants ────────────────────────────────────────────────── */
#define BUFFER_SIZE   256
#define TIMEOUT_SEC   5
#define MAX_MSG_LEN   128
#define DEFAULT_DEV   "/dev/ttyS0"
#define DEFAULT_BAUD  9600

/* ── Function Prototypes ──────────────────────────────────────── */
int  open_uart(const char *device);
int  configure_uart(int fd, int baud_rate);
int  uart_send(int fd, const char *message);
int  uart_receive(int fd);
speed_t get_baud_constant(int baud_rate);
void print_usage(const char *prog_name);

/* ================================================================
 * open_uart()
 * Opens the UART device file.
 * Returns file descriptor on success, -1 on failure.
 * ================================================================ */
int open_uart(const char *device) {

    /* Validate device path */
    if (device == NULL || strlen(device) == 0) {
        fprintf(stderr, "ERROR: Device path is empty or NULL.\n");
        return -1;
    }

    if (strlen(device) > 64) {
        fprintf(stderr, "ERROR: Device path is too long.\n");
        return -1;
    }

    /* Open device - read/write, not controlling terminal, non-blocking */
    int fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd < 0) {
        /* Specific error messages for common failure cases */
        if (errno == ENOENT) {
            fprintf(stderr, "ERROR: Device %s not found.\n", device);
            fprintf(stderr, "TIP: Check available ports with: ls /dev/tty*\n");
        } else if (errno == EACCES) {
            fprintf(stderr, "ERROR: Permission denied on %s.\n", device);
            fprintf(stderr, "TIP: Try running with sudo, or: sudo chmod 666 %s\n", device);
        } else {
            fprintf(stderr, "ERROR: Cannot open %s - %s\n", device, strerror(errno));
        }
        return -1;
    }

    printf("UART device opened: %s\n", device);
    return fd;
}

/* ================================================================
 * get_baud_constant()
 * Converts integer baud rate to termios speed_t constant.
 * Returns B0 (invalid) if baud rate is not supported.
 * ================================================================ */
speed_t get_baud_constant(int baud_rate) {
    switch (baud_rate) {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        default:
            fprintf(stderr, "ERROR: Unsupported baud rate %d.\n", baud_rate);
            fprintf(stderr, "TIP: Supported rates: 9600, 19200, 38400, 57600, 115200, 230400\n");
            return B0;
    }
}

/* ================================================================
 * configure_uart()
 * Configures UART parameters using termios API.
 * Settings: 8N1 (8 data bits, no parity, 1 stop bit)
 * Returns 0 on success, -1 on failure.
 * ================================================================ */
int configure_uart(int fd, int baud_rate) {
    struct termios tty;

    /* Validate baud rate before doing anything */
    speed_t baud_const = get_baud_constant(baud_rate);
    if (baud_const == B0) {
        return -1;
    }

    /* Fetch current terminal settings */
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "ERROR: tcgetattr failed - %s\n", strerror(errno));
        return -1;
    }

    /* ── Baud Rate ── */
    cfsetispeed(&tty, baud_const);   /* Input baud rate  */
    cfsetospeed(&tty, baud_const);   /* Output baud rate */

    /* ── Control Flags ── */
    tty.c_cflag &= ~PARENB;          /* No parity                    */
    tty.c_cflag &= ~CSTOPB;          /* 1 stop bit                   */
    tty.c_cflag &= ~CSIZE;           /* Clear data bit mask          */
    tty.c_cflag |=  CS8;             /* 8 data bits                  */
    tty.c_cflag |=  CREAD | CLOCAL; /* Enable receiver, local mode  */
    tty.c_cflag &= ~CRTSCTS;         /* No hardware flow control     */

    /* ── Local Flags: Raw Mode ── */
    tty.c_lflag &= ~ICANON;          /* Disable canonical mode       */
    tty.c_lflag &= ~ECHO;            /* Disable echo                 */
    tty.c_lflag &= ~ECHOE;           /* Disable erasure echo         */
    tty.c_lflag &= ~ISIG;            /* Disable signal characters    */

    /* ── Input Flags ── */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);   /* No software flow control */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK |
                     ISTRIP | INLCR | IGNCR | ICRNL); /* Clean input flags */

    /* ── Output Flags ── */
    tty.c_oflag &= ~OPOST;           /* Raw output                   */
    tty.c_oflag &= ~ONLCR;          /* No newline conversion        */

    /* ── Read Behaviour ── */
    tty.c_cc[VMIN]  = 0;             /* Return immediately           */
    tty.c_cc[VTIME] = 0;             /* No inter-character timer     */

    /* Apply settings immediately */
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "ERROR: tcsetattr failed - %s\n", strerror(errno));
        return -1;
    }

    printf("UART configured: %d baud | 8N1 | no flow control\n", baud_rate);
    return 0;
}

/* ================================================================
 * uart_send()
 * Transmits a message over the UART interface.
 * Validates message length before sending.
 * Returns 0 on success, -1 on failure.
 * ================================================================ */
int uart_send(int fd, const char *message) {

    /* Validate message */
    if (message == NULL || strlen(message) == 0) {
        fprintf(stderr, "ERROR: Message is empty or NULL.\n");
        return -1;
    }

    if (strlen(message) > MAX_MSG_LEN) {
        fprintf(stderr, "ERROR: Message too long (max %d chars).\n", MAX_MSG_LEN);
        return -1;
    }

    /* Transmit */
    ssize_t bytes_written = write(fd, message, strlen(message));

    if (bytes_written < 0) {
        fprintf(stderr, "ERROR: Write failed - %s\n", strerror(errno));
        return -1;
    }

    /* Check for partial write */
    if ((size_t)bytes_written != strlen(message)) {
        fprintf(stderr, "WARNING: Partial write — sent %zd of %zu bytes\n",
                bytes_written, strlen(message));
    }

    printf("Sent (%zd bytes): %s", bytes_written, message);
    return 0;
}

/* ================================================================
 * uart_receive()
 * Waits for incoming data using select() with a timeout.
 * Non-blocking — returns after TIMEOUT_SEC seconds if no data.
 * Returns 0 on success, 1 on timeout, -1 on error.
 * ================================================================ */
int uart_receive(int fd) {
    fd_set read_fds;
    struct timeval timeout;
    char buffer[BUFFER_SIZE];

    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    timeout.tv_sec  = TIMEOUT_SEC;
    timeout.tv_usec = 0;

    printf("Waiting for incoming data (%d sec timeout)...\n", TIMEOUT_SEC);

    int activity = select(fd + 1, &read_fds, NULL, NULL, &timeout);

    if (activity < 0) {
        fprintf(stderr, "ERROR: select() failed - %s\n", strerror(errno));
        return -1;
    }

    if (activity == 0) {
        printf("Timeout: No data received within %d seconds.\n", TIMEOUT_SEC);
        return 1;
    }

    /* Data available — read it */
    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);

    if (bytes_read < 0) {
        fprintf(stderr, "ERROR: Read failed - %s\n", strerror(errno));
        return -1;
    }

    if (bytes_read == 0) {
        printf("WARNING: Connection closed by remote end.\n");
        return 1;
    }

    /* Null terminate and print */
    buffer[bytes_read] = '\0';
    printf("Received (%zd bytes): %s\n", bytes_read, buffer);
    return 0;
}

/* ================================================================
 * print_usage()
 * Prints usage instructions to stderr.
 * ================================================================ */
void print_usage(const char *prog_name) {
    fprintf(stderr, "\nUsage: %s [device] [baud_rate]\n", prog_name);
    fprintf(stderr, "  device     Serial port (default: %s)\n", DEFAULT_DEV);
    fprintf(stderr, "  baud_rate  Baud rate   (default: %d)\n", DEFAULT_BAUD);
    fprintf(stderr, "\nExamples:\n");
    fprintf(stderr, "  sudo %s\n", prog_name);
    fprintf(stderr, "  sudo %s /dev/ttyUSB0\n", prog_name);
    fprintf(stderr, "  sudo %s /dev/ttyUSB0 115200\n\n", prog_name);
}

/* ================================================================
 * main()
 * Entry point — parses arguments, runs UART send/receive flow.
 * ================================================================ */
int main(int argc, char *argv[]) {

    const char *device = DEFAULT_DEV;
    int baud_rate       = DEFAULT_BAUD;

    /* Parse optional arguments */
    if (argc > 1) {
        /* Handle help flag */
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }
        device = argv[1];
    }

    if (argc > 2) {
        baud_rate = atoi(argv[2]);
        if (baud_rate <= 0) {
            fprintf(stderr, "ERROR: Invalid baud rate '%s'\n", argv[2]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    printf("=== UART Communication Test ===\n");
    printf("Device: %s | Baud: %d\n\n", device, baud_rate);

    /* Step 1: Open UART device */
    int fd = open_uart(device);
    if (fd < 0) return EXIT_FAILURE;

    /* Step 2: Configure UART */
    if (configure_uart(fd, baud_rate) < 0) {
        close(fd);
        return EXIT_FAILURE;
    }

    /* Step 3: Send test message */
    const char *message = "Hello from UART - RISC-V ACT Challenge!\n";
    if (uart_send(fd, message) < 0) {
        close(fd);
        return EXIT_FAILURE;
    }

    /* Step 4: Receive response */
    uart_receive(fd);

    /* Step 5: Clean up */
    close(fd);
    printf("\nUART device closed. Done.\n");
    return EXIT_SUCCESS;
}
