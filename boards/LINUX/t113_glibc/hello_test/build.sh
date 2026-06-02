#!/bin/bash
# Build hello_world with the T113 toolchain wrapper (no make required).
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

GCC="${SCRIPT_DIR}/../toolchain_wrapper/t113-gcc"
if [[ ! -x "${GCC}" ]]; then
    echo "Error: ${GCC} not found or not executable"
    exit 1
fi

"${GCC}" -O2 -Wall -Wextra -std=c99 -o hello_world main.c
file hello_world
echo "OK: ${SCRIPT_DIR}/hello_world"
