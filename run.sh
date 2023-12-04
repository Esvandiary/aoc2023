#!/bin/bash

set -e

IMAGE=esvandiary/aoc2023:3

docker image inspect "${IMAGE}" >/dev/null 2>&1 || docker build .docker -t "${IMAGE}"

exec docker run --rm -i -v "${PWD}:/code" "${IMAGE}" "${@}"
