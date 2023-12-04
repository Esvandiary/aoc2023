#!/bin/bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
   # As per the spec here: https://github.com/ShaneMcC/aocbench#repo-requirements, this was run with no args to build the container
   exit 0
fi

if [[ -d "Day${1}" ]]; then
    cd "Day${1}"
    if [[ ! -d "build" ]]; then
        cmake . --preset=makefiles
    fi
    if [[ ! -f "build/Day${1}" || "main.cpp" -nt "build/Day${1}" ]]; then
        cmake --build --preset=makefiles
    fi
    time build/Day${1}
fi
