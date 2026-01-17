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
RUN mkdir -p /build/lib && \
    g++ -c -fPIC -I. *.cpp && \
    ar rcs /build/lib/libviessmann.a *.o && \
    g++ -shared -o /build/lib/libviessmann.so *.o

# Build the Linux serial wrapper
WORKDIR /build/src
RUN g++ -c -fPIC -I../include -I../library_src *.cpp

# Build the webserver application
WORKDIR /build/webserver
RUN g++ -o /usr/local/bin/viessmann_webserver \
    main.cpp \
    ../src/*.o \
    -I../include \
    -I../library_src \
    -L../lib \
    -lviessmann \
    -lmicrohttpd \
    -lpthread

# Clean up build files
WORKDIR /
RUN rm -rf /build

# Copy run script
COPY homeassistant-addon/run.sh /
RUN chmod +x /run.sh

CMD ["/run.sh"]
