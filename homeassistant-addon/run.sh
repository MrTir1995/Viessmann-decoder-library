#!/usr/bin/with-contenv bashio

# Read configuration from Home Assistant
CONFIG_FILE="/data/options.json"
SERIAL_PORT=$(bashio::config 'serial_port')
BAUD_RATE=$(bashio::config 'baud_rate')
PROTOCOL=$(bashio::config 'protocol')
SERIAL_CONFIG=$(bashio::config 'serial_config')

bashio::log.info "Starting Viessmann Decoder Webserver..."
bashio::log.info "Serial Port: ${SERIAL_PORT}"
bashio::log.info "Baud Rate: ${BAUD_RATE}"
bashio::log.info "Protocol: ${PROTOCOL}"
bashio::log.info "Serial Config: ${SERIAL_CONFIG}"

# Check if serial port exists
if [ ! -e "${SERIAL_PORT}" ]; then
    bashio::log.error "Serial port ${SERIAL_PORT} not found!"
    bashio::log.error "Available serial ports:"
    ls -la /dev/tty* 2>/dev/null || bashio::log.error "No serial ports found"
    exit 1
fi

# Run the webserver
exec viessmann_webserver \
    -p "${SERIAL_PORT}" \
    -b "${BAUD_RATE}" \
    -t "${PROTOCOL}" \
    -c "${SERIAL_CONFIG}" \
    -w 8099
