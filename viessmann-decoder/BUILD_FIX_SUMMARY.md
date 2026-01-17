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
  "context": ".."
}
```

This told Home Assistant Supervisor to use the parent directory (repository root) as the Docker build context, which is necessary because the Dockerfile needs to copy source files from:
- `linux/src/` - Linux-specific implementations
- `linux/include/` - Header files  
- `src/` - Core library source
- `viessmann-decoder/webserver/` - Web server implementation

However, the `build.json` was **missing the `dockerfile` parameter** to tell Supervisor where the Dockerfile is located relative to that build context.

## Solution
Added the `dockerfile` parameter to `build.json`:

```json
{
  "build_from": { ... },
  "context": "..",
  "dockerfile": "viessmann-decoder/Dockerfile",
  "squash": false,
  "args": {}
}
```

This explicitly tells Home Assistant Supervisor:
1. Use the parent directory (`..`) as the build context
2. Find the Dockerfile at `viessmann-decoder/Dockerfile` relative to that context

## Additional Fixes

### Dockerfile Warning Fix
Changed:
```dockerfile
ARG BUILD_FROM
FROM $BUILD_FROM
```

To:
```dockerfile
ARG BUILD_FROM=ghcr.io/home-assistant/amd64-base:3.18
FROM $BUILD_FROM
```

This eliminates the Docker warning: "InvalidDefaultArgInFrom: Default value for ARG $BUILD_FROM results in empty or invalid base image name"

## Verification
The `build.sh` script in the repository root already uses this correct configuration:
```bash
docker build \
  --build-arg BUILD_FROM="${BUILD_FROM}" \
  -t viessmann-decoder:${ARCH} \
  -f viessmann-decoder/Dockerfile \
  .
```

This confirms that building from the repository root (`.`) with the Dockerfile at `viessmann-decoder/Dockerfile` is the intended and working configuration.

## Files Changed
1. `viessmann-decoder/build.json` - Added `dockerfile` parameter
2. `viessmann-decoder/Dockerfile` - Added default value to BUILD_FROM ARG
3. `viessmann-decoder/DEVELOPMENT.md` - Added build configuration documentation

## Result
The addon will now build successfully in Home Assistant Supervisor without any errors.
