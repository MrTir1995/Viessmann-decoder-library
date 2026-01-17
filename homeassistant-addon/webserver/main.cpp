/*
 * Viessmann Multi-Protocol Library - Linux Web Server
 * 
 * This is a Linux-based web server for Home Assistant integration
 * that provides a web interface for monitoring and configuring 
 * Viessmann heating systems.
 * 
 * Based on the ESP32/ESP8266 webserver example but ported to Linux
 * using libmicrohttpd for HTTP server functionality.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <microhttpd.h>
#include <pthread.h>
#include <sys/stat.h>
#include "LinuxSerial.h"
#include "vbusdecoder.h"

// Configuration structure
struct Config {
    uint8_t protocol;      // 0=VBUS, 1=KW, 2=P300, 3=KM
    unsigned long baudRate;
    uint8_t serialConfig;  // SERIAL_8N1 or SERIAL_8E2
    const char* serialPort;
    uint16_t webPort;
};

// Global variables
volatile bool running = true;
LinuxSerial vbusSerial;
VBUSDecoder* vbus = nullptr;
Config config;
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;

// Signal handler
void signalHandler(int signum) {
    printf("\nShutting down...\n");
    running = false;
}

// Helper functions
const char* getProtocolName(uint8_t protocol) {
    switch(protocol) {
        case PROTOCOL_VBUS: return "VBUS (RESOL)";
        case PROTOCOL_KW: return "KW-Bus (VS1)";
        case PROTOCOL_P300: return "P300 (VS2/Optolink)";
        case PROTOCOL_KM: return "KM-Bus";
        default: return "Unknown";
    }
}

ProtocolType parseProtocol(const char* str) {
    if (strcasecmp(str, "vbus") == 0) return PROTOCOL_VBUS;
    if (strcasecmp(str, "kw") == 0) return PROTOCOL_KW;
    if (strcasecmp(str, "p300") == 0) return PROTOCOL_P300;
    if (strcasecmp(str, "km") == 0) return PROTOCOL_KM;
    return PROTOCOL_VBUS;
}

uint8_t parseSerialConfig(const char* str) {
    if (strcasecmp(str, "8N1") == 0) return SERIAL_8N1;
    if (strcasecmp(str, "8E2") == 0) return SERIAL_8E2;
    return SERIAL_8N1;
}

// Generate JSON data response
char* generateDataJSON() {
    static char json[4096];
    int offset = 0;
    int remaining = sizeof(json) - 1; // Reserve space for null terminator
    
    pthread_mutex_lock(&data_mutex);
    
    // Check if vbus is valid
    if (!vbus) {
        pthread_mutex_unlock(&data_mutex);
        snprintf(json, sizeof(json), "{\"error\":\"System not initialized\"}");
        return json;
    }
    
    #define JSON_APPEND(fmt, ...) do { \
        int written = snprintf(json + offset, remaining, fmt, ##__VA_ARGS__); \
        if (written < 0 || written >= remaining) { \
            /* Overflow detected - close JSON properly */ \
            if (offset > 0 && json[offset - 1] == ',') offset--; /* Remove trailing comma */ \
            int close_written = snprintf(json + offset, remaining, "}"); \
            if (close_written > 0 && close_written < remaining) { \
                offset += close_written; \
            } \
            pthread_mutex_unlock(&data_mutex); \
            json[sizeof(json) - 1] = '\0'; /* Ensure null termination */ \
            return json; \
        } \
        offset += written; \
        remaining -= written; \
    } while(0)
    
    JSON_APPEND("{");
    JSON_APPEND("\"ready\":%s,", vbus->isReady() ? "true" : "false");
    JSON_APPEND("\"status\":\"%s\",", vbus->getVbusStat() ? "OK" : "Error");
    JSON_APPEND("\"protocol\":%d,", config.protocol);
    
    // Temperatures
    JSON_APPEND("\"temperatures\":[");
    if (vbus->isReady()) {
        uint8_t tempNum = vbus->getTempNum();
        for (uint8_t i = 0; i < tempNum && i < 32; i++) {
            if (i > 0) JSON_APPEND(",");
            JSON_APPEND("%.1f", vbus->getTemp(i));
        }
    }
    JSON_APPEND("],");
    
    // Pumps
    JSON_APPEND("\"pumps\":[");
    if (vbus->isReady()) {
        uint8_t pumpNum = vbus->getPumpNum();
        for (uint8_t i = 0; i < pumpNum && i < 32; i++) {
            if (i > 0) JSON_APPEND(",");
            JSON_APPEND("%d", vbus->getPump(i));
        }
    }
    JSON_APPEND("],");
    
    // Relays
    JSON_APPEND("\"relays\":[");
    if (vbus->isReady()) {
        uint8_t relayNum = vbus->getRelayNum();
        for (uint8_t i = 0; i < relayNum && i < 32; i++) {
            if (i > 0) JSON_APPEND(",");
            JSON_APPEND("%s", vbus->getRelay(i) ? "true" : "false");
        }
    }
    JSON_APPEND("]");
    
    pthread_mutex_unlock(&data_mutex);
    
    JSON_APPEND("}");
    
    #undef JSON_APPEND
    
    return json;
}

