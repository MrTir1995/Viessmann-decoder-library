cmake_minimum_required(VERSION 3.10)
project(ViessmannMultiProtocol VERSION 2.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    # Add definitions for Windows
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    # MinGW or other compilers
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Library source files
set(LIB_SOURCES
    src/Arduino.cpp
    src/WindowsSerial.cpp
    src/vbusdecoder.cpp
)

# Library headers
set(LIB_HEADERS
    include/Arduino.h
    include/WindowsSerial.h
    include/vbusdecoder.h
)

# Create static library
add_library(viessmann_static STATIC ${LIB_SOURCES})
set_target_properties(viessmann_static PROPERTIES OUTPUT_NAME viessmann)

# Create shared library (DLL)
add_library(viessmann_shared SHARED ${LIB_SOURCES})
set_target_properties(viessmann_shared PROPERTIES OUTPUT_NAME viessmann)

# Example executable
add_executable(vbusdecoder_windows examples/vbusdecoder_windows.cpp)
target_link_libraries(vbusdecoder_windows viessmann_static)

# Installation rules
include(GNUInstallDirs)

# Install libraries
install(TARGETS viessmann_static viessmann_shared
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(FILES ${LIB_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/viessmann
)

# Install example
install(TARGETS vbusdecoder_windows
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Print summary
message(STATUS "Viessmann Multi-Protocol Library (Windows)")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Library directory: ${CMAKE_INSTALL_FULL_LIBDIR}")
message(STATUS "  Include directory: ${CMAKE_INSTALL_FULL_INCLUDEDIR}/viessmann")
