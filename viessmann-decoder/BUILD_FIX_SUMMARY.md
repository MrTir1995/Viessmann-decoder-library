# Docker Build Fix Summary

## Problem
Home Assistant Supervisor was failing to build the Viessmann Decoder addon with the following errors:

```
ERROR: failed to calculate checksum: "/linux/include": not found
ERROR: failed to calculate checksum: "/linux/src": not found
ERROR: failed to calculate checksum: "/src": not found
ERROR: failed to calculate checksum: "/viessmann-decoder/webserver": not found
ERROR: failed to calculate checksum: "/viessmann-decoder/run.sh": not found
```

## Root Cause
The addon's `build.json` file was configured with:
```json
{
  "context": "..",
  "dockerfile": "viessmann-decoder/Dockerfile"
}
```

This configuration told Home Assistant Supervisor to use the parent directory (repository root) as the Docker build context. However, Home Assistant Supervisor does not properly support parent directory context access when building addons from git repositories. The supervisor cannot access files outside the addon directory during the build process.

## Solution (Current Implementation)
Made the addon **self-contained** by copying all required source files into the addon directory structure:

### Files Added to viessmann-decoder/
- `linux/src/` - Linux-specific implementations (Arduino.cpp, LinuxSerial.cpp, vbusdecoder.cpp)
- `linux/include/` - Header files (Arduino.h, LinuxSerial.h, vbusdecoder.h)
- `src/` - Core library source (VBUSDataLogger, VBUSMqttClient, VBUSScheduler, vbusdecoder)

### Changes to Configuration
Updated `build.json` to use default context (addon directory):
```json
{
  "build_from": { ... },
  "squash": false,
  "args": {}
}
```

Updated `Dockerfile` COPY commands to use local paths:
```dockerfile
# Before (parent directory references)
COPY linux/src /build/src
COPY linux/include /build/include
COPY src /build/library_src
COPY viessmann-decoder/webserver /build/webserver
COPY viessmann-decoder/run.sh /etc/s6-overlay/s6-rc.d/viessmann-webserver/run

# After (local paths)
COPY linux/src /build/src
COPY linux/include /build/include
COPY src /build/library_src
COPY webserver /build/webserver
COPY run.sh /etc/s6-overlay/s6-rc.d/viessmann-webserver/run
```

## Dockerfile Warning Fix
The Dockerfile already has:
```dockerfile
ARG BUILD_FROM=ghcr.io/home-assistant/amd64-base:3.18
FROM $BUILD_FROM
```

This provides a default value and eliminates Docker warnings about empty base image names.

## Result
The addon is now self-contained and builds successfully in Home Assistant Supervisor without requiring parent directory access. All necessary source files are included within the addon directory.
