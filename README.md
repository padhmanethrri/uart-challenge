# UART Communication Program
### RISC-V ACT Framework Enablement – Coding Challenge Submission

## Overview
This program opens and configures a UART serial interface on Linux using the termios API. It sends a test message over UART and listens for a response using select() with a non-blocking timeout. Error handling covers common real-world failures like missing devices, permission issues, invalid baud rates, and partial writes.

UART is the primary communication channel in RISC-V hardware validation workflows — used to read ACT test results from M-mode firmware running on boards like the VisionFive 2 and Milk-V Jupiter. This program covers the core Linux serial programming skills needed for that work.

---

## Files

| File | Description |
|------|-------------|
| uart.c | Main UART program |
| Makefile | Build configuration |
| test.sh | Automated test suite (6 tests) |
| RESULTS.md | Test results and program output |
| README.md | This file |

---

## How the Code Works

The program is split into clean, single-purpose functions:

- open_uart() opens the device file with validation on path length and null checks. Returns specific error messages for common failures like device not found or permission denied
- get_baud_constant() converts integer baud rate to termios speed_t constant and rejects unsupported values with a helpful message
- configure_uart() applies 8N1 settings using termios and disables echo, canonical mode, and flow control for raw serial communication
- uart_send() validates the message before writing and detects partial writes
- uart_receive() uses select() to wait up to 5 seconds for incoming data without blocking and handles timeout, empty reads, and errors separately
- main() parses optional device and baud rate arguments, supports --help flag, and runs the full send/receive flow

---

## UART Configuration

| Parameter | Value |
|-----------|-------|
| Baud Rate | 9600 (configurable) |
| Data Bits | 8 |
| Parity | None |
| Stop Bits | 1 |
| Flow Control | None |
| Read Mode | Non-blocking via select() |

---

## Edge Cases Handled

- NULL or empty device path
- Device path too long
- Device not found
- Permission denied — suggests running with sudo
- Unsupported baud rate — lists valid options
- tcgetattr and tcsetattr failures
- NULL or empty message
- Message exceeding maximum length
- Partial write detection
- select() failure
- Empty read and remote end closed
- Read failure

---

## Requirements

- Linux or WSL (Windows Subsystem for Linux)
- GCC
- Make
- socat (for testing only)

Install everything:

    sudo apt update
    sudo apt install gcc make socat -y

---

## Build

    make

---

## Run

    sudo ./uart                          # default /dev/ttyS0 at 9600 baud
    sudo ./uart /dev/ttyUSB0             # custom device
    sudo ./uart /dev/ttyUSB0 115200      # custom device and baud rate
    sudo ./uart --help                   # show usage

---

## Automated Tests

    ./test.sh

See RESULTS.md for full test output.

---

## Manual Testing on WSL

WSL does not expose real hardware COM ports so socat is used to create a virtual UART pair for end to end testing.

Terminal 1 — create virtual pair:

    socat -d -d pty,raw,echo=0 pty,raw,echo=0

Note the two port names printed (e.g. /dev/pts/2 and /dev/pts/3)

Terminal 2 — listen on one end:

    cat /dev/pts/3

Terminal 3 — run the program:

    sudo ./uart /dev/pts/2

See RESULTS.md for full output from this test.

---

## Relevance to RISC-V ACT Mentorship

In the RISC-V ACT validation workflow UART is the interface between the host machine and the target board running bare-metal M-mode firmware. Getting this right is a direct prerequisite for reading PASS/FAIL results from the tohost interface, debugging trap and boot flow issues on VisionFive 2 and Milk-V Jupiter, and automating ACT test runs to capture logs for validation reports.

---

## Author
Padhmanethrri
Coding challenge submission for RISC-V ACT Framework Enablement Mentorship — 10xEngineers
