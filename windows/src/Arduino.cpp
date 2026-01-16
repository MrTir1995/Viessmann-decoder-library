/*
 * Arduino compatibility layer for Windows
 * Implements Arduino-like timing functions
 */

#include "Arduino.h"

// Static variable for timing reference
static LARGE_INTEGER frequency;
static LARGE_INTEGER startCount;
static bool initialized = false;

// Initialize timing
static void initTiming() {
    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&startCount);
        initialized = true;
    }
}

// Returns milliseconds since program start
unsigned long millis() {
    initTiming();
    
    LARGE_INTEGER currentCount;
    QueryPerformanceCounter(&currentCount);
    
    // Calculate elapsed time in milliseconds
    LONGLONG elapsed = currentCount.QuadPart - startCount.QuadPart;
    return (unsigned long)((elapsed * 1000) / frequency.QuadPart);
}

// Returns microseconds since program start
unsigned long micros() {
    initTiming();
    
    LARGE_INTEGER currentCount;
    QueryPerformanceCounter(&currentCount);
    
    // Calculate elapsed time in microseconds
    LONGLONG elapsed = currentCount.QuadPart - startCount.QuadPart;
    return (unsigned long)((elapsed * 1000000) / frequency.QuadPart);
}

// Delay for specified milliseconds
void delay(unsigned long ms) {
    Sleep(ms);
}

// Delay for specified microseconds
void delayMicroseconds(unsigned int us) {
    // For short delays, use busy-wait for better accuracy
    if (us < 1000) {
        unsigned long start = micros();
        while ((micros() - start) < us) {
            // Busy wait
        }
    } else {
        // For longer delays, use Sleep (less CPU intensive)
        Sleep(us / 1000);
    }
}
