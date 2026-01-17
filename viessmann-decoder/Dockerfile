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

# Setup S6-Overlay v3 service structure
RUN mkdir -p /etc/s6-overlay/s6-rc.d/viessmann-webserver && \
    mkdir -p /etc/s6-overlay/s6-rc.d/viessmann-webserver/dependencies.d && \
    mkdir -p /etc/s6-overlay/s6-rc.d/user/contents.d && \
    echo "longrun" > /etc/s6-overlay/s6-rc.d/viessmann-webserver/type && \
    touch /etc/s6-overlay/s6-rc.d/viessmann-webserver/dependencies.d/base && \
    touch /etc/s6-overlay/s6-rc.d/user/contents.d/viessmann-webserver

# Create S6 service run script (now using local addon directory structure)
COPY run.sh /etc/s6-overlay/s6-rc.d/viessmann-webserver/run
RUN chmod +x /etc/s6-overlay/s6-rc.d/viessmann-webserver/run

# Health check to monitor webserver availability (curl is kept for healthcheck)
HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
    CMD curl -f http://localhost:8099/ || exit 1

# S6-Overlay v3 requires /init as ENTRYPOINT to run as PID 1
ENTRYPOINT ["/init"]

CMD []
