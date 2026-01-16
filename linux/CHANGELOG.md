# Changelog - Linux Support

## Version 2.0.0-linux (2026-01-16)

### Added - Linux Support

This release adds comprehensive Linux support for Debian-based systems (Debian, Ubuntu, Raspbian, Linux Mint, etc.).

#### New Features

- **Linux Port**: Complete port of the library to work natively on Linux systems
  - Arduino abstraction layer (Arduino.h compatibility)
  - Linux serial port implementation (LinuxSerial class)
  - POSIX timing functions (millis(), micros(), delay())
  
- **Build Systems**:
  - Makefile-based build system with multiple targets
  - CMake support as alternative build system
  - pkg-config integration for easy library integration
  
- **Example Applications**:
  - Command-line example program (vbusdecoder_linux)
  - Support for all protocols (VBUS, KW-Bus, P300, KM-Bus)
  - Configurable serial port, baud rate, and protocol
  - Real-time data monitoring
  
- **Installation Support**:
  - Automated installation script (install.sh)
  - System-wide library installation
  - Proper library path configuration
  - Serial port permission setup
  
- **Documentation**:
  - Comprehensive Linux installation guide (README_LINUX.md)
  - Usage examples and troubleshooting
  - Build instructions for both Make and CMake
  - Quick start guide

#### File Structure

```
linux/
├── include/              # Header files
│   ├── Arduino.h        # Arduino compatibility layer
│   ├── LinuxSerial.h    # Linux serial port implementation
│   └── vbusdecoder.h    # Library header
├── src/                  # Source files
│   ├── Arduino.cpp      # Arduino compatibility implementation
│   ├── LinuxSerial.cpp  # Linux serial port implementation
│   └── vbusdecoder.cpp  # Library implementation
├── examples/             # Example programs
│   └── vbusdecoder_linux.cpp  # Command-line example
├── Makefile             # Make build system
├── CMakeLists.txt       # CMake build system
├── viessmann.pc.in      # pkg-config template
├── install.sh           # Installation script
├── quickstart.sh        # Quick start script
├── README_LINUX.md      # Linux documentation
└── .gitignore           # Git ignore file for build artifacts
```

#### Installation

Quick installation:
```bash
cd linux
sudo ./install.sh
```

Manual installation:
```bash
cd linux
make
sudo make install
sudo ldconfig
```

#### Usage Examples

```bash
# VBUS protocol
vbusdecoder_linux -p /dev/ttyUSB0 -b 9600 -t vbus

# KW-Bus protocol
vbusdecoder_linux -p /dev/ttyUSB0 -b 4800 -t kw -c 8E2

# P300 protocol
vbusdecoder_linux -p /dev/ttyUSB0 -b 4800 -t p300 -c 8E2
```

### Technical Details

#### Arduino Abstraction Layer

The Linux port includes a complete Arduino abstraction layer that provides:
- `Stream` base class for serial communication
- `millis()` and `micros()` timing functions
- `delay()` and `delayMicroseconds()` delay functions
- Arduino-compatible type definitions

#### Serial Port Implementation

The LinuxSerial class provides:
- Standard POSIX termios-based serial communication
- Support for various baud rates (4800, 9600, 19200, 38400, 57600, 115200)
- Support for different serial configurations (8N1, 8E2, etc.)
- Non-blocking I/O for efficient data processing

#### Build System Features

Makefile targets:
- `make all` - Build everything
- `make examples` - Build only examples
- `make install` - Install library and examples
- `make uninstall` - Remove installed files
- `make clean` - Clean build files
- `make help` - Show help

CMake features:
- Standard CMake installation directories
- pkg-config file generation
- Support for custom installation prefix

### Compatibility

- **Tested on**: Ubuntu 20.04 LTS, Ubuntu 22.04 LTS, Debian 11, Debian 12
- **Compiler**: GCC 7.0+ or Clang 6.0+
- **C++ Standard**: C++11
- **Dependencies**: Standard POSIX libraries only

### Breaking Changes

None. The Arduino version remains fully compatible with existing code.

### Known Limitations

- Serial port hardware flow control not yet implemented
- Debian package (.deb) not yet available (planned for future release)

### Future Plans

- Debian/Ubuntu .deb package
- RPM package for Red Hat/Fedora
- Additional example programs
- Python bindings
- Web-based monitoring tool (standalone)
