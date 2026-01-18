#!/usr/bin/with-contenv bashio

# Set log level from configuration (check both 'log_level' and 'log' for compatibility)
if bashio::config.has_value 'log_level'; then
    bashio::log.level "$(bashio::config 'log_level')"
elif bashio::config.has_value 'log'; then
    bashio::log.level "$(bashio::config 'log')"
fi

# Read configuration from Home Assistant
CONFIG_FILE="/data/options.json"
SERIAL_PORT=$(bashio::config 'serial_port')
BAUD_RATE=$(bashio::config 'baud_rate')
PROTOCOL=$(bashio::config 'protocol')
SERIAL_CONFIG=$(bashio::config 'serial_config')

# Set defaults for missing configuration
if ! bashio::var.has_value "${SERIAL_PORT}"; then
    bashio::log.warning "Serial port configuration is missing, using default /dev/ttyUSB0"
    SERIAL_PORT="/dev/ttyUSB0"
fi

if ! bashio::var.has_value "${BAUD_RATE}"; then
    bashio::log.warning "Baud rate configuration is missing, using default 9600"
    BAUD_RATE="9600"
fi

if ! bashio::var.has_value "${PROTOCOL}"; then
    bashio::log.warning "Protocol configuration is missing, using default vbus"
    PROTOCOL="vbus"
fi

if ! bashio::var.has_value "${SERIAL_CONFIG}"; then
    bashio::log.warning "Serial config configuration is missing, using default 8N1"
    SERIAL_CONFIG="8N1"
fi

bashio::log.info "Starting Viessmann Decoder Webserver..."
bashio::log.info "Serial Port: ${SERIAL_PORT}"
bashio::log.info "Baud Rate: ${BAUD_RATE}"
bashio::log.info "Protocol: ${PROTOCOL}"
bashio::log.info "Serial Config: ${SERIAL_CONFIG}"

# Wait for serial port to become available (retry up to 30 seconds)
RETRY_COUNT=0
MAX_RETRIES=30
while [ $RETRY_COUNT -lt $MAX_RETRIES ]; do
    if bashio::fs.file_exists "${SERIAL_PORT}"; then
        # Try to open the serial port
        if exec 3<>"${SERIAL_PORT}" 2>/dev/null; then
            exec 3>&-
            bashio::log.info "Serial port ${SERIAL_PORT} is available"
            break
        fi
    fi
    
    RETRY_COUNT=$((RETRY_COUNT + 1))
    if [ $RETRY_COUNT -eq 1 ]; then
        bashio::log.warning "Serial port ${SERIAL_PORT} not available, waiting..."
        bashio::log.info "Available serial ports:"
        ls -la /dev/tty* 2>/dev/null | head -20 || bashio::log.warning "Could not list serial ports"
    fi
    
    if [ $RETRY_COUNT -lt $MAX_RETRIES ]; then
        sleep 1
    fi
done

if [ $RETRY_COUNT -ge $MAX_RETRIES ]; then
    bashio::log.error "Serial port ${SERIAL_PORT} not available after ${MAX_RETRIES} seconds"
    bashio::log.error "Please check that the device is connected and properly configured"
    bashio::log.error "Available serial ports:"
    ls -la /dev/tty* 2>/dev/null || bashio::log.error "No serial ports found"
    bashio::exit.nok
fi

# Log startup completion
bashio::log.info "Starting webserver with validated configuration..."
bashio::log.info "Webserver will be available at http://localhost:8099"

# Run the webserver
exec viessmann_webserver \
    -p "${SERIAL_PORT}" \
    -b "${BAUD_RATE}" \
    -t "${PROTOCOL}" \
    -c "${SERIAL_CONFIG}" \
    -w 8099
