#!/bin/bash

docker build . -f Dockerfile -t lkv-toolchain
docker run -it -v "$(readlink -f ../custom_firmware/Hello\ World/):/source" -w /source lkv-toolchain ./compile.sh *.c
