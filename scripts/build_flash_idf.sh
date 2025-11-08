#!/bin/bash
set -euo pipefail

if [[ -z "${IDF_PATH:-}" ]]; then
  echo "IDF_PATH must be set"
  exit 1
fi

pushd idf_app >/dev/null
idf.py build
idf.py -p ${1:-/dev/tty.usbmodemXYZ} flash
popd >/dev/null
