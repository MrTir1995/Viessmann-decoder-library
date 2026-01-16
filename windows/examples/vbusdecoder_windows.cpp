/*
 * Viessmann Multi-Protocol Library - Windows Example
 * 
 * This example demonstrates how to use the library on Windows systems
 * to communicate with Viessmann heating systems.
 * 
 * Usage: vbusdecoder_windows.exe [options]
 *   -p <port>      Serial port (default: COM1)
 *   -b <baud>      Baud rate (default: 9600)
 *   -t <protocol>  Protocol type: vbus, kw, p300, km (default: vbus)
 *   -h             Show this help
 * 
 * Examples:
 *   vbusdecoder_windows.exe -p COM1 -b 9600 -t vbus
 *   vbusdecoder_windows.exe -p COM3 -b 4800 -t kw
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "WindowsSerial.h"
#include "vbusdecoder.h"

// Global variables for signal handling
volatile bool running = true;
WindowsSerial vbusSerial;

BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT) {
        printf("\nShutting down...\n");
        running = false;
        return TRUE;
    }
    return FALSE;
}

void printHelp(const char* progname) {
    printf("Viessmann Multi-Protocol Library - Windows Example\n");
    printf("\nUsage: %s [options]\n", progname);
    printf("  -p <port>      Serial port (default: COM1)\n");
    printf("  -b <baud>      Baud rate (default: 9600)\n");
    printf("  -t <protocol>  Protocol type: vbus, kw, p300, km (default: vbus)\n");
    printf("  -c <config>    Serial config: 8N1, 8E2 (default: 8N1)\n");
    printf("  -h             Show this help\n");
    printf("\nExamples:\n");
    printf("  %s -p COM1 -b 9600 -t vbus\n", progname);
    printf("  %s -p COM3 -b 4800 -t kw -c 8E2\n", progname);
    printf("\nProtocol Guide:\n");
    printf("  vbus  - RESOL VBUS Protocol (Vitosolic 200, DeltaSol) - 9600 baud, 8N1\n");
    printf("  kw    - KW-Bus (VS1) for Vitotronic 100/200/300 - 4800 baud, 8E2\n");
    printf("  p300  - P300/VS2 (Optolink) for newer Vitodens - 4800 baud, 8E2\n");
    printf("  km    - KM-Bus for remote controls and modules - varies\n");
    printf("\nFeatures:\n");
    printf("  - Automatic bus participant discovery (enabled by default)\n");
    printf("  - Real-time monitoring of all detected devices\n");
    printf("  - Manual device configuration supported via API\n");
}
    printf("  km    - KM-Bus for remote controls and modules - varies\n");
}

ProtocolType parseProtocol(const char* str) {
    if (_stricmp(str, "vbus") == 0) return PROTOCOL_VBUS;
    if (_stricmp(str, "kw") == 0) return PROTOCOL_KW;
    if (_stricmp(str, "p300") == 0) return PROTOCOL_P300;
    if (_stricmp(str, "km") == 0) return PROTOCOL_KM;
    return PROTOCOL_VBUS;  // Default
}

uint8_t parseConfig(const char* str) {
    if (_stricmp(str, "8N1") == 0) return SERIAL_8N1;
    if (_stricmp(str, "8E2") == 0) return SERIAL_8E2;
    return SERIAL_8N1;  // Default
}

int main(int argc, char* argv[]) {
    // Default parameters
    const char* port = "COM1";
    unsigned long baud = 9600;
    ProtocolType protocol = PROTOCOL_VBUS;
    uint8_t config = SERIAL_8N1;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = argv[++i];
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            baud = atol(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            protocol = parseProtocol(argv[++i]);
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            config = parseConfig(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0) {
            printHelp(argv[0]);
            return 0;
        } else {
            printf("Unknown option: %s\n\n", argv[i]);
            printHelp(argv[0]);
            return 1;
        }
    }
    
    // Set up console handler
    if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
        fprintf(stderr, "Warning: Could not set console handler\n");
    }
    
    // Open serial port
    printf("Opening serial port %s at %lu baud...\n", port, baud);
    if (!vbusSerial.begin(port, baud, config)) {
        fprintf(stderr, "Failed to open serial port %s\n", port);
        fprintf(stderr, "Make sure:\n");
        fprintf(stderr, "  1. The device is connected\n");
        fprintf(stderr, "  2. The port name is correct (e.g., COM1, COM3)\n");
        fprintf(stderr, "  3. No other application is using the port\n");
        fprintf(stderr, "  4. You have permission to access the port\n");
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
                
                // Display discovered bus participants
                uint8_t participantCount = vbus.getParticipantCount();
                if (participantCount > 0) {
                    printf("=== Discovered Bus Participants: %d ===\n", participantCount);
                    for (uint8_t i = 0; i < participantCount; i++) {
                        const BusParticipant* p = vbus.getParticipant(i);
                        if (p != nullptr) {
                            printf("  [%d] Address: 0x%04X, Name: %s\n", i + 1, p->address, p->name);
                            printf("      Channels: Temp=%d, Pump=%d, Relay=%d\n", 
                                   p->tempChannels, p->pumpChannels, p->relayChannels);
                            printf("      Status: %s, Last seen: %lu ms ago\n",
                                   p->autoDetected ? "Auto-detected" : "Manual",
                                   (unsigned long)(millis() - p->lastSeen));
                        }
                    }
                    printf("\n");
                }
                
                uint8_t tempNum = vbus.getTempNum();
                uint8_t relayNum = vbus.getRelayNum();
                uint8_t pumpNum = vbus.getPumpNum();
                
                // Current device data
                printf("=== Current Device (0x%04X) ===\n", vbus.getCurrentSourceAddress());
                
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
