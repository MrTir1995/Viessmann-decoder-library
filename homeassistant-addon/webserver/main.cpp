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
    
    pthread_mutex_lock(&data_mutex);
    
    offset += snprintf(json + offset, sizeof(json) - offset, "{");
    offset += snprintf(json + offset, sizeof(json) - offset, 
                      "\"ready\":%s,", vbus->isReady() ? "true" : "false");
    offset += snprintf(json + offset, sizeof(json) - offset,
                      "\"status\":\"%s\",", vbus->getVbusStat() ? "OK" : "Error");
    offset += snprintf(json + offset, sizeof(json) - offset,
                      "\"protocol\":%d,", config.protocol);
    
    // Temperatures
    offset += snprintf(json + offset, sizeof(json) - offset, "\"temperatures\":[");
    if (vbus->isReady()) {
        uint8_t tempNum = vbus->getTempNum();
        for (uint8_t i = 0; i < tempNum && i < 32; i++) {
            if (i > 0) offset += snprintf(json + offset, sizeof(json) - offset, ",");
            offset += snprintf(json + offset, sizeof(json) - offset, "%.1f", vbus->getTemp(i));
        }
    }
    offset += snprintf(json + offset, sizeof(json) - offset, "],");
    
    // Pumps
    offset += snprintf(json + offset, sizeof(json) - offset, "\"pumps\":[");
    if (vbus->isReady()) {
        uint8_t pumpNum = vbus->getPumpNum();
        for (uint8_t i = 0; i < pumpNum && i < 32; i++) {
            if (i > 0) offset += snprintf(json + offset, sizeof(json) - offset, ",");
            offset += snprintf(json + offset, sizeof(json) - offset, "%d", vbus->getPump(i));
        }
    }
    offset += snprintf(json + offset, sizeof(json) - offset, "],");
    
    // Relays
    offset += snprintf(json + offset, sizeof(json) - offset, "\"relays\":[");
    if (vbus->isReady()) {
        uint8_t relayNum = vbus->getRelayNum();
        for (uint8_t i = 0; i < relayNum && i < 32; i++) {
            if (i > 0) offset += snprintf(json + offset, sizeof(json) - offset, ",");
            offset += snprintf(json + offset, sizeof(json) - offset, "%s", 
                             vbus->getRelay(i) ? "true" : "false");
        }
    }
    offset += snprintf(json + offset, sizeof(json) - offset, "]");
    
    pthread_mutex_unlock(&data_mutex);
    
    offset += snprintf(json + offset, sizeof(json) - offset, "}");
    
    return json;
}

// Generate HTML pages
const char* getDashboardHTML() {
    static const char* html = 
    "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
    "<title>Viessmann Decoder - Dashboard</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>"
    "body{font-family:Arial,sans-serif;margin:0;padding:0;background:#f5f5f5;}"
    "header{background:#0066cc;color:white;padding:20px;text-align:center;}"
    ".container{max-width:1200px;margin:20px auto;padding:20px;}"
    ".card{background:white;border-radius:8px;padding:20px;margin-bottom:20px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}"
    ".card h2{margin-top:0;color:#333;}"
    ".nav{display:flex;justify-content:center;gap:20px;padding:20px;}"
    ".nav a{text-decoration:none;color:white;background:#0066cc;padding:10px 20px;border-radius:5px;}"
    ".nav a:hover{background:#0052a3;}"
    ".status{font-size:18px;margin:10px 0;}"
    ".status.ok{color:green;}"
    ".status.error{color:red;}"
    "table{width:100%;border-collapse:collapse;}"
    "th,td{text-align:left;padding:10px;border-bottom:1px solid #ddd;}"
    "th{background:#f0f0f0;}"
    "</style>"
    "<script>"
    "function updateData(){"
    "fetch('/data').then(r=>r.json()).then(d=>{"
    "document.getElementById('status').textContent=d.status;"
    "document.getElementById('status').className='status '+(d.status==='OK'?'ok':'error');"
    "let protocols=['VBUS','KW-Bus','P300','KM-Bus'];"
    "document.getElementById('protocol').textContent=protocols[d.protocol];"
    "let tbody=document.getElementById('data');"
    "tbody.innerHTML='';"
    "if(d.temperatures.length>0){"
    "d.temperatures.forEach((t,i)=>{"
    "tbody.innerHTML+='<tr><td>Temperature '+(i+1)+'</td><td>'+t.toFixed(1)+' °C</td></tr>';"
    "});"
    "}"
    "if(d.pumps.length>0){"
    "d.pumps.forEach((p,i)=>{"
    "tbody.innerHTML+='<tr><td>Pump '+(i+1)+'</td><td>'+p+' %</td></tr>';"
    "});"
    "}"
    "if(d.relays.length>0){"
    "d.relays.forEach((r,i)=>{"
    "tbody.innerHTML+='<tr><td>Relay '+(i+1)+'</td><td>'+(r?'ON':'OFF')+'</td></tr>';"
    "});"
    "}"
    "if(!d.ready){tbody.innerHTML='<tr><td colspan=\"2\">Waiting for data...</td></tr>';}"
    "});"
    "}"
    "setInterval(updateData,2000);"
    "window.onload=updateData;"
    "</script>"
    "</head><body>"
    "<header><h1>Viessmann Decoder Dashboard</h1></header>"
    "<div class='nav'>"
    "<a href='/'>Dashboard</a>"
    "<a href='/status'>Status</a>"
    "</div>"
    "<div class='container'>"
    "<div class='card'>"
    "<h2>System Status</h2>"
    "<p>Communication Status: <span id='status' class='status'>Checking...</span></p>"
    "<p>Active Protocol: <span id='protocol'>-</span></p>"
    "</div>"
    "<div class='card'>"
    "<h2>Live Data</h2>"
    "<table><thead><tr><th>Parameter</th><th>Value</th></tr></thead>"
    "<tbody id='data'><tr><td colspan='2'>Loading...</td></tr></tbody></table>"
    "</div>"
    "</div></body></html>";
    
    return html;
}