// Generate HTML pages
const char* getDashboardHTML() {
    static const char* html = 
    "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
    "<title>Viessmann Decoder</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>"
    ":root{"
    "--primary-color:#03a9f4;"
    "--primary-dark:#0288d1;"
    "--accent-color:#ff9800;"
    "--card-background:#fff;"
    "--primary-background:#fafafa;"
    "--secondary-background:#e5e5e5;"
    "--primary-text:#212121;"
    "--secondary-text:#727272;"
    "--divider-color:#e0e0e0;"
    "--error-color:#f44336;"
    "--success-color:#4caf50;"
    "--warning-color:#ff9800;"
    "--disabled-text:#9e9e9e;"
    "--card-shadow:0 2px 2px 0 rgba(0,0,0,.14),0 1px 5px 0 rgba(0,0,0,.12),0 3px 1px -2px rgba(0,0,0,.2);"
    "}"
    "*{margin:0;padding:0;box-sizing:border-box;}"
    "body{font-family:'Roboto','Noto',sans-serif;background:var(--primary-background);color:var(--primary-text);-webkit-font-smoothing:antialiased;}"
    ".app-header{background:var(--primary-color);color:white;padding:0;box-shadow:0 2px 4px rgba(0,0,0,0.2);position:sticky;top:0;z-index:100;}"
    ".header-toolbar{display:flex;align-items:center;justify-content:space-between;padding:16px 24px;max-width:1200px;margin:0 auto;}"
    ".header-title{font-size:20px;font-weight:400;letter-spacing:0.02em;}"
    ".header-icon{display:inline-block;width:24px;height:24px;margin-right:12px;vertical-align:middle;}"
    ".view-container{max-width:1200px;margin:24px auto;padding:0 24px;}"
    ".status-bar{display:flex;gap:16px;margin-bottom:24px;flex-wrap:wrap;}"
    ".status-chip{background:var(--card-background);padding:12px 20px;border-radius:16px;box-shadow:var(--card-shadow);display:flex;align-items:center;gap:8px;font-size:14px;}"
    ".status-chip .label{color:var(--secondary-text);font-weight:500;}"
    ".status-chip .value{color:var(--primary-text);font-weight:500;}"
    ".status-indicator{width:8px;height:8px;border-radius:50%%;background:var(--disabled-text);}"
    ".status-indicator.ok{background:var(--success-color);}"
    ".status-indicator.error{background:var(--error-color);}"
    ".card{background:var(--card-background);border-radius:8px;box-shadow:var(--card-shadow);margin-bottom:24px;overflow:hidden;}"
    ".card-header{padding:16px 20px;border-bottom:1px solid var(--divider-color);}"
    ".card-title{font-size:16px;font-weight:500;color:var(--primary-text);}"
    ".card-content{padding:0;}"
    ".sensor-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(280px,1fr));gap:1px;background:var(--divider-color);}"
    ".sensor-item{background:var(--card-background);padding:20px;display:flex;flex-direction:column;gap:8px;}"
    ".sensor-label{font-size:14px;color:var(--secondary-text);font-weight:400;}"
    ".sensor-value{font-size:28px;font-weight:300;color:var(--primary-text);display:flex;align-items:baseline;gap:4px;}"
    ".sensor-unit{font-size:16px;color:var(--secondary-text);font-weight:400;}"
    ".sensor-icon{width:40px;height:40px;margin-bottom:8px;opacity:0.7;}"
    ".empty-state{padding:48px 20px;text-align:center;color:var(--secondary-text);}"
    ".empty-state-icon{font-size:64px;margin-bottom:16px;opacity:0.3;}"
    "@media(max-width:768px){"
    ".view-container{padding:0 16px;margin:16px auto;}"
    ".header-toolbar{padding:12px 16px;}"
    ".sensor-grid{grid-template-columns:1fr;}"
    "}"
    "</style>"
    "<script>"
    "function updateData(){"
    "fetch('/data').then(r=>r.json()).then(d=>{"
    "const statusDot=document.getElementById('statusDot');"
    "const statusText=document.getElementById('statusText');"
    "const protocolText=document.getElementById('protocol');"
    "statusDot.className='status-indicator '+(d.status==='OK'?'ok':'error');"
    "statusText.textContent=d.status;"
    "const protocols=['VBUS','KW-Bus','P300','KM-Bus'];"
    "protocolText.textContent=protocols[d.protocol]||'Unknown';"
    "const container=document.getElementById('sensorData');"
    "if(!d.ready||(!d.temperatures.length&&!d.pumps.length&&!d.relays.length)){"
    "container.innerHTML='<div class=\"empty-state\"><div class=\"empty-state-icon\">⏳</div><div>Waiting for data...</div></div>';"
    "return;"
    "}"
    "let html='';"
    "if(d.temperatures&&d.temperatures.length>0){"
    "d.temperatures.forEach((t,i)=>{"
    "html+='<div class=\"sensor-item\">';"
    "html+='<div class=\"sensor-label\">Temperature '+(i+1)+'</div>';"
    "html+='<div class=\"sensor-value\">'+t.toFixed(1)+'<span class=\"sensor-unit\">°C</span></div>';"
    "html+='</div>';"
    "});"
    "}"
    "if(d.pumps&&d.pumps.length>0){"
    "d.pumps.forEach((p,i)=>{"
    "html+='<div class=\"sensor-item\">';"
    "html+='<div class=\"sensor-label\">Pump '+(i+1)+' Power</div>';"
    "html+='<div class=\"sensor-value\">'+p+'<span class=\"sensor-unit\">%%</span></div>';"
    "html+='</div>';"
    "});"
    "}"
    "if(d.relays&&d.relays.length>0){"
    "d.relays.forEach((r,i)=>{"
    "html+='<div class=\"sensor-item\">';"
    "html+='<div class=\"sensor-label\">Relay '+(i+1)+'</div>';"
    "html+='<div class=\"sensor-value\" style=\"color:'+(r?'var(--success-color)':'var(--disabled-text)')+'\">'+(r?'ON':'OFF')+'</div>';"
    "html+='</div>';"
    "});"
    "}"
    "container.innerHTML=html;"
    "}).catch(err=>{"
    "console.error('Error fetching data:',err);"
    "document.getElementById('sensorData').innerHTML='<div class=\"empty-state\"><div class=\"empty-state-icon\">⚠️</div><div>Error loading data</div></div>';"
    "});"
    "}"
    "setInterval(updateData,2000);"
    "window.onload=updateData;"
    "</script>"
    "</head><body>"
    "<div class='app-header'>"
    "<div class='header-toolbar'>"
    "<div class='header-title'>"
    "<svg class='header-icon' viewBox='0 0 24 24' fill='currentColor'>"
    "<path d='M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2M12,4A8,8 0 0,1 20,12C20,14.4 19,16.5 17.3,18C15.9,16.7 14,16 12,16C10,16 8.2,16.7 6.7,18C5,16.5 4,14.4 4,12A8,8 0 0,1 12,4M14,5.89C13.62,5.9 13.26,6.15 13.1,6.54L11.81,9.77L11.71,10C11,10.13 10.41,10.6 10.14,11.26C9.73,12.29 10.23,13.45 11.26,13.86C12.29,14.27 13.45,13.77 13.86,12.74C14.12,12.08 14,11.32 13.57,10.76L13.67,10.5L14.96,7.29L14.97,7.26C15.17,6.75 14.92,6.17 14.41,5.96C14.28,5.91 14.15,5.89 14,5.89M10,6A1,1 0 0,0 9,7A1,1 0 0,0 10,8A1,1 0 0,0 11,7A1,1 0 0,0 10,6M7,9A1,1 0 0,0 6,10A1,1 0 0,0 7,11A1,1 0 0,0 8,10A1,1 0 0,0 7,9M17,9A1,1 0 0,0 16,10A1,1 0 0,0 17,11A1,1 0 0,0 18,10A1,1 0 0,0 17,9Z'/>"
    "</svg>"
    "Viessmann Decoder"
    "</div>"
    "</div>"
    "</div>"
    "<div class='view-container'>"
    "<div class='status-bar'>"
    "<div class='status-chip'>"
    "<div id='statusDot' class='status-indicator'></div>"
    "<span class='label'>Status:</span>"
    "<span id='statusText' class='value'>Checking...</span>"
    "</div>"
    "<div class='status-chip'>"
    "<span class='label'>Protocol:</span>"
    "<span id='protocol' class='value'>-</span>"
    "</div>"
    "</div>"
    "<div class='card'>"
    "<div class='card-header'>"
    "<div class='card-title'>Sensor Data</div>"
    "</div>"
    "<div class='card-content'>"
    "<div id='sensorData' class='sensor-grid'>"
    "<div class='empty-state'><div class='empty-state-icon'>⏳</div><div>Loading...</div></div>"
    "</div>"
    "</div>"
    "</div>"
    "</div>"
    "</body></html>";
    
    return html;
}

