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
    curl

# Set working directory
WORKDIR /build

# Copy source files
COPY linux/src /build/src
COPY linux/include /build/include
COPY src /build/library_src
COPY homeassistant-addon/webserver /build/webserver

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
    -lpthread

# Clean up build files and remove build dependencies
WORKDIR /
RUN rm -rf /build && apk del --no-cache build-base g++ make linux-headers libmicrohttpd-dev

# Copy run script
COPY homeassistant-addon/run.sh /
RUN chmod +x /run.sh

CMD ["/run.sh"]
