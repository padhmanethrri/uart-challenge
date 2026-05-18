# Test Results

## Environment
- OS: Windows 11 with WSL (Ubuntu)
- Compiler: GCC
- Testing method: Virtual UART pair using socat

## Build Output

    gcc -Wall -Wextra -g -o uart uart.c

## Automated Test Run

    ===============================
     UART Automated Test Suite
    ===============================
    [TEST 1] Checking binary exists...
      PASS: ./uart found
    [TEST 2] Checking socat is installed...
      PASS: socat found
    [TEST 3] Checking --help flag...
      PASS: --help flag works
    [TEST 4] Checking invalid device path handling...
      PASS: Invalid device handled correctly
    [TEST 5] Checking invalid baud rate handling...
      PASS: Invalid baud rate handled correctly
    [TEST 6] Running virtual UART send/receive test...
      Virtual ports: /dev/pts/2 <-> /dev/pts/3
      PASS: Message transmitted and received successfully
      Received: Hello from UART - RISC-V ACT Challenge!
    ===============================
     Results: 6 passed | 0 failed
    ===============================
     All tests passed!

## Manual socat Test

    Terminal 1: socat running, ports /dev/pts/2 and /dev/pts/3 created
    Terminal 2: cat /dev/pts/3 listening
    Terminal 3: sudo ./uart /dev/pts/2 executed

    Program output:
    === UART Communication Test ===
    Device: /dev/pts/2 | Baud: 9600

    UART device opened: /dev/pts/2
    UART configured: 9600 baud | 8N1 | no flow control
    Sent (40 bytes): Hello from UART - RISC-V ACT Challenge!
    Waiting for incoming data (5 sec timeout)...
    Timeout: No data received within 5 seconds.
    UART device closed. Done.

    Receiver output (Terminal 2):
    Hello from UART - RISC-V ACT Challenge!