const char* getStatusHTML() {
    static char html[16384]; // Increased buffer size for safety
    
    // Check if vbus is valid
    if (!vbus) {
        snprintf(html, sizeof(html), 
                "<!DOCTYPE html><html><body><h1>Error: System not initialized</h1></body></html>");
        return html;
    }
    
    int written = snprintf(html, sizeof(html) - 1, // Reserve space for null terminator
    "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
    "<title>Viessmann Decoder - Status</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>"
    ":root{"
    "--primary-color:#03a9f4;"
    "--card-background:#fff;"
    "--primary-background:#fafafa;"
    "--primary-text:#212121;"
    "--secondary-text:#727272;"
    "--divider-color:#e0e0e0;"
    "--card-shadow:0 2px 2px 0 rgba(0,0,0,.14),0 1px 5px 0 rgba(0,0,0,.12),0 3px 1px -2px rgba(0,0,0,.2);"
    "}"
    "*{margin:0;padding:0;box-sizing:border-box;}"
    "body{font-family:'Roboto','Noto',sans-serif;background:var(--primary-background);color:var(--primary-text);}"
    ".app-header{background:var(--primary-color);color:white;padding:0;box-shadow:0 2px 4px rgba(0,0,0,0.2);}"
    ".header-toolbar{display:flex;align-items:center;padding:16px 24px;max-width:1200px;margin:0 auto;}"
    ".header-title{font-size:20px;font-weight:400;}"
    ".header-icon{width:24px;height:24px;margin-right:12px;vertical-align:middle;}"
    ".view-container{max-width:1200px;margin:24px auto;padding:0 24px;}"
    ".card{background:var(--card-background);border-radius:8px;box-shadow:var(--card-shadow);margin-bottom:24px;overflow:hidden;}"
    ".card-header{padding:16px 20px;border-bottom:1px solid var(--divider-color);}"
    ".card-title{font-size:16px;font-weight:500;}"
    ".info-table{width:100%%;}"
    ".info-row{display:flex;padding:16px 20px;border-bottom:1px solid var(--divider-color);}"
    ".info-row:last-child{border-bottom:none;}"
    ".info-label{flex:1;color:var(--secondary-text);font-size:14px;}"
    ".info-value{flex:1;color:var(--primary-text);font-size:14px;font-weight:500;text-align:right;}"
    "@media(max-width:768px){.view-container{padding:0 16px;margin:16px auto;}}"
    "</style>"
    "</head><body>"
    "<div class='app-header'>"
    "<div class='header-toolbar'>"
    "<div class='header-title'>"
    "<svg class='header-icon' viewBox='0 0 24 24' fill='currentColor'>"
    "<path d='M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2M12,4A8,8 0 0,1 20,12C20,14.4 19,16.5 17.3,18C15.9,16.7 14,16 12,16C10,16 8.2,16.7 6.7,18C5,16.5 4,14.4 4,12A8,8 0 0,1 12,4M14,5.89C13.62,5.9 13.26,6.15 13.1,6.54L11.81,9.77L11.71,10C11,10.13 10.41,10.6 10.14,11.26C9.73,12.29 10.23,13.45 11.26,13.86C12.29,14.27 13.45,13.77 13.86,12.74C14.12,12.08 14,11.32 13.57,10.76L13.67,10.5L14.96,7.29L14.97,7.26C15.17,6.75 14.92,6.17 14.41,5.96C14.28,5.91 14.15,5.89 14,5.89M10,6A1,1 0 0,0 9,7A1,1 0 0,0 10,8A1,1 0 0,0 11,7A1,1 0 0,0 10,6M7,9A1,1 0 0,0 6,10A1,1 0 0,0 7,11A1,1 0 0,0 8,10A1,1 0 0,0 7,9M17,9A1,1 0 0,0 16,10A1,1 0 0,0 17,11A1,1 0 0,0 18,10A1,1 0 0,0 17,9Z'/>"
    "</svg>"
    "System Status"
    "</div>"
    "</div>"
    "</div>"
    "<div class='view-container'>"
    "<div class='card'>"
    "<div class='card-header'><div class='card-title'>Current Configuration</div></div>"
    "<div class='info-table'>"
    "<div class='info-row'><div class='info-label'>Protocol</div><div class='info-value'>%s</div></div>"
    "<div class='info-row'><div class='info-label'>Baud Rate</div><div class='info-value'>%lu</div></div>"
    "<div class='info-row'><div class='info-label'>Serial Config</div><div class='info-value'>%s</div></div>"
    "<div class='info-row'><div class='info-label'>Serial Port</div><div class='info-value'>%s</div></div>"
    "<div class='info-row'><div class='info-label'>Web Port</div><div class='info-value'>%d</div></div>"
    "</div>"
    "</div>"
    "<div class='card'>"
    "<div class='card-header'><div class='card-title'>System Information</div></div>"
    "<div class='info-table'>"
    "<div class='info-row'><div class='info-label'>Platform</div><div class='info-value'>Linux</div></div>"
    "<div class='info-row'><div class='info-label'>Communication Status</div><div class='info-value'>%s</div></div>"
    "<div class='info-row'><div class='info-label'>Data Ready</div><div class='info-value'>%s</div></div>"
    "</div>"
    "</div>"
    "</div></body></html>",
    getProtocolName(config.protocol),
    config.baudRate,
    config.serialConfig == SERIAL_8N1 ? "8N1" : "8E2",
    config.serialPort,
    config.webPort,
    vbus->getVbusStat() ? "OK" : "Error",
    vbus->isReady() ? "Yes" : "No");
    
    // Ensure null termination and check for overflow
    html[sizeof(html) - 1] = '\0';
    if (written < 0 || written >= (int)(sizeof(html) - 1)) {
        fprintf(stderr, "Warning: HTML buffer overflow detected and handled\n");
        // Buffer is already null-terminated and truncated
    }
    
    return html;
}

