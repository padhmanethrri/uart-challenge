#!/bin/bash
# =============================================================
# test.sh - Automated UART Test Script
# RISC-V ACT Framework – Coding Challenge
# Author: Padhmanethrri
#
# This script creates a virtual UART pair using socat and
# automatically tests send/receive without any hardware.
# =============================================================

BINARY="./uart"
PASS=0
FAIL=0

echo "==============================="
echo " UART Automated Test Suite"
echo "==============================="
echo ""

# ── Check 1: Binary exists ──────────────────────────────────
echo "[TEST 1] Checking binary exists..."
if [ -f "$BINARY" ]; then
    echo "  PASS: $BINARY found"
    PASS=$((PASS + 1))
else
    echo "  FAIL: Binary not found. Run 'make' first."
    FAIL=$((FAIL + 1))
fi

# ── Check 2: socat installed ────────────────────────────────
echo "[TEST 2] Checking socat is installed..."
if command -v socat &> /dev/null; then
    echo "  PASS: socat found"
    PASS=$((PASS + 1))
else
    echo "  FAIL: socat not found. Run: sudo apt install socat -y"
    FAIL=$((FAIL + 1))
    echo ""
    echo "Cannot run virtual UART tests without socat. Exiting."
    exit 1
fi

# ── Check 3: Help flag works ────────────────────────────────
echo "[TEST 3] Checking --help flag..."
$BINARY --help > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "  PASS: --help flag works"
    PASS=$((PASS + 1))
else
    echo "  FAIL: --help flag returned error"
    FAIL=$((FAIL + 1))
fi

# ── Check 4: Invalid device path ────────────────────────────
echo "[TEST 4] Checking invalid device path handling..."
$BINARY /dev/invalid_device_xyz 2>&1 | grep -q "ERROR"
if [ $? -eq 0 ]; then
    echo "  PASS: Invalid device handled correctly"
    PASS=$((PASS + 1))
else
    echo "  FAIL: Invalid device not handled"
    FAIL=$((FAIL + 1))
fi

# ── Check 5: Invalid baud rate ──────────────────────────────
echo "[TEST 5] Checking invalid baud rate handling..."
$BINARY /dev/ttyS0 99999 2>&1 | grep -q "ERROR\|Unsupported"
if [ $? -eq 0 ]; then
    echo "  PASS: Invalid baud rate handled correctly"
    PASS=$((PASS + 1))
else
    echo "  FAIL: Invalid baud rate not handled"
    FAIL=$((FAIL + 1))
fi

# ── Check 6: Virtual UART send/receive ──────────────────────
echo "[TEST 6] Running virtual UART send/receive test..."

# Create virtual UART pair
socat -d -d pty,raw,echo=0 pty,raw,echo=0 > /tmp/socat_output 2>&1 &
SOCAT_PID=$!
sleep 1

# Extract port names from socat output
PTY1=$(grep "PTY is" /tmp/socat_output | head -1 | awk '{print $NF}')
PTY2=$(grep "PTY is" /tmp/socat_output | tail -1 | awk '{print $NF}')

if [ -z "$PTY1" ] || [ -z "$PTY2" ]; then
    echo "  FAIL: Could not create virtual UART pair"
    FAIL=$((FAIL + 1))
else
    echo "  Virtual ports: $PTY1 <-> $PTY2"

    # Listen on PTY2 in background
    cat $PTY2 > /tmp/uart_received.txt &
    CAT_PID=$!
    sleep 0.5

    # Send from PTY1
    sudo $BINARY $PTY1 > /tmp/uart_output.txt 2>&1
    sleep 1

    # Check received file
    if grep -q "Hello from UART" /tmp/uart_received.txt 2>/dev/null; then
        echo "  PASS: Message transmitted and received successfully"
        echo "  Received: $(cat /tmp/uart_received.txt)"
        PASS=$((PASS + 1))
    else
        echo "  FAIL: Message not received"
        FAIL=$((FAIL + 1))
    fi

    kill $CAT_PID 2>/dev/null
fi

kill $SOCAT_PID 2>/dev/null
rm -f /tmp/socat_output /tmp/uart_received.txt /tmp/uart_output.txt

# ── Summary ─────────────────────────────────────────────────
echo ""
echo "==============================="
echo " Results: $PASS passed | $FAIL failed"
echo "==============================="

if [ $FAIL -eq 0 ]; then
    echo " All tests passed!"
    exit 0
else
    echo " Some tests failed."
    exit 1
fi
