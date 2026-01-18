# Validation Scripts

This directory contains validation scripts for the Viessmann Decoder addon.

## verify_s6_config.sh

Validates the addon is properly configured for S6-Overlay v3 to prevent the "[FATAL tini (7)] exec /init failed: Permission denied" error.

### What it checks:

1. **init: false in config.yaml** - Prevents Docker's tini from conflicting with S6-Overlay
2. **No CMD or ENTRYPOINT in Dockerfile** - Allows S6-Overlay's /init to run as PID 1
3. **Service script executable permissions** - Ensures the service can start
4. **Git permissions** - Verifies executable flag is tracked in git (100755)
5. **Bashio shebang** - Confirms proper integration with Home Assistant

### Usage:

```bash
cd .validation
./verify_s6_config.sh
```

### Background

Home Assistant base images use S6-Overlay v3, which must run as PID 1 to manage services correctly. The `init: false` configuration tells the Supervisor not to inject Docker's default init system (tini), allowing S6-Overlay to properly manage the addon.

This configuration matches all official Home Assistant addons including mosquitto, mariadb, and the example addon.

### References

- [Home Assistant S6-Overlay v3 Migration Guide](https://developers.home-assistant.io/blog/2022/05/12/s6-overlay-base-images/)
- [Add-on Configuration Documentation](https://developers.home-assistant.io/docs/add-ons/configuration/)
