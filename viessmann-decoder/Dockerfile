ARG BUILD_FROM=ghcr.io/home-assistant/amd64-base:3.18
FROM $BUILD_FROM

# Install build dependencies and runtime libraries
RUN apk add --no-cache \
    build-base \
    g++ \
    make \
    linux-headers \
    libmicrohttpd-dev \
    libmicrohttpd \
    curl

# Set working directory
WORKDIR /build

# Copy source files (now using local addon directory structure)
COPY linux/src /build/src
COPY linux/include /build/include
COPY src /build/library_src
COPY webserver /build/webserver

# Build the library
WORKDIR /build/library_src
RUN g++ -c -fPIC -I. -I../include vbusdecoder.cpp -o vbusdecoder.o

# Build the Linux serial wrapper
WORKDIR /build/src
RUN g++ -c -fPIC -I../include -I../library_src LinuxSerial.cpp -o LinuxSerial.o && \
    g++ -c -fPIC -I../include -I../library_src Arduino.cpp -o Arduino.o

# Build the webserver application
WORKDIR /build/webserver
RUN g++ -o /usr/local/bin/viessmann_webserver \
    main.cpp \
    ../library_src/vbusdecoder.o \
    ../src/LinuxSerial.o \
    ../src/Arduino.o \
    -I../include \
    -I../library_src \
    -lmicrohttpd \
    -lpthread \
    && strip /usr/local/bin/viessmann_webserver

# Clean up build files and remove build dependencies
WORKDIR /
RUN rm -rf /build /root/.cache && \
    apk del --no-cache build-base g++ make linux-headers libmicrohttpd-dev

# Copy the run script to the root directory (Home Assistant addon standard location)
COPY run.sh /run.sh
RUN chmod a+x /run.sh

# Health check to monitor webserver availability (curl is kept for healthcheck)
# Increased tolerance: 60s start period, 5 retries to avoid false positives triggering restarts
# Uses /health endpoint which is lightweight and always responds quickly
HEALTHCHECK --interval=30s --timeout=10s --start-period=60s --retries=5 \
    CMD curl -f http://localhost:8099/health || exit 1

# Home Assistant uses s6-overlay v3 which must be PID 1
# init: false in config.yaml allows s6-overlay to manage the process
# Do not set ENTRYPOINT - it conflicts with the Supervisor's management
CMD ["/run.sh"]
