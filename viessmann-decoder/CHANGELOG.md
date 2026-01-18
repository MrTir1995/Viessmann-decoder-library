# Changelog

All notable changes to the Viessmann Decoder Home Assistant Add-on will be documented in this file.

## [Unreleased]

### Fixed
- Fixed "/bin/sh: can't open '/init': Permission denied" error by ensuring `init: false` in config.yaml
  - S6-Overlay v3 requires running as PID 1, which is prevented when `init: true` causes Docker to inject tini as PID 1
  - With `init: false`, the Home Assistant base image's S6-Overlay properly initializes as PID 1
  - Added explicit `chmod a+x` in Dockerfile to ensure run.sh has execute permissions
  - See: https://developers.home-assistant.io/blog/2022/05/12/s6-overlay-base-images/

## [2.1.0] - 2026-01-17

### Added
- Initial release of Home Assistant add-on
- Web-based dashboard for monitoring heating system
- Support for VBUS, KW-Bus, P300, and KM-Bus protocols
- Real-time data display (temperatures, pumps, relays)
- Configuration through Home Assistant UI
- Docker-based deployment
- Alpine Linux for minimal resource usage
- RESTful API at `/data` endpoint for Home Assistant integration
- Status page with system information
- Automatic serial port configuration
- Multi-architecture support (armhf, armv7, aarch64, amd64, i386)

### Features
- Dashboard with auto-refreshing data (2-second interval)
- JSON API for programmatic access
- Status monitoring and error reporting
- Lightweight C++ implementation with libmicrohttpd
- Based on proven Viessmann Multi-Protocol Library

## Protocol Support

### VBUS Protocol
- Viessmann Vitosolic 200
- RESOL DeltaSol BX/BX Plus/MX
- Up to 32 temperature sensors
- Up to 32 pumps
- Up to 32 relays

### KW-Bus Protocol
- Vitotronic 100/200/300 series
- Temperature monitoring
- Status information

### P300 Protocol
- Modern Vitodens boilers
- Vitocrossal systems
- Optolink interface support

### KM-Bus Protocol
- Vitotrol remote controls
- Expansion modules
- Full status decoding
