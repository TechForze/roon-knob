#!/usr/bin/env bash
set -euo pipefail
PORT="${PORT:-/dev/cu.usbmodem101}"
SSID="${SSID:-YourWiFi}"
PASS="${PASS:-YourPass}"
BRIDGE_BASE="${BRIDGE_BASE:-http://192.168.1.10:8088}"
ZONE_ID="${ZONE_ID:-office}"

cd "$(dirname "$0")/../idf_app"

# Write credentials to git-ignored sdkconfig.local
cat > sdkconfig.local <<EOF
CONFIG_RK_DEFAULT_SSID="${SSID}"
CONFIG_RK_DEFAULT_PASS="${PASS}"
CONFIG_RK_DEFAULT_BRIDGE_BASE="${BRIDGE_BASE}"
CONFIG_RK_DEFAULT_ZONE_ID="${ZONE_ID}"
EOF
echo "Updated sdkconfig.local: SSID=$SSID BRIDGE_BASE=$BRIDGE_BASE ZONE_ID=$ZONE_ID"

# Only set-target if not already set to avoid unnecessary fullclean
if [ ! -f build/CMakeCache.txt ] || ! grep -q "IDF_TARGET:STRING=esp32s3" build/CMakeCache.txt 2>/dev/null; then
    idf.py set-target esp32s3
fi

# Set SDKCONFIG_DEFAULTS to load sdkconfig.local along with defaults
export SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.local"

# Ensure sdkconfig exists and has been regenerated with sdkconfig.local
rm -f sdkconfig
idf.py reconfigure

idf.py build
export ESPTOOL_OPEN_PORT_ATTEMPTS=0
idf.py -p "$PORT" -b 921600 flash
echo "Flashed with: SSID=$SSID BRIDGE_BASE=$BRIDGE_BASE ZONE_ID=$ZONE_ID"
