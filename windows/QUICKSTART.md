# Windows Quick Start Guide

This is a simplified guide to get you started with the Windows version as quickly as possible.

## Prerequisites

You need a C++ compiler. Pick one:

### Option A: MinGW via MSYS2 (Easiest for beginners)
1. Download MSYS2: https://www.msys2.org/
2. Install MSYS2
3. Open "MSYS2 MinGW 64-bit" from Start Menu
4. Run: `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make`

### Option B: Visual Studio (If you already have it)
1. Download Visual Studio Community (free): https://visualstudio.microsoft.com/
2. Install with "Desktop development with C++" workload

## Build in 3 Steps

### For MinGW Users:

1. **Open Command Prompt** (Win+R, type `cmd`, press Enter)

2. **Navigate to windows folder**:
   ```batch
   cd C:\path\to\Viessmann-decoder-library\windows
   ```
   (Replace with your actual path)

3. **Build**:
   ```batch
   build_mingw.bat
   ```

### For Visual Studio Users:

1. **Open "x64 Native Tools Command Prompt for VS"** from Start Menu

2. **Navigate to windows folder**:
   ```batch
   cd C:\path\to\Viessmann-decoder-library\windows
   ```

3. **Build**:
   ```batch
   build.bat
   ```

## Run the Example

After building successfully:

```batch
build\bin\vbusdecoder_windows.exe -h
```

This shows help. To actually use it:

```batch
build\bin\vbusdecoder_windows.exe -p COM1 -b 9600 -t vbus
```

Replace `COM1` with your actual COM port (check Device Manager → Ports).

## Finding Your COM Port

1. Press Win+X
2. Select "Device Manager"
3. Expand "Ports (COM & LPT)"
4. Look for your USB device (e.g., "USB Serial Port (COM3)")
5. Remember the COM number (COM3 in this example)

## Common Issues

**"g++ is not recognized"** or **"mingw32-make is not recognized"**
- Make sure MinGW bin folder is in your PATH
- For MSYS2: Add `C:\msys64\mingw64\bin` to System PATH
- Or use MSYS2 terminal instead of Command Prompt

**"Permission denied" when opening COM port**
- Close other programs using the port (Arduino IDE, PuTTY, etc.)
- Try running as Administrator

**"The system cannot find the file specified"**
- Wrong COM port number
- Device not connected
- Check Device Manager

## What Protocol to Use?

| Your Device | Protocol | Baud | Config | Example |
|-------------|----------|------|--------|---------|
| Vitosolic 200, DeltaSol | vbus | 9600 | 8N1 | `-p COM1 -b 9600 -t vbus` |
| Vitotronic 100/200/300 | kw | 4800 | 8E2 | `-p COM1 -b 4800 -t kw -c 8E2` |
| Vitodens (with Optolink) | p300 | 4800 | 8E2 | `-p COM1 -b 4800 -t p300 -c 8E2` |

## Need More Help?

See the complete guide: [README_WINDOWS.md](README_WINDOWS.md)
