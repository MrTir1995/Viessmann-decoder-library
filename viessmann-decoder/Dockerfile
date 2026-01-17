ARG BUILD_FROM
FROM $BUILD_FROM

# Install build dependencies and runtime libraries
RUN apk add --no-cache \
    build-base \
    g++ \
    make \
    linux-headers \
    libmicrohttpd-dev \
    libmicrohttpd \
    curl \
    && mkdir -p /etc/s6-overlay/s6-rc.d

# Set working directory
WORKDIR /build

# Copy source files
COPY linux/src /build/src
COPY linux/include /build/include
COPY src /build/library_src
COPY viessmann-decoder/webserver /build/webserver

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
    mkdir -p /etc/s6-overlay/s6-rc.d/user/contents.d

# Create S6 service type file
RUN echo "longrun" > /etc/s6-overlay/s6-rc.d/viessmann-webserver/type

# Create S6 service run script
COPY viessmann-decoder/run.sh /etc/s6-overlay/s6-rc.d/viessmann-webserver/run
RUN chmod +x /etc/s6-overlay/s6-rc.d/viessmann-webserver/run

# Create service dependencies (depends on base)
RUN touch /etc/s6-overlay/s6-rc.d/viessmann-webserver/dependencies.d/base

# Add service to user bundle
RUN touch /etc/s6-overlay/s6-rc.d/user/contents.d/viessmann-webserver

# Health check to monitor webserver availability
HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
    CMD curl -f http://localhost:8099/ || exit 1

CMD []
