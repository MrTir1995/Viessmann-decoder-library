# Windows Port Changelog

## Version 1.0.0 (2026-01-16)

### Initial Windows Support Release

This release adds full Windows support for the Viessmann Multi-Protocol Library, enabling native Windows applications without requiring Arduino hardware.

#### New Features

- **Native Windows Implementation**
  - Complete Windows serial port support using Win32 API
  - Arduino-compatible API layer for Windows
  - High-precision timing functions using QueryPerformanceCounter

- **Build System**
  - CMake support for flexible building with multiple compilers
  - Makefile for MinGW/MSYS2 users
  - Automated build scripts (build.bat, build_mingw.bat)
  - Support for Visual Studio and MinGW-w64 compilers

- **Example Application**
  - Command-line interface similar to Linux version
  - Support for all protocol types (VBUS, KW-Bus, P300, KM-Bus)
  - Windows-style COM port naming (COM1, COM3, etc.)
  - Comprehensive help and usage information

- **Documentation**
  - Complete Windows installation guide (README_WINDOWS.md)
  - Troubleshooting section for common Windows issues
  - Multiple build method instructions
  - Serial port configuration guide

#### Files Added

- `windows/include/Arduino.h` - Arduino compatibility layer for Windows
- `windows/include/WindowsSerial.h` - Windows serial port interface
- `windows/include/vbusdecoder.h` - Protocol decoder header
- `windows/src/Arduino.cpp` - Timing functions implementation
- `windows/src/WindowsSerial.cpp` - Serial port implementation
- `windows/src/vbusdecoder.cpp` - Protocol decoder implementation
- `windows/examples/vbusdecoder_windows.cpp` - Example application
- `windows/CMakeLists.txt` - CMake build configuration
- `windows/Makefile` - Makefile for MinGW
- `windows/build.bat` - Automated build script
- `windows/build_mingw.bat` - Quick MinGW build script
- `windows/README_WINDOWS.md` - Windows installation guide

#### Technical Details

- **Serial Port Implementation**: Uses Win32 CreateFile/ReadFile/WriteFile API
- **Timing**: QueryPerformanceCounter for microsecond-precision timing
- **Compatibility**: Windows 10, Windows 11, Windows Server 2016+
- **Compilers**: Visual Studio 2017+, MinGW-w64, MSYS2
- **Architecture**: Both 32-bit and 64-bit supported

#### Protocol Support

All protocols from the main library are fully supported on Windows:
- VBUS (RESOL Protocol 1.0)
- KW-Bus (VS1)
- P300 (VS2/Optolink)
- KM-Bus

#### Usage Example

```batch
vbusdecoder_windows.exe -p COM1 -b 9600 -t vbus
vbusdecoder_windows.exe -p COM3 -b 4800 -t kw -c 8E2
```

#### Known Limitations

- None at this time

#### Future Enhancements

- Potential GUI application using Windows Forms or WPF
- Windows Service support for background operation
- Installer package (MSI) for easier deployment

## Acknowledgments

Based on the Linux port architecture with Windows-specific adaptations for the Win32 API and Windows development toolchains.
