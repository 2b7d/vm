#!/bin/bash

set -e

name="$1"
flags="-Werror=declaration-after-statement -Wall -Wextra -pedantic -std=c99 -g"
libs=""

if [ "$2" = "prod" ]; then
    flags=${flags/-g/-O2}
fi

if [ "$name" = "asm" ]; then
    libs="-I/home/fosseddy/programming/c-sandbox/mem -L/home/fosseddy/programming/c-sandbox/mem -lmem"
fi

gcc $flags -o $name $name.c $libs
