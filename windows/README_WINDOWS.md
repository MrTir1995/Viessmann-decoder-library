# Viessmann Multi-Protocol Library - Windows Installation Guide

This guide provides instructions for installing and using the Viessmann Multi-Protocol Library on Windows operating systems.

## Table of Contents

- [System Requirements](#system-requirements)
- [Quick Installation](#quick-installation)
- [Building from Source](#building-from-source)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Uninstallation](#uninstallation)

## System Requirements

### Supported Systems
- Windows 10 (64-bit or 32-bit)
- Windows 11 (64-bit or 32-bit)
- Windows Server 2016 or later

### Hardware Requirements
- USB-to-Serial adapter or built-in serial port
- Connection to Viessmann heating system (VBUS, KW-Bus, P300/Optolink, or KM-Bus)

### Software Requirements

You need one of the following build toolchains:

**Option 1: MinGW-w64 (Recommended for beginners)**
- Download from: https://www.mingw-w64.org/
- Or install via MSYS2: https://www.msys2.org/

**Option 2: Visual Studio**
- Visual Studio 2017 or later with C++ support
- Download Community Edition (free) from: https://visualstudio.microsoft.com/

**Option 3: CMake (for advanced users)**
- CMake 3.10 or later
- Download from: https://cmake.org/download/

## Quick Installation

### Method 1: Using MinGW-w64 (Easiest)

1. **Install MinGW-w64 via MSYS2** (recommended):
   - Download MSYS2 installer from https://www.msys2.org/
   - Run the installer and follow the instructions
   - Open "MSYS2 MinGW 64-bit" terminal
   - Install build tools:
     ```bash
     pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake make
     ```

2. **Build the library**:
   - Open Command Prompt (cmd.exe)
   - Navigate to the windows directory:
     ```batch
     cd path\to\Viessmann-decoder-library\windows
     ```
   - Run the build script:
     ```batch
     build_mingw.bat
     ```

3. **Run the example**:
   ```batch
   build\bin\vbusdecoder_windows.exe -p COM1 -b 9600 -t vbus
   ```

### Method 2: Using Visual Studio

1. **Install Visual Studio** (if not already installed):
   - Download Visual Studio Community (free)
   - During installation, select "Desktop development with C++"

2. **Build using Visual Studio Developer Command Prompt**:
   - Open "x64 Native Tools Command Prompt for VS 2019" (or your version)
   - Navigate to the windows directory:
     ```batch
     cd path\to\Viessmann-decoder-library\windows
     ```
   - Build with CMake:
     ```batch
     mkdir build
     cd build
     cmake .. -G "NMake Makefiles"
     nmake
     ```

3. **Run the example**:
   ```batch
   bin\vbusdecoder_windows.exe -p COM1 -b 9600 -t vbus
   ```

### Method 3: Using CMake GUI

1. **Install CMake** from https://cmake.org/download/
2. **Run CMake GUI**:
   - Set source code location to: `path\to\Viessmann-decoder-library\windows`
   - Set build location to: `path\to\Viessmann-decoder-library\windows\build`
   - Click "Configure" and select your compiler
   - Click "Generate"
   - Click "Open Project" or build from command line

## Building from Source

### Using the Build Script (Recommended)

The `build.bat` script automatically detects your compiler and builds the library:

```batch
cd windows
build.bat
```

The script will:
1. Detect available compilers (Visual Studio or MinGW)
2. Configure the build with CMake
3. Build the library and examples
4. Display the location of output files

### Manual Build with MinGW Make

```batch
cd windows
mingw32-make
```

This creates:
- **Static library**: `build\lib\libviessmann.a`
- **Shared library (DLL)**: `build\lib\libviessmann.dll`
- **Example program**: `build\bin\vbusdecoder_windows.exe`

### Manual Build with CMake

```batch
cd windows
mkdir build
cd build

REM For MinGW
cmake .. -G "MinGW Makefiles"
mingw32-make

REM OR for Visual Studio (from VS Developer Command Prompt)
cmake .. -G "NMake Makefiles"
nmake

REM OR for Visual Studio IDE
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### Cleaning Build Files

```batch
cd windows
mingw32-make clean
```

Or simply delete the `build` directory:
```batch
rmdir /S /Q build
```

## Usage

### Finding Your COM Port

1. **Using Device Manager**:
   - Open Device Manager (Win+X, then select Device Manager)
   - Expand "Ports (COM & LPT)"
   - Look for your USB-to-Serial adapter (e.g., "USB Serial Port (COM3)")

2. **Using Command Prompt**:
   ```batch
   mode
   ```

### Running the Example Program

The library includes a command-line example program for testing:

```batch
REM Show help
build\bin\vbusdecoder_windows.exe -h

REM VBUS protocol (Vitosolic 200, DeltaSol)
build\bin\vbusdecoder_windows.exe -p COM1 -b 9600 -t vbus

REM KW-Bus protocol (Vitotronic 100/200/300)
build\bin\vbusdecoder_windows.exe -p COM3 -b 4800 -t kw -c 8E2

REM P300 protocol (newer Vitodens with Optolink)
build\bin\vbusdecoder_windows.exe -p COM1 -b 4800 -t p300 -c 8E2
```

### Command-Line Options

- `-p <port>` - Serial port device (default: COM1)
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
#include "WindowsSerial.h"
#include "vbusdecoder.h"

int main() {
    // Create serial port
    WindowsSerial serial;
    if (!serial.begin("COM1", 9600, SERIAL_8N1)) {
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

**With MinGW:**
```batch
g++ -o myprogram.exe myprogram.cpp -I.\include -L.\build\lib -lviessmann -static-libgcc -static-libstdc++
```

**With Visual Studio (from VS Developer Command Prompt):**
```batch
cl /EHsc myprogram.cpp /I.\include .\build\lib\viessmann.lib
```

## Troubleshooting

### Serial Port Access Denied

**Problem**: `Failed to open serial port: Access denied`

**Solutions**:
1. Close any other applications using the COM port (Arduino IDE, PuTTY, etc.)
2. Check if another program is monitoring the port
3. Try running your program as Administrator (right-click → Run as administrator)
4. Check Device Manager to ensure the device is working properly

### Serial Port Not Found

**Problem**: `Failed to open serial port: The system cannot find the file specified`

**Solutions**:
1. Verify the COM port number in Device Manager
2. Ensure the USB device is properly connected
3. Try unplugging and replugging the USB adapter
4. Check if the device driver is installed correctly

### No Data Received

**Problem**: Communication status shows "Error" or "Data ready: No"

**Possible causes**:
1. Wrong baud rate or serial configuration
2. Wrong protocol selected
3. Cable not properly connected
4. Device not powered or not transmitting

**Solutions**:
1. Verify protocol settings match your device (see Protocol Configuration Guide)
2. Check physical connections
3. Try different protocol settings
4. Check cable polarity and wiring

### Build Errors

**Problem**: Compilation fails with errors

**Solutions**:
1. Ensure you have a C++ compiler installed (MinGW or Visual Studio)
2. For MinGW: Make sure `g++` and `mingw32-make` are in your PATH
3. For Visual Studio: Use the "Developer Command Prompt" not regular Command Prompt
4. Try cleaning and rebuilding:
   ```batch
   mingw32-make clean
   mingw32-make
   ```

### DLL Not Found at Runtime

**Problem**: `The code execution cannot proceed because libviessmann.dll was not found`

**Solutions**:
1. Copy `build\lib\libviessmann.dll` to the same directory as your executable
2. Or add `build\lib` to your system PATH
3. Or use the static library instead (`.a` file)

### Windows Defender or Antivirus Blocks Execution

**Problem**: Windows Defender or antivirus software blocks the executable

**Solutions**:
1. Add an exception for the `build` directory in Windows Defender
2. The executable is safe - it's compiled from source code
3. You can rebuild from source yourself to verify

## Uninstallation

### Remove Build Files

```batch
cd windows
mingw32-make clean
```

Or manually delete the `build` directory:
```batch
rmdir /S /Q build
```

### Complete Removal

Simply delete the entire `windows` directory if you no longer need the Windows port.

## Additional Resources

- [Main README](../README.md) - General library documentation
- [PROTOCOLS.md](../PROTOCOLS.md) - Detailed protocol information
- [GitHub Repository](https://github.com/MrTir1995/Viessmann-decoder-library)

## License

See the main [LICENSE](../LICENSE) file for license information.

## Contributing

Contributions are welcome! Please submit issues and pull requests on GitHub.

## Support

For issues specific to Windows:
- Check this guide first
- Search existing GitHub issues
- Create a new issue with:
  - Windows version
  - Compiler used (MinGW/Visual Studio version)
  - Complete error messages
  - Steps to reproduce

## Serial Port Notes for Windows

### Common USB-to-Serial Chipsets

- **FTDI FT232** - Usually appears as "USB Serial Port (COMx)"
- **CH340/CH341** - Requires CH341 driver
- **CP2102** - Silicon Labs driver
- **PL2303** - Prolific driver

### Driver Installation

Most modern Windows versions automatically install drivers for common USB-to-Serial adapters. If not:

1. Check Device Manager for yellow exclamation marks
2. Download the specific driver from the manufacturer
3. Install the driver and restart if required
4. Verify the COM port appears in Device Manager

### Multiple Devices

If you have multiple USB-to-Serial adapters:
- Each will have a different COM port number
- Use Device Manager to identify which is which
- You can rename devices in Device Manager for easier identification
