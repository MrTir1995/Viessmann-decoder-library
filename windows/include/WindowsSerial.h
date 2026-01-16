/*
 * Windows Serial Port implementation
 * Provides Arduino-like Serial interface for Windows serial ports
 */

#pragma once
#ifndef WINDOWS_SERIAL_H
#define WINDOWS_SERIAL_H

#include "Arduino.h"

class WindowsSerial : public Stream {
public:
    WindowsSerial();
    ~WindowsSerial();
    
    // Open serial port with baud rate
    bool begin(const char* port, unsigned long baud, uint8_t config = SERIAL_8N1);
    void end();
    
    // Stream interface implementation
    int available() override;
    int read() override;
    size_t write(uint8_t data) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    void flush() override;
    
    // Additional methods
    bool isOpen() const { return hSerial != INVALID_HANDLE_VALUE; }
    
private:
    HANDLE hSerial;
    DCB dcbSerialParams;
    COMMTIMEOUTS timeouts;
    
    bool configure(unsigned long baud, uint8_t config);
};

#endif // WINDOWS_SERIAL_H
