/*
 * Linux Serial Port implementation
 */

#include "LinuxSerial.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

LinuxSerial::LinuxSerial() : fd(-1) {
}

LinuxSerial::~LinuxSerial() {
    end();
}

bool LinuxSerial::begin(const char* port, unsigned long baud, uint8_t config) {
    // Open the serial port
    fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "Error opening serial port %s: %s\n", port, strerror(errno));
        return false;
    }
    
    // Save old terminal settings
    if (tcgetattr(fd, &oldtio) < 0) {
        fprintf(stderr, "Error getting terminal attributes: %s\n", strerror(errno));
        close(fd);
        fd = -1;
        return false;
    }
    
    // Configure the serial port
    if (!configure(baud, config)) {
        close(fd);
        fd = -1;
        return false;
    }
    
    return true;
}

void LinuxSerial::end() {
    if (fd >= 0) {
        tcsetattr(fd, TCSANOW, &oldtio);
        close(fd);
        fd = -1;
    }
}

int LinuxSerial::available() {
    if (fd < 0) return 0;
    
    int bytes_available;
    if (ioctl(fd, FIONREAD, &bytes_available) < 0) {
        return 0;
    }
    return bytes_available;
}

int LinuxSerial::read() {
    if (fd < 0) return -1;
    
    uint8_t data;
    ssize_t n = ::read(fd, &data, 1);
    if (n <= 0) return -1;
    
    return data;
}

size_t LinuxSerial::write(uint8_t data) {
    if (fd < 0) return 0;
    
    ssize_t n = ::write(fd, &data, 1);
    return (n > 0) ? n : 0;
}

size_t LinuxSerial::write(const uint8_t *buffer, size_t size) {
    if (fd < 0) return 0;
    
    ssize_t n = ::write(fd, buffer, size);
    return (n > 0) ? n : 0;
}

void LinuxSerial::flush() {
    if (fd >= 0) {
        tcdrain(fd);
    }
}

speed_t LinuxSerial::getBaudRate(unsigned long baud) {
    switch(baud) {
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        default: return B9600;
    }
}

bool LinuxSerial::configure(unsigned long baud, uint8_t config) {
    struct termios newtio;
    memset(&newtio, 0, sizeof(newtio));
    
    // Control modes
    newtio.c_cflag = CS8 | CLOCAL | CREAD;
    
    // Parse config byte
    // Bit 0-1: unused
    // Bit 2-3: parity (00=N, 10=E, 11=O)
    // Bit 4-5: stop bits (00=1, 01=2)
    
    // Parity
    if (config & 0x10) {
        newtio.c_cflag |= PARENB;
        if (config & 0x20) {
            newtio.c_cflag |= PARODD;  // Odd parity
        }
        // Even parity is default when PARENB is set
    }
    
    // Stop bits
    if (config & 0x08) {
        newtio.c_cflag |= CSTOPB;  // 2 stop bits
    }
    
    // Input modes
    newtio.c_iflag = IGNPAR;
    
    // Output modes
    newtio.c_oflag = 0;
    
    // Local modes
    newtio.c_lflag = 0;
    
    // Control characters
    newtio.c_cc[VTIME] = 0;  // Non-blocking
    newtio.c_cc[VMIN] = 0;
    
    // Set baud rate
    speed_t speed = getBaudRate(baud);
    cfsetispeed(&newtio, speed);
    cfsetospeed(&newtio, speed);
    
    // Flush and apply settings
    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &newtio) < 0) {
        fprintf(stderr, "Error setting terminal attributes: %s\n", strerror(errno));
        return false;
    }
    
    return true;
}
