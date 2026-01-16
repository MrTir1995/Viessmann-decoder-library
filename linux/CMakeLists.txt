cmake_minimum_required(VERSION 3.10)
project(ViessmannMultiProtocol VERSION 2.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Library source files
set(LIB_SOURCES
    src/Arduino.cpp
    src/LinuxSerial.cpp
    src/vbusdecoder.cpp
)

# Library headers
set(LIB_HEADERS
    include/Arduino.h
    include/LinuxSerial.h
    include/vbusdecoder.h
)

# Create static library
add_library(viessmann_static STATIC ${LIB_SOURCES})
set_target_properties(viessmann_static PROPERTIES OUTPUT_NAME viessmann)

# Create shared library
add_library(viessmann_shared SHARED ${LIB_SOURCES})
set_target_properties(viessmann_shared PROPERTIES OUTPUT_NAME viessmann)

# Example executable
add_executable(vbusdecoder_linux examples/vbusdecoder_linux.cpp)
target_link_libraries(vbusdecoder_linux viessmann_static)

# Installation rules
include(GNUInstallDirs)

# Install libraries
install(TARGETS viessmann_static viessmann_shared
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install headers
install(FILES ${LIB_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/viessmann
)

# Install example
install(TARGETS vbusdecoder_linux
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/viessmann.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/viessmann.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/viessmann.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Print summary
message(STATUS "Viessmann Multi-Protocol Library")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Library directory: ${CMAKE_INSTALL_FULL_LIBDIR}")
message(STATUS "  Include directory: ${CMAKE_INSTALL_FULL_INCLUDEDIR}/viessmann")