const char* getStatusHTML() {
    static char html[8192];
    snprintf(html, sizeof(html),
    "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
    "<title>Viessmann Decoder - Status</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>"
    "body{font-family:Arial,sans-serif;margin:0;padding:0;background:#f5f5f5;}"
    "header{background:#0066cc;color:white;padding:20px;text-align:center;}"
    ".container{max-width:800px;margin:20px auto;padding:20px;}"
    ".card{background:white;border-radius:8px;padding:20px;margin-bottom:20px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}"
    ".card h2{margin-top:0;color:#333;}"
    ".nav{display:flex;justify-content:center;gap:20px;padding:20px;}"
    ".nav a{text-decoration:none;color:white;background:#0066cc;padding:10px 20px;border-radius:5px;}"
    ".nav a:hover{background:#0052a3;}"
    "table{width:100%%;;border-collapse:collapse;}"
    "th,td{text-align:left;padding:10px;border-bottom:1px solid #ddd;}"
    "th{background:#f0f0f0;font-weight:bold;}"
    "</style>"
    "</head><body>"
    "<header><h1>System Status</h1></header>"
    "<div class='nav'>"
    "<a href='/'>Dashboard</a>"
    "<a href='/status'>Status</a>"
    "</div>"
    "<div class='container'>"
    "<div class='card'>"
    "<h2>Current Configuration</h2>"
    "<table>"
    "<tr><th>Parameter</th><th>Value</th></tr>"
    "<tr><td>Protocol</td><td>%s</td></tr>"
    "<tr><td>Baud Rate</td><td>%lu</td></tr>"
    "<tr><td>Serial Config</td><td>%s</td></tr>"
    "<tr><td>Serial Port</td><td>%s</td></tr>"
    "<tr><td>Web Port</td><td>%d</td></tr>"
    "</table>"
    "</div>"
    "<div class='card'>"
    "<h2>System Information</h2>"
    "<table>"
    "<tr><th>Parameter</th><th>Value</th></tr>"
    "<tr><td>Platform</td><td>Linux</td></tr>"
    "<tr><td>Communication Status</td><td>%s</td></tr>"
    "<tr><td>Data Ready</td><td>%s</td></tr>"
    "</table>"
    "</div>"
    "</div></body></html>",
    getProtocolName(config.protocol),
    config.baudRate,
    config.serialConfig == SERIAL_8N1 ? "8N1" : "8E2",
    config.serialPort,
    config.webPort,
    vbus->getVbusStat() ? "OK" : "Error",
    vbus->isReady() ? "Yes" : "No");
    
    return html;
}

// HTTP request handler
static int handle_request(void *cls,
                         struct MHD_Connection *connection,
                         const char *url,
                         const char *method,
                         const char *version,
                         const char *upload_data,
                         size_t *upload_data_size,
                         void **con_cls) {
    
    struct MHD_Response *response;
    int ret;
    
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
