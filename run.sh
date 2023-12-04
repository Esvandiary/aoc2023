#!/bin/bash

set -e

IMAGE=Esvandiary/aoc2023

docker image inspect "${IMAGE}" >/dev/null 2>&1 || docker build .docker -t "${IMAGE}"

exec docker run --rm -i -v "${PWD}:/code" "${IMAGE}" "${@}"
