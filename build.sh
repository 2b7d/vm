#!/bin/bash

set -e

name="$1"
flags="-Werror=declaration-after-statement -Wall -Wextra -pedantic -std=c99 -g"
libs=""
files=""

if [ "$name" = "asm" ] || [ "$name" = "preproc" ] || [ "$name" = "ld" ]; then
    libs="-I/home/fosseddy/programming/c-sandbox/mem -L/home/fosseddy/programming/c-sandbox/mem -lmem"
    files="util.c"
fi

if [ "$2" = "prod" ]; then
    flags=${flags/-g/-O2}
fi

gcc $flags -o $name $name.c $files $libs
