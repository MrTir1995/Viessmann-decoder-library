/*
 * Viessmann Multi-Protocol Library - Linux Example
 * 
 * This example demonstrates how to use the library on Linux systems
 * to communicate with Viessmann heating systems.
 * 
 * Usage: ./vbusdecoder_linux [options]
 *   -p <port>      Serial port (default: /dev/ttyUSB0)
 *   -b <baud>      Baud rate (default: 9600)
 *   -t <protocol>  Protocol type: vbus, kw, p300, km (default: vbus)
 *   -h             Show this help
 * 
 * Examples:
 *   ./vbusdecoder_linux -p /dev/ttyUSB0 -b 9600 -t vbus
 *   ./vbusdecoder_linux -p /dev/ttyUSB1 -b 4800 -t kw
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "LinuxSerial.h"
#include "vbusdecoder.h"

// Global variables for signal handling
volatile bool running = true;
LinuxSerial vbusSerial;

void signalHandler(int signum) {
    printf("\nShutting down...\n");
    running = false;
}

void printHelp(const char* progname) {
    printf("Viessmann Multi-Protocol Library - Linux Example\n");
    printf("\nUsage: %s [options]\n", progname);
    printf("  -p <port>      Serial port (default: /dev/ttyUSB0)\n");
    printf("  -b <baud>      Baud rate (default: 9600)\n");
    printf("  -t <protocol>  Protocol type: vbus, kw, p300, km (default: vbus)\n");
    printf("  -c <config>    Serial config: 8N1, 8E2 (default: 8N1)\n");
    printf("  -h             Show this help\n");
    printf("\nExamples:\n");
    printf("  %s -p /dev/ttyUSB0 -b 9600 -t vbus\n", progname);
    printf("  %s -p /dev/ttyUSB1 -b 4800 -t kw -c 8E2\n", progname);
    printf("\nProtocol Guide:\n");
    printf("  vbus  - RESOL VBUS Protocol (Vitosolic 200, DeltaSol) - 9600 baud, 8N1\n");
    printf("  kw    - KW-Bus (VS1) for Vitotronic 100/200/300 - 4800 baud, 8E2\n");
    printf("  p300  - P300/VS2 (Optolink) for newer Vitodens - 4800 baud, 8E2\n");
    printf("  km    - KM-Bus for remote controls and modules - varies\n");
}

ProtocolType parseProtocol(const char* str) {
    if (strcasecmp(str, "vbus") == 0) return PROTOCOL_VBUS;
    if (strcasecmp(str, "kw") == 0) return PROTOCOL_KW;
    if (strcasecmp(str, "p300") == 0) return PROTOCOL_P300;
    if (strcasecmp(str, "km") == 0) return PROTOCOL_KM;
    return PROTOCOL_VBUS;  // Default
}

uint8_t parseConfig(const char* str) {
    if (strcasecmp(str, "8N1") == 0) return SERIAL_8N1;
    if (strcasecmp(str, "8E2") == 0) return SERIAL_8E2;
    return SERIAL_8N1;  // Default
}

int main(int argc, char* argv[]) {
    // Default parameters
    const char* port = "/dev/ttyUSB0";
    unsigned long baud = 9600;
    ProtocolType protocol = PROTOCOL_VBUS;
    uint8_t config = SERIAL_8N1;
    
    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "p:b:t:c:h")) != -1) {
        switch (opt) {
            case 'p':
                port = optarg;
                break;
            case 'b':
                baud = atol(optarg);
                break;
            case 't':
                protocol = parseProtocol(optarg);
                break;
            case 'c':
                config = parseConfig(optarg);
                break;
            case 'h':
                printHelp(argv[0]);
                return 0;
            default:
                printHelp(argv[0]);
                return 1;
        }
    }
    
    // Set up signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Open serial port
    printf("Opening serial port %s at %lu baud...\n", port, baud);
    if (!vbusSerial.begin(port, baud, config)) {
        fprintf(stderr, "Failed to open serial port %s\n", port);
        fprintf(stderr, "Make sure:\n");
        fprintf(stderr, "  1. The device is connected\n");
        fprintf(stderr, "  2. You have permission to access the port (add user to 'dialout' group)\n");
        fprintf(stderr, "  3. The port path is correct\n");
        return 1;
    }
    printf("Serial port opened successfully.\n");
    
    // Initialize decoder
    VBUSDecoder vbus(&vbusSerial);
    vbus.begin(protocol);
    
    // Display active protocol
    printf("\nActive Protocol: ");
    switch(vbus.getProtocol()) {
        case PROTOCOL_VBUS:
            printf("VBUS (RESOL)\n");
            break;
        case PROTOCOL_KW:
            printf("KW-Bus (VS1)\n");
            break;
        case PROTOCOL_P300:
            printf("P300 (VS2/Optolink)\n");
            break;
        case PROTOCOL_KM:
            printf("KM-Bus\n");
            break;
    }
    printf("\nWaiting for data from Viessmann device...\n");
    printf("Press Ctrl+C to exit.\n\n");
    
    unsigned long lastMillis = millis();
    bool firstData = true;
    
    // Main loop
    while (running) {
        vbus.loop();
        
        // Display data every 5 seconds
        if ((millis() - lastMillis) > 5000) {
            const char* vbusStat = vbus.getVbusStat() ? "Ok" : "Error";
            printf("Communication status: %s\n", vbusStat);
            
            const char* vbusReady = vbus.isReady() ? "Yes" : "No";
            printf("Data ready: %s\n", vbusReady);
            
            if (vbus.isReady()) {
                if (firstData) {
                    printf("\n--- First data received! ---\n\n");
                    firstData = false;
                }
                
                uint8_t tempNum = vbus.getTempNum();
                uint8_t relayNum = vbus.getRelayNum();
                uint8_t pumpNum = vbus.getPumpNum();
                
                // Temperature sensors
                if (tempNum > 0) {
                    printf("Temperature sensors [%d]: ", tempNum);
                    for (uint8_t i = 0; i < tempNum; i++) {
                        printf("%.1f°C", vbus.getTemp(i));
                        if (i < tempNum - 1) printf(", ");
                    }
                    printf("\n");
                }
                
                // Pump power
                if (pumpNum > 0) {
                    printf("Pump power [%d]: ", pumpNum);
                    for (uint8_t i = 0; i < pumpNum; i++) {
                        printf("%d%%", vbus.getPump(i));
                        if (i < pumpNum - 1) printf(", ");
                    }
                    printf("\n");
                }
                
                // Relay states
                if (relayNum > 0) {
                    printf("Relay status [%d]: ", relayNum);
                    for (uint8_t i = 0; i < relayNum; i++) {
                        printf("%s", vbus.getRelay(i) ? "ON" : "OFF");
                        if (i < relayNum - 1) printf(", ");
                    }
                    printf("\n");
                }
                
                // Extended information
                uint16_t errorMask = vbus.getErrorMask();
                if (errorMask != 0) {
                    printf("Error Mask: 0x%04X\n", errorMask);
                }
                
                uint16_t systemTime = vbus.getSystemTime();
                if (systemTime > 0) {
                    printf("System Time: %d minutes\n", systemTime);
                }
                
                uint32_t opHours0 = vbus.getOperatingHours(0);
                uint32_t opHours1 = vbus.getOperatingHours(1);
                if (opHours0 > 0 || opHours1 > 0) {
                    printf("Operating Hours: [1] %lu h, [2] %lu h\n", 
                           (unsigned long)opHours0, (unsigned long)opHours1);
                }
                
                uint16_t heatQty = vbus.getHeatQuantity();
                if (heatQty > 0) {
                    printf("Heat Quantity: %d Wh\n", heatQty);
                }
                
                uint8_t sysVariant = vbus.getSystemVariant();
                if (sysVariant > 0) {
                    printf("System Variant: %d\n", sysVariant);
                }
                
                printf("\n");
            }
            
            lastMillis = millis();
        }
        
        // Small delay to prevent busy waiting
        delay(10);
    }
    
    // Clean up
    vbusSerial.end();
    printf("Closed serial port.\n");
    
    return 0;
}