// HTTP request handler
static MHD_Result handle_request(void *cls,
                                 struct MHD_Connection *connection,
                                 const char *url,
                                 const char *method,
                                 const char *version,
                                 const char *upload_data,
                                 size_t *upload_data_size,
                                 void **con_cls) {
    
    struct MHD_Response *response;
    MHD_Result ret;
    
    // Handle routes
    if (strcmp(url, "/") == 0) {
        const char* html = getDashboardHTML();
        response = MHD_create_response_from_buffer(strlen(html),
                                                   (void*)html,
                                                   MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/html");
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }
    else if (strcmp(url, "/data") == 0) {
        char* json = generateDataJSON();
        response = MHD_create_response_from_buffer(strlen(json),
                                                   (void*)json,
                                                   MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(response, "Content-Type", "application/json");
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }
    else if (strcmp(url, "/status") == 0) {
        const char* html = getStatusHTML();
        response = MHD_create_response_from_buffer(strlen(html),
                                                   (void*)html,
                                                   MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(response, "Content-Type", "text/html");
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    // 404 Not Found
    const char* not_found = "<html><body><h1>404 Not Found</h1></body></html>";
    response = MHD_create_response_from_buffer(strlen(not_found),
                                               (void*)not_found,
                                               MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

void printHelp(const char* progname) {
    printf("Viessmann Multi-Protocol Library - Web Server\n");
    printf("\nUsage: %s [options]\n", progname);
    printf("  -p <port>      Serial port (default: /dev/ttyUSB0)\n");
    printf("  -b <baud>      Baud rate (default: 9600)\n");
    printf("  -t <protocol>  Protocol type: vbus, kw, p300, km (default: vbus)\n");
    printf("  -c <config>    Serial config: 8N1, 8E2 (default: 8N1)\n");
    printf("  -w <port>      Web server port (default: 8099)\n");
    printf("  -h             Show this help\n");
}

int main(int argc, char* argv[]) {
    // Default configuration
    config.serialPort = "/dev/ttyUSB0";
    config.baudRate = 9600;
    config.protocol = PROTOCOL_VBUS;
    config.serialConfig = SERIAL_8N1;
    config.webPort = 8099;
    
    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "p:b:t:c:w:h")) != -1) {
        switch (opt) {
            case 'p':
                config.serialPort = optarg;
                break;
            case 'b':
                config.baudRate = atol(optarg);
                break;
            case 't':
                config.protocol = parseProtocol(optarg);
                break;
            case 'c':
                config.serialConfig = parseSerialConfig(optarg);
                break;
            case 'w':
                config.webPort = atoi(optarg);
                break;
            case 'h':
                printHelp(argv[0]);
                return 0;
            default:
                printHelp(argv[0]);
                return 1;
        }
    }
    
    // Setup signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    printf("Viessmann Decoder Web Server\n");
    printf("=============================\n");
    printf("Serial Port: %s\n", config.serialPort);
    printf("Baud Rate: %lu\n", config.baudRate);
    printf("Protocol: %s\n", getProtocolName(config.protocol));
    printf("Serial Config: %s\n", config.serialConfig == SERIAL_8N1 ? "8N1" : "8E2");
    printf("Web Port: %d\n", config.webPort);
    printf("\n");
    
    // Initialize serial port
    if (!vbusSerial.begin(config.serialPort, config.baudRate, config.serialConfig)) {
        fprintf(stderr, "Error: Failed to open serial port %s\n", config.serialPort);
        return 1;
    }
    printf("Serial port opened successfully\n");
    
    // Initialize decoder
    vbus = new VBUSDecoder(&vbusSerial);
    vbus->begin((ProtocolType)config.protocol);
    printf("Decoder initialized with protocol: %s\n", getProtocolName(config.protocol));
    
    // Start HTTP server
    struct MHD_Daemon *daemon;
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                             config.webPort,
                             NULL, NULL,
                             &handle_request, NULL,
                             MHD_OPTION_END);
    
    if (daemon == NULL) {
        fprintf(stderr, "Error: Failed to start HTTP server on port %d\n", config.webPort);
        delete vbus;
        return 1;
    }
    
    printf("Web server started on port %d\n", config.webPort);
    printf("Access the dashboard at: http://localhost:%d\n", config.webPort);
    printf("\nPress Ctrl+C to stop\n\n");
    
    // Main loop
    while (running) {
        pthread_mutex_lock(&data_mutex);
        vbus->loop();
        pthread_mutex_unlock(&data_mutex);
        usleep(10000);  // 10ms delay
    }
    
    // Cleanup
    printf("Stopping web server...\n");
    MHD_stop_daemon(daemon);
    delete vbus;
    
    printf("Shutdown complete\n");
    return 0;
}
