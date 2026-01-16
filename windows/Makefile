# Makefile for Viessmann Multi-Protocol Library (Windows)
# For use with MinGW-w64 or MSYS2

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++11
INCLUDES = -I./include
LDFLAGS = -static-libgcc -static-libstdc++
LIBS = 

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
LIB_SHARED = $(LIB_DIR)/$(LIB_NAME).dll

# Source files
LIB_SOURCES = $(SRC_DIR)/Arduino.cpp \
              $(SRC_DIR)/WindowsSerial.cpp \
              $(SRC_DIR)/vbusdecoder.cpp

# Object files
LIB_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(LIB_SOURCES))

# Example executables
EXAMPLE_TARGETS = $(BIN_DIR)/vbusdecoder_windows.exe

# Installation directories (for MSYS2)
PREFIX ?= /usr/local
INSTALL_LIB_DIR = $(PREFIX)/lib
INSTALL_INC_DIR = $(PREFIX)/include/viessmann
INSTALL_BIN_DIR = $(PREFIX)/bin

# Default target
all: directories $(LIB_STATIC) $(LIB_SHARED) examples

# Create necessary directories
directories:
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	@if not exist "$(LIB_DIR)" mkdir "$(LIB_DIR)"

# Compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo Compiling $<...
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Build static library
$(LIB_STATIC): $(LIB_OBJECTS)
	@echo Creating static library $@...
	ar rcs $@ $^

# Build shared library (DLL)
$(LIB_SHARED): $(LIB_OBJECTS)
	@echo Creating shared library $@...
	$(CXX) -shared -o $@ $^ $(LDFLAGS) $(LIBS) -Wl,--out-implib,$(LIB_DIR)/$(LIB_NAME).dll.a

# Build examples
examples: $(EXAMPLE_TARGETS)

$(BIN_DIR)/vbusdecoder_windows.exe: $(EXAMPLES_DIR)/vbusdecoder_windows.cpp $(LIB_STATIC)
	@echo Building example: vbusdecoder_windows.exe...
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ -L$(LIB_DIR) -lviessmann $(LDFLAGS)

# Install library and headers (for MSYS2/MinGW environments)
install: all
	@echo Installing library to $(PREFIX)...
	@if not exist "$(INSTALL_LIB_DIR)" mkdir "$(INSTALL_LIB_DIR)"
	copy /Y "$(LIB_STATIC)" "$(INSTALL_LIB_DIR)"
	copy /Y "$(LIB_SHARED)" "$(INSTALL_LIB_DIR)"
	@if not exist "$(INSTALL_INC_DIR)" mkdir "$(INSTALL_INC_DIR)"
	copy /Y "$(INC_DIR)\*.h" "$(INSTALL_INC_DIR)"
	@if not exist "$(INSTALL_BIN_DIR)" mkdir "$(INSTALL_BIN_DIR)"
	copy /Y "$(BIN_DIR)\vbusdecoder_windows.exe" "$(INSTALL_BIN_DIR)"
	@echo Installation complete!
	@echo Library installed to: $(INSTALL_LIB_DIR)
	@echo Headers installed to: $(INSTALL_INC_DIR)
	@echo Example installed to: $(INSTALL_BIN_DIR)

# Uninstall
uninstall:
	@echo Uninstalling library from $(PREFIX)...
	@if exist "$(INSTALL_LIB_DIR)\$(LIB_NAME).a" del "$(INSTALL_LIB_DIR)\$(LIB_NAME).a"
	@if exist "$(INSTALL_LIB_DIR)\$(LIB_NAME).dll" del "$(INSTALL_LIB_DIR)\$(LIB_NAME).dll"
	@if exist "$(INSTALL_INC_DIR)" rmdir /S /Q "$(INSTALL_INC_DIR)"
	@if exist "$(INSTALL_BIN_DIR)\vbusdecoder_windows.exe" del "$(INSTALL_BIN_DIR)\vbusdecoder_windows.exe"
	@echo Uninstallation complete!

# Clean build files
clean:
	@echo Cleaning build files...
	@if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)"

# Clean and rebuild
rebuild: clean all

# Show help
help:
	@echo Viessmann Multi-Protocol Library - Windows Build System
	@echo.
	@echo Available targets:
	@echo   all        - Build library and examples (default)
	@echo   examples   - Build example applications
	@echo   install    - Install library, headers, and examples (MSYS2 only)
	@echo   uninstall  - Remove installed files (MSYS2 only)
	@echo   clean      - Remove build files
	@echo   rebuild    - Clean and rebuild everything
	@echo   help       - Show this help message
	@echo.
	@echo Build Requirements:
	@echo   - MinGW-w64 or MSYS2 with g++ compiler
	@echo   - Make utility
	@echo.
	@echo Examples:
	@echo   mingw32-make              # Build everything
	@echo   mingw32-make clean        # Clean build files
	@echo   mingw32-make rebuild      # Clean and rebuild

.PHONY: all directories examples install uninstall clean rebuild help
