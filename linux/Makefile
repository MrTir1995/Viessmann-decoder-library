# Makefile for Viessmann Multi-Protocol Library (Linux)

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++11
INCLUDES = -I./include
LDFLAGS = 

# Directories
SRC_DIR = src
INC_DIR = include
EXAMPLES_DIR = examples
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
LIB_DIR = $(BUILD_DIR)/lib

# Library files
LIB_NAME = libviessmann
LIB_STATIC = $(LIB_DIR)/$(LIB_NAME).a
LIB_SHARED = $(LIB_DIR)/$(LIB_NAME).so

# Source files
LIB_SOURCES = $(SRC_DIR)/Arduino.cpp \
              $(SRC_DIR)/LinuxSerial.cpp \
              $(SRC_DIR)/vbusdecoder.cpp

# Object files
LIB_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(LIB_SOURCES))

# Example executables
EXAMPLE_TARGETS = $(BIN_DIR)/vbusdecoder_linux

# Installation directories
PREFIX ?= /usr/local
INSTALL_LIB_DIR = $(PREFIX)/lib
INSTALL_INC_DIR = $(PREFIX)/include/viessmann
INSTALL_BIN_DIR = $(PREFIX)/bin

# Default target
all: directories $(LIB_STATIC) $(LIB_SHARED) examples

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)

# Compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -fPIC -c $< -o $@

# Build static library
$(LIB_STATIC): $(LIB_OBJECTS)
	@echo "Creating static library $@..."
	ar rcs $@ $^

# Build shared library
$(LIB_SHARED): $(LIB_OBJECTS)
	@echo "Creating shared library $@..."
	$(CXX) -shared -o $@ $^ $(LDFLAGS)

# Build examples
examples: $(EXAMPLE_TARGETS)

$(BIN_DIR)/vbusdecoder_linux: $(EXAMPLES_DIR)/vbusdecoder_linux.cpp $(LIB_STATIC)
	@echo "Building example: vbusdecoder_linux..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ -L$(LIB_DIR) -lviessmann

# Install library and headers
install: all
	@echo "Installing library to $(PREFIX)..."
	install -d $(INSTALL_LIB_DIR)
	install -m 644 $(LIB_STATIC) $(INSTALL_LIB_DIR)
	install -m 755 $(LIB_SHARED) $(INSTALL_LIB_DIR)
	install -d $(INSTALL_INC_DIR)
	install -m 644 $(INC_DIR)/*.h $(INSTALL_INC_DIR)
	install -d $(INSTALL_BIN_DIR)
	install -m 755 $(BIN_DIR)/vbusdecoder_linux $(INSTALL_BIN_DIR)
	@echo "Installation complete!"
	@echo "Library installed to: $(INSTALL_LIB_DIR)"
	@echo "Headers installed to: $(INSTALL_INC_DIR)"
	@echo "Example installed to: $(INSTALL_BIN_DIR)"
	@echo ""
	@echo "Run 'ldconfig' as root to update library cache:"
	@echo "  sudo ldconfig"

# Uninstall
uninstall:
	@echo "Uninstalling library from $(PREFIX)..."
	rm -f $(INSTALL_LIB_DIR)/$(LIB_NAME).a
	rm -f $(INSTALL_LIB_DIR)/$(LIB_NAME).so
	rm -rf $(INSTALL_INC_DIR)
	rm -f $(INSTALL_BIN_DIR)/vbusdecoder_linux
	@echo "Uninstallation complete!"

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)

# Clean and rebuild
rebuild: clean all

# Show help
help:
	@echo "Viessmann Multi-Protocol Library - Linux Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build library and examples (default)"
	@echo "  examples   - Build example applications"
	@echo "  install    - Install library, headers, and examples"
	@echo "  uninstall  - Remove installed files"
	@echo "  clean      - Remove build files"
	@echo "  rebuild    - Clean and rebuild everything"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Installation options:"
	@echo "  PREFIX=/path  - Set installation prefix (default: /usr/local)"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build everything"
	@echo "  make install            # Install to /usr/local"
	@echo "  make PREFIX=/opt install  # Install to /opt"
	@echo "  sudo make install       # Install with root privileges"

.PHONY: all directories examples install uninstall clean rebuild help
