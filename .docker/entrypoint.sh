#!/bin/bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
   # As per the spec here: https://github.com/ShaneMcC/aocbench#repo-requirements, this was run with no args to build the container
   exit 0
fi

if [[ -d "Day${1}" ]]; then
    cd Day${1}
    if [ "publish_dir/Day{$1}" -nt "Program.cs" ]; then
        dotnet publish -o publish_dir -c Release
    fi
    time publish_dir/Day${1}
fi
