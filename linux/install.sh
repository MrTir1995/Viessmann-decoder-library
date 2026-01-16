#!/bin/bash
#
# Installation script for Viessmann Multi-Protocol Library on Debian/Ubuntu systems
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}Error: This script must be run as root (use sudo)${NC}"
    exit 1
fi

echo -e "${GREEN}Viessmann Multi-Protocol Library - Installation Script${NC}"
echo "==========================================================="
echo ""

# Detect OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
else
    OS=$(uname -s)
    VER=$(uname -r)
fi

echo "Detected OS: $OS $VER"
echo ""

# Check for Debian-based system
if [[ ! "$OS" =~ (Debian|Ubuntu|Raspbian|Linux Mint) ]]; then
    echo -e "${YELLOW}Warning: This script is designed for Debian-based systems.${NC}"
    echo "Your system may not be supported, but we'll try anyway."
    read -p "Continue? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Install build dependencies
echo -e "${GREEN}Step 1: Installing build dependencies...${NC}"
apt-get update
apt-get install -y build-essential g++ make cmake pkg-config

echo ""
echo -e "${GREEN}Step 2: Building library...${NC}"

# Choose build system
echo "Select build system:"
echo "  1) Make (default)"
echo "  2) CMake"
read -p "Choice [1]: " choice
choice=${choice:-1}

if [ "$choice" = "2" ]; then
    echo "Building with CMake..."
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    echo ""
    echo -e "${GREEN}Step 3: Installing library...${NC}"
    make install
    cd ..
else
    echo "Building with Make..."
    make -j$(nproc)
    echo ""
    echo -e "${GREEN}Step 3: Installing library...${NC}"
    make install
fi

# Update library cache
echo ""
echo -e "${GREEN}Step 4: Updating library cache...${NC}"
ldconfig

# Set up user permissions for serial port access
echo ""
echo -e "${GREEN}Step 5: Setting up serial port permissions...${NC}"
echo "To use serial ports without root privileges, users must be in the 'dialout' group."
read -p "Add current user to 'dialout' group? (Y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Nn]$ ]]; then
    SUDO_USER=${SUDO_USER:-$USER}
    if [ -n "$SUDO_USER" ] && [ "$SUDO_USER" != "root" ]; then
        usermod -a -G dialout $SUDO_USER
        echo "User '$SUDO_USER' added to 'dialout' group."
        echo -e "${YELLOW}Note: Log out and log back in for group changes to take effect.${NC}"
    else
        echo -e "${YELLOW}Warning: Could not determine non-root user. Please add your user manually:${NC}"
        echo "  sudo usermod -a -G dialout YOUR_USERNAME"
    fi
fi

echo ""
echo -e "${GREEN}Installation complete!${NC}"
echo ""
echo "Installed components:"
echo "  - Library: /usr/local/lib/libviessmann.{a,so}"
echo "  - Headers: /usr/local/include/viessmann/"
echo "  - Example: /usr/local/bin/vbusdecoder_linux"
echo ""
echo "To test the installation, run:"
echo "  vbusdecoder_linux -h"
echo ""
echo "To use with a Viessmann device:"
echo "  vbusdecoder_linux -p /dev/ttyUSB0 -b 9600 -t vbus"
echo ""
echo -e "${YELLOW}Important: If you were added to the 'dialout' group, log out and log back in.${NC}"
