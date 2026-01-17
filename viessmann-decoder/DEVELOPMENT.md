# Viessmann Decoder - Home Assistant Add-on

This directory contains the Home Assistant add-on for the Viessmann Multi-Protocol Library. This add-on runs as a Docker container and provides a web interface for monitoring Viessmann heating systems.

## Structure

```
homeassistant-addon/
├── config.json          # Add-on configuration and metadata
├── build.json           # Docker build configuration
├── Dockerfile           # Container build instructions
├── run.sh              # Entry point script
├── README.md           # User documentation
├── DOCS.md             # Brief add-on description
├── CHANGELOG.md        # Version history
└── webserver/
    └── main.cpp        # C++ web server implementation
```

## Building the Add-on

### Local Build

To build the add-on locally for testing:

```bash
# Build for your architecture
docker build -t viessmann-decoder \
  --build-arg BUILD_FROM=ghcr.io/home-assistant/amd64-base:3.18 \
  -f homeassistant-addon/Dockerfile .

# Run locally
docker run -it --rm \
  --device=/dev/ttyUSB0 \
  -e SERIAL_PORT=/dev/ttyUSB0 \
  -e BAUD_RATE=9600 \
  -e PROTOCOL=vbus \
  -e SERIAL_CONFIG=8N1 \
  -p 8099:8099 \
  viessmann-decoder
```

### Multi-Architecture Build

For publishing to multiple architectures:

```bash
# Install buildx if not already available
docker buildx create --name multiarch --use

# Build for all architectures
docker buildx build \
  --platform linux/amd64,linux/arm64,linux/arm/v7 \
  -t ghcr.io/mrtir1995/viessmann-decoder:latest \
  -f homeassistant-addon/Dockerfile \
  --push .
```

## Installation in Home Assistant

### Method 1: Add Repository

1. Navigate to Settings → Add-ons → Add-on Store
2. Click the three dots (⋮) in the top right
3. Select "Repositories"
4. Add: `https://github.com/MrTir1995/Viessmann-decoder-library`
5. Find "Viessmann Decoder" in the add-on store
6. Click Install

### Method 2: Local Add-on

1. Copy the `homeassistant-addon` directory to `/addons/viessmann-decoder` on your Home Assistant system
2. Go to Settings → Add-ons → Add-on Store
3. Refresh the page
4. Find "Viessmann Decoder" under Local Add-ons
5. Click Install

## Configuration

Configure through the Home Assistant UI:

- **serial_port**: The serial device (e.g., `/dev/ttyUSB0`)
- **baud_rate**: Communication speed (9600 for VBUS, 4800 for KW/P300)
- **protocol**: Protocol type (vbus, kw, p300, km)
- **serial_config**: Serial settings (8N1 or 8E2)

## Development

### Prerequisites

- Docker
- C++ compiler (g++)
- libmicrohttpd

### Modifying the Webserver

The webserver source is in `webserver/main.cpp`. After making changes:

1. Test locally with the build command above
2. Verify functionality
3. Update version in `config.json`
4. Update `CHANGELOG.md`

### Testing

```bash
# Build the container
docker build -t viessmann-decoder-test \
  --build-arg BUILD_FROM=ghcr.io/home-assistant/amd64-base:3.18 \
  -f homeassistant-addon/Dockerfile .

# Run with test configuration
docker run -it --rm \
  --device=/dev/ttyUSB0 \
  -p 8099:8099 \
  viessmann-decoder-test
```

## Dependencies

The add-on uses:
- **Alpine Linux**: Lightweight base OS
- **libmicrohttpd**: HTTP server library
- **Viessmann Multi-Protocol Library**: Core protocol implementation

## Troubleshooting

### Build Fails

Check that all source files are present:
- `src/` - Library source code
- `linux/src/` and `linux/include/` - Linux wrappers
- `homeassistant-addon/webserver/main.cpp` - Webserver

### Serial Port Access

Ensure the device is mapped in `config.json`:
```json
"devices": [
  "/dev/ttyUSB0",
  "/dev/ttyUSB1"
]
```

### Web Interface Not Accessible

- Check that port 8099 is not blocked
- Verify the container is running: `docker ps`
- Check logs: Settings → Add-ons → Viessmann Decoder → Log

## Contributing

Contributions are welcome! Please:
1. Test changes thoroughly
2. Update documentation
3. Follow existing code style
4. Submit pull requests to the main repository

## License

See the main repository LICENSE file.
