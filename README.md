# UART Communication Program
### RISC-V ACT Framework – Coding Challenge Submission

## Overview
This program initializes and configures a UART serial interface on Linux using the termios API. It demonstrates hardware-level communication by transmitting a test message and receiving incoming data with a non-blocking timeout mechanism.

This is directly relevant to the RISC-V ACT mentorship project, where UART is used as the primary interface for communicating test results from M-mode firmware running on hardware boards such as the VisionFive 2 and Milk-V Jupiter.

---

## Features
- Configures UART parameters: baud rate (9600), data bits (8), parity (none), stop bits (1) — standard 8N1
- Transmits a test message over the UART interface
- Receives incoming data using select() with a 5-second timeout (non-blocking I/O)
- Prints received data to the console
- Gracefully handles errors: invalid device paths, permission issues, read/write failures
- Well-commented code explaining every implementation decision

---

## File Structure

uart-challenge/
├── uart.c       # Main UART program
├── Makefile     # Build configuration
└── README.md    # This file

---

## Requirements
- Linux or WSL (Windows Subsystem for Linux)
- GCC compiler
- Make

Install dependencies on Ubuntu/Debian:

sudo apt update
sudo apt install gcc make -y

---

## Build

make

Expected output:

gcc -Wall -Wextra -g -o uart uart.c

---

## Run

sudo ./uart [device]

Default device is /dev/ttyS0. You can specify any UART device:

sudo ./uart /dev/ttyUSB0

---

## Testing on WSL (Virtual UART using socat)
Since WSL does not expose real hardware COM ports, we use socat to create a virtual UART pair for end-to-end testing.

Install socat:

sudo apt install socat -y

Terminal 1 — Create virtual UART pair:

socat -d -d pty,raw,echo=0 pty,raw,echo=0

Note the two device paths printed (e.g. /dev/pts/2 and /dev/pts/3)

Terminal 2 — Listen for incoming data:

cat /dev/pts/3

Terminal 3 — Run the program:

sudo ./uart /dev/pts/2

---

## Expected Output

Terminal 3 (program):

Opening UART device: /dev/pts/2
UART device opened successfully!
UART configured: 9600 baud | 8N1
Sent (40 bytes): Hello from UART - RISC-V ACT Challenge!
Waiting for incoming data (5 sec timeout)...
Timeout: No data received within 5 seconds.
UART device closed. Done.

Terminal 2 (receiver):

Hello from UART - RISC-V ACT Challenge!

---

## UART Configuration Details

| Parameter    | Value        | Description               |
|--------------|--------------|---------------------------|
| Baud Rate    | 9600         | Bits per second           |
| Data Bits    | 8            | Bits per frame            |
| Parity       | None         | No parity checking        |
| Stop Bits    | 1            | Standard 8N1 format       |
| Flow Control | None         | No hardware/software flow |
| Read Mode    | Non-blocking | Uses select() with timeout|

---

## Error Handling
The program handles the following error cases:
- Invalid or missing device path
- Permission denied (suggests running with sudo)
- tcgetattr / tcsetattr configuration failures
- Write failures
- Read failures
- select() failures

---

## Relevance to RISC-V ACT Mentorship
In the RISC-V ACT hardware validation workflow, UART serves as the primary communication channel between the host machine and the target board running M-mode firmware. This program demonstrates the foundational Linux serial programming skills needed to:
- Read test results from the tohost interface via UART
- Debug trap and boot flow issues on VisionFive 2 / Milk-V Jupiter
- Automate ACT test execution and log capture

---

## Author
Padhmanethrri
Submission for RISC-V ACT Framework Enablement Mentorship – 10xEngineers
