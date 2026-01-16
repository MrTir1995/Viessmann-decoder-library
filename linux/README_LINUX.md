# Viessmann Multi-Protocol Library - Linux Installation Guide

This guide provides instructions for installing and using the Viessmann Multi-Protocol Library on Linux Debian and its derivatives (Ubuntu, Raspbian, Linux Mint, etc.).

## Table of Contents

- [System Requirements](#system-requirements)
- [Quick Installation](#quick-installation)
- [Manual Installation](#manual-installation)
- [Building from Source](#building-from-source)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Uninstallation](#uninstallation)

## System Requirements

### Supported Systems
- Debian 10 (Buster) or later
- Ubuntu 18.04 LTS or later
- Raspbian Buster or later
- Linux Mint 19 or later
- Other Debian-based distributions

### Hardware Requirements
- USB-to-Serial adapter or built-in serial port
- Connection to Viessmann heating system (VBUS, KW-Bus, P300/Optolink, or KM-Bus)

### Software Requirements
The following packages are needed for building and using the library:
- `build-essential` - C++ compiler and basic build tools
- `g++` - GNU C++ compiler
- `make` - Build automation tool
- `cmake` (optional) - Alternative build system
- `pkg-config` (optional) - Library configuration tool

## Quick Installation

The easiest way to install the library is using the provided installation script:

```bash
cd linux
sudo ./install.sh
```

The script will:
1. Install required dependencies
2. Build the library
3. Install the library, headers, and examples
4. Configure serial port permissions
5. Update the library cache

After installation, log out and log back in for group permissions to take effect.

## Manual Installation

### Step 1: Install Dependencies

On Debian/Ubuntu systems:
```bash
sudo apt-get update
sudo apt-get install build-essential g++ make cmake pkg-config
```

### Step 2: Build the Library

You can use either Make or CMake:

#### Option A: Using Make (Recommended)

```bash
cd linux
make
sudo make install
sudo ldconfig
```

#### Option B: Using CMake

```bash
cd linux
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
```

### Step 3: Configure Serial Port Permissions

To use serial ports without root privileges:

```bash
sudo usermod -a -G dialout $USER
```

Log out and log back in for the changes to take effect.

## Building from Source

### Build Options

#### Make Build System

```bash
# Build everything (default)
make

# Build only the library
make $(LIB_STATIC) $(LIB_SHARED)

# Build only examples
make examples

# Clean build files
make clean

# Install with custom prefix
make PREFIX=/opt install
```

#### CMake Build System

```bash
mkdir build && cd build

# Default installation to /usr/local
cmake ..

# Install to custom location
cmake -DCMAKE_INSTALL_PREFIX=/opt ..

# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..

make
sudo make install
```

### Build Output

After building, you'll find:
- **Static library**: `build/lib/libviessmann.a`
- **Shared library**: `build/lib/libviessmann.so`
- **Example program**: `build/bin/vbusdecoder_linux`

## Usage

### Running the Example Program

The library includes a command-line example program:

```bash
# Show help
vbusdecoder_linux -h

# VBUS protocol (Vitosolic 200, DeltaSol)
vbusdecoder_linux -p /dev/ttyUSB0 -b 9600 -t vbus

# KW-Bus protocol (Vitotronic 100/200/300)
vbusdecoder_linux -p /dev/ttyUSB0 -b 4800 -t kw -c 8E2

# P300 protocol (newer Vitodens with Optolink)
vbusdecoder_linux -p /dev/ttyUSB0 -b 4800 -t p300 -c 8E2
```

### Command-Line Options

- `-p <port>` - Serial port device (default: /dev/ttyUSB0)
- `-b <baud>` - Baud rate (default: 9600)
- `-t <protocol>` - Protocol type: vbus, kw, p300, km (default: vbus)
- `-c <config>` - Serial configuration: 8N1, 8E2 (default: 8N1)
- `-h` - Display help message

### Protocol Configuration Guide

| Device Type | Protocol | Baud Rate | Config |
|-------------|----------|-----------|--------|
| Vitosolic 200, DeltaSol BX/MX | `vbus` | 9600 | 8N1 |
| Vitotronic 100/200/300 (KW-Bus) | `kw` | 4800 | 8E2 |
| Vitodens, Vitocrossal (Optolink) | `p300` | 4800 | 8E2 |
| Remote controls, expansion modules | `km` | varies | varies |

### Using the Library in Your Own Code

#### Example C++ Program

```cpp
#include <stdio.h>
#include "LinuxSerial.h"
#include "vbusdecoder.h"

int main() {
    // Create serial port
    LinuxSerial serial;
    if (!serial.begin("/dev/ttyUSB0", 9600, SERIAL_8N1)) {
        fprintf(stderr, "Failed to open serial port\n");
        return 1;
    }
    
    // Create decoder
    VBUSDecoder vbus(&serial);
    vbus.begin(PROTOCOL_VBUS);
    
    // Main loop
    while (true) {
        vbus.loop();
        
        if (vbus.isReady()) {
            printf("Temperature 1: %.1f°C\n", vbus.getTemp(0));
        }
        
        delay(1000);
    }
    
    return 0;
}
```

#### Compiling Your Program

```bash
# Using installed library
g++ -o myprogram myprogram.cpp -lviessmann

# Using pkg-config
g++ -o myprogram myprogram.cpp $(pkg-config --cflags --libs viessmann)
```

## Troubleshooting

### Serial Port Permission Denied

**Problem**: `Error opening serial port: Permission denied`

**Solution**: Add your user to the dialout group:
```bash
sudo usermod -a -G dialout $USER
```
Log out and log back in for changes to take effect.

### Serial Port Not Found

**Problem**: `Error opening serial port: No such file or directory`

**Solutions**:
1. Check if device is connected: `ls -l /dev/ttyUSB*` or `ls -l /dev/ttyACM*`
2. Check USB connection with: `dmesg | tail`
3. Verify the correct port name

### No Data Received

**Problem**: Communication status shows "Error" or "Data ready: No"

**Possible causes**:
1. Wrong baud rate or serial configuration
2. Wrong protocol selected
3. Cable not properly connected
4. Device not powered or not transmitting

**Solutions**:
1. Verify protocol settings match your device
2. Check physical connections
3. Try different protocol settings
4. Check cable polarity and wiring

### Build Errors

**Problem**: Compilation fails with missing headers or libraries

**Solution**: Install build dependencies:
```bash
sudo apt-get install build-essential g++ make cmake pkg-config
```

### Library Not Found at Runtime

**Problem**: `error while loading shared libraries: libviessmann.so`

**Solution**: Update library cache:
```bash
sudo ldconfig
```

Or add library path to LD_LIBRARY_PATH:
```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

## Uninstallation

### Using Make

```bash
cd linux
sudo make uninstall
```

### Using CMake

```bash
cd linux/build
sudo make uninstall
```

### Manual Uninstallation

```bash
sudo rm /usr/local/lib/libviessmann.*
sudo rm -rf /usr/local/include/viessmann
sudo rm /usr/local/bin/vbusdecoder_linux
sudo ldconfig
```

## Additional Resources

- [Main README](../README.md) - General library documentation
- [PROTOCOLS.md](../PROTOCOLS.md) - Detailed protocol information
- [GitHub Repository](https://github.com/MrTir1995/Viessmann-decoder-library)

## License

See the main [LICENSE](../LICENSE) file for license information.

## Contributing

Contributions are welcome! Please submit issues and pull requests on GitHub.
