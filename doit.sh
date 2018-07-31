#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd ${DIR}
mkdir -p build
pushd build
emconfigure ../configure  CFLAGS=-O2 LDFLAGS=-O2
emmake make -j 8
emcc -o bindings.bc -O2 ../bindings.c
emcc -o ../fec.js bindings.bc -s MODULARIZE=1 -s EXPORT_NAME="fec" -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap", "_malloc", "_free"]' -O2 -L $PWD -lfec
popd #build

popd #dir
