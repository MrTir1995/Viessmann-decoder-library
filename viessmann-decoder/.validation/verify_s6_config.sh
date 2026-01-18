#!/bin/bash
# Validation script for S6-Overlay v3 configuration
# This verifies the addon is properly configured to avoid tini /init errors

set -e

echo "=== S6-Overlay v3 Configuration Validation ==="
echo ""

# Check 1: Verify init: false in config.yaml
echo "✓ Checking config.yaml for 'init: false'..."
if grep -q "^init: false" ../config.yaml; then
    echo "  ✓ PASS: init: false is present in config.yaml"
else
    echo "  ✗ FAIL: init: false is missing from config.yaml"
    exit 1
fi
echo ""

# Check 2: Verify no CMD or ENTRYPOINT in Dockerfile
echo "✓ Checking Dockerfile has no CMD or ENTRYPOINT..."
if grep -qE "^(CMD|ENTRYPOINT)" ../Dockerfile; then
    echo "  ✗ FAIL: Dockerfile contains CMD or ENTRYPOINT which conflicts with S6-Overlay"
    exit 1
else
    echo "  ✓ PASS: No CMD or ENTRYPOINT in Dockerfile"
fi
echo ""

# Check 3: Verify service script has execute permissions
echo "✓ Checking service script permissions..."
if [ -x "../rootfs/etc/services.d/viessmann-decoder/run" ]; then
    echo "  ✓ PASS: Service script is executable"
else
    echo "  ✗ FAIL: Service script is not executable"
    exit 1
fi
echo ""

# Check 4: Verify git permissions for service script
echo "✓ Checking git permissions for service script..."
GIT_PERMS=$(git ls-files -s ../rootfs/etc/services.d/viessmann-decoder/run | awk '{print $1}')
if [ "$GIT_PERMS" = "100755" ]; then
    echo "  ✓ PASS: Git permissions are 100755 (executable)"
else
    echo "  ✗ FAIL: Git permissions are $GIT_PERMS (should be 100755)"
    exit 1
fi
echo ""

# Check 5: Verify shebang uses bashio
echo "✓ Checking service script uses bashio..."
if head -n1 ../rootfs/etc/services.d/viessmann-decoder/run | grep -q "bashio"; then
    echo "  ✓ PASS: Service script uses bashio shebang"
else
    echo "  ✗ WARNING: Service script doesn't use bashio shebang"
fi
echo ""

echo "=== All S6-Overlay v3 Configuration Checks Passed ==="
echo ""
echo "Configuration Summary:"
echo "  - init: false prevents Docker's tini from conflicting with S6-Overlay"
echo "  - No CMD/ENTRYPOINT allows S6-Overlay's /init to run as PID 1"
echo "  - Service script has proper executable permissions (755)"
echo "  - Home Assistant Supervisor will use S6-Overlay v3 to manage the service"
echo ""
echo "This configuration matches official Home Assistant addons like mosquitto and mariadb."
