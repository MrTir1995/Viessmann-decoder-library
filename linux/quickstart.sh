#!/bin/bash
#
# Quick Start Script for Viessmann Multi-Protocol Library (Linux)
# This script demonstrates basic usage without installation
#

echo "====================================================================="
echo "Viessmann Multi-Protocol Library - Linux Quick Start"
echo "====================================================================="
echo ""

# Check if we're in the linux directory
if [ ! -f "Makefile" ] || [ ! -d "src" ]; then
    echo "Error: Please run this script from the 'linux' directory"
    echo "Usage: cd linux && ./quickstart.sh"
    exit 1
fi

# Build the library
echo "Step 1: Building library..."
if ! make clean && make -j$(nproc); then
    echo ""
    echo "Error: Build failed. Please install build dependencies:"
    echo "  sudo apt-get install build-essential g++ make"
    exit 1
fi

echo ""
echo "====================================================================="
echo "Build successful!"
echo "====================================================================="
echo ""
echo "The following files have been built:"
echo "  - build/lib/libviessmann.a     (static library)"
echo "  - build/lib/libviessmann.so    (shared library)"
echo "  - build/bin/vbusdecoder_linux  (example program)"
echo ""
echo "====================================================================="
echo "Quick Test"
echo "====================================================================="
echo ""

# Show help
LD_LIBRARY_PATH=build/lib build/bin/vbusdecoder_linux -h

echo ""
echo "====================================================================="
echo "Next Steps"
echo "====================================================================="
echo ""
echo "1. Connect your Viessmann device to a serial port (e.g., /dev/ttyUSB0)"
echo ""
echo "2. Add your user to the 'dialout' group for serial port access:"
echo "   sudo usermod -a -G dialout \$USER"
echo "   (then log out and log back in)"
echo ""
echo "3. Run the example program:"
echo "   # For VBUS protocol (Vitosolic 200, DeltaSol):"
echo "   LD_LIBRARY_PATH=build/lib build/bin/vbusdecoder_linux -p /dev/ttyUSB0 -b 9600 -t vbus"
echo ""
echo "   # For KW-Bus protocol (Vitotronic 100/200/300):"
echo "   LD_LIBRARY_PATH=build/lib build/bin/vbusdecoder_linux -p /dev/ttyUSB0 -b 4800 -t kw -c 8E2"
echo ""
echo "4. To install system-wide (optional):"
echo "   sudo make install"
echo "   sudo ldconfig"
echo ""
echo "For detailed documentation, see README_LINUX.md"
echo ""
