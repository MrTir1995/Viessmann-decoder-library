#!/usr/bin/with-contenv bashio

# Read configuration from Home Assistant
CONFIG_FILE="/data/options.json"
SERIAL_PORT=$(bashio::config 'serial_port')
BAUD_RATE=$(bashio::config 'baud_rate')
PROTOCOL=$(bashio::config 'protocol')
SERIAL_CONFIG=$(bashio::config 'serial_config')

# Validate configuration values
if ! bashio::var.has_value "${SERIAL_PORT}"; then
    bashio::log.fatal "Serial port configuration is missing!"
    bashio::exit.nok
fi

if ! bashio::var.has_value "${BAUD_RATE}"; then
    bashio::log.fatal "Baud rate configuration is missing!"
    bashio::exit.nok
fi

if ! bashio::var.has_value "${PROTOCOL}"; then
    bashio::log.fatal "Protocol configuration is missing!"
    bashio::exit.nok
fi

if ! bashio::var.has_value "${SERIAL_CONFIG}"; then
    bashio::log.fatal "Serial config configuration is missing!"
    bashio::exit.nok
fi

bashio::log.info "Starting Viessmann Decoder Webserver..."
bashio::log.info "Serial Port: ${SERIAL_PORT}"
bashio::log.info "Baud Rate: ${BAUD_RATE}"
bashio::log.info "Protocol: ${PROTOCOL}"
bashio::log.info "Serial Config: ${SERIAL_CONFIG}"

# Check if serial port exists
if ! bashio::fs.file_exists "${SERIAL_PORT}"; then
    bashio::log.error "Serial port ${SERIAL_PORT} not found!"
    bashio::log.error "Available serial ports:"
    ls -la /dev/tty* 2>/dev/null || bashio::log.error "No serial ports found"
    bashio::exit.nok
fi

# Verify that the serial port is accessible (permissions / availability)
if ! exec 3<>"${SERIAL_PORT}" 2>/dev/null; then
    bashio::log.error "Cannot open serial port ${SERIAL_PORT}."
    bashio::log.error "Please check device permissions and ensure it is not in use by another process."
    bashio::exit.nok
fi
# Close the temporary file descriptor
if ! exec 3>&-; then
    bashio::log.warning "Failed to close test file descriptor, but continuing..."
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
