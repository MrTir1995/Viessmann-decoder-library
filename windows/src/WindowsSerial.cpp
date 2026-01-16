/*
 * Windows Serial Port implementation
 * Implements Arduino-like Serial interface for Windows
 */

#include "WindowsSerial.h"
#include <stdio.h>

WindowsSerial::WindowsSerial() : hSerial(INVALID_HANDLE_VALUE) {
    memset(&dcbSerialParams, 0, sizeof(dcbSerialParams));
    memset(&timeouts, 0, sizeof(timeouts));
}

WindowsSerial::~WindowsSerial() {
    end();
}

bool WindowsSerial::begin(const char* port, unsigned long baud, uint8_t config) {
    // Close port if already open
    if (hSerial != INVALID_HANDLE_VALUE) {
        end();
    }
    
    // Convert port name to Windows format (COM1 -> \\\\.\\COM1)
    char winPort[32];
    if (strncmp(port, "COM", 3) == 0 || strncmp(port, "com", 3) == 0) {
        snprintf(winPort, sizeof(winPort), "\\\\.\\%s", port);
    } else {
        strncpy(winPort, port, sizeof(winPort) - 1);
        winPort[sizeof(winPort) - 1] = '\0';
    }
    
    // Open serial port
    hSerial = CreateFileA(
        winPort,
        GENERIC_READ | GENERIC_WRITE,
        0,      // No sharing
        NULL,   // No security
        OPEN_EXISTING,
        0,      // Non-overlapped I/O
        NULL
    );
    
    if (hSerial == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening serial port %s: %lu\n", port, GetLastError());
        return false;
    }
    
    // Configure serial port
    if (!configure(baud, config)) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return false;
    }
    
    return true;
}

void WindowsSerial::end() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
    }
}

bool WindowsSerial::configure(unsigned long baud, uint8_t config) {
    // Get current DCB parameters
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        fprintf(stderr, "Error getting serial port state: %lu\n", GetLastError());
        return false;
    }
    
    // Set baud rate
    dcbSerialParams.BaudRate = baud;
    
    // Set data bits (always 8)
    dcbSerialParams.ByteSize = 8;
    
    // Set parity
    // Parity is in bits 2-3: 00=None, 10=Even, 11=Odd
    uint8_t parity = (config >> 2) & 0x03;
    if (parity == 0) {
        dcbSerialParams.Parity = NOPARITY;
    } else if (parity == 2) {
        dcbSerialParams.Parity = EVENPARITY;
    } else if (parity == 3) {
        dcbSerialParams.Parity = ODDPARITY;
    } else {
        dcbSerialParams.Parity = NOPARITY;
    }
    
    // Set stop bits
    // Stop bits are in bits 4-5: 00=1, 01=2
    uint8_t stopbits = (config >> 4) & 0x03;
    if (stopbits == 1) {
        dcbSerialParams.StopBits = TWOSTOPBITS;
    } else {
        dcbSerialParams.StopBits = ONESTOPBIT;
    }
    
    // Other settings
    dcbSerialParams.fBinary = TRUE;
    dcbSerialParams.fParity = (dcbSerialParams.Parity != NOPARITY) ? TRUE : FALSE;
    dcbSerialParams.fOutxCtsFlow = FALSE;
    dcbSerialParams.fOutxDsrFlow = FALSE;
    dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
    dcbSerialParams.fDsrSensitivity = FALSE;
    dcbSerialParams.fTXContinueOnXoff = FALSE;
    dcbSerialParams.fOutX = FALSE;
    dcbSerialParams.fInX = FALSE;
    dcbSerialParams.fErrorChar = FALSE;
    dcbSerialParams.fNull = FALSE;
    dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;
    dcbSerialParams.fAbortOnError = FALSE;
    
    // Apply DCB parameters
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        fprintf(stderr, "Error setting serial port state: %lu\n", GetLastError());
        return false;
    }
    
    // Set timeouts for non-blocking read
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        fprintf(stderr, "Error setting serial port timeouts: %lu\n", GetLastError());
        return false;
    }
    
    return true;
}

int WindowsSerial::available() {
    if (hSerial == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    COMSTAT status;
    DWORD errors;
    
    if (!ClearCommError(hSerial, &errors, &status)) {
        return 0;
    }
    
    return status.cbInQue;
}

int WindowsSerial::read() {
    if (hSerial == INVALID_HANDLE_VALUE) {
        return -1;
    }
    
    uint8_t byte;
    DWORD bytesRead = 0;
    
    if (!ReadFile(hSerial, &byte, 1, &bytesRead, NULL)) {
        return -1;
    }
    
    if (bytesRead == 0) {
        return -1;
    }
    
    return byte;
}

size_t WindowsSerial::write(uint8_t data) {
    return write(&data, 1);
}

size_t WindowsSerial::write(const uint8_t *buffer, size_t size) {
    if (hSerial == INVALID_HANDLE_VALUE || buffer == NULL) {
        return 0;
    }
    
    DWORD bytesWritten = 0;
    
    if (!WriteFile(hSerial, buffer, size, &bytesWritten, NULL)) {
        return 0;
    }
    
    return bytesWritten;
}

void WindowsSerial::flush() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        FlushFileBuffers(hSerial);
    }
}
