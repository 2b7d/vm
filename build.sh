#!/bin/bash

set -e

name="$1"
flags="-Werror=declaration-after-statement -Wall -Wextra -pedantic -std=c99 -g"
libs=""
incl=""
files=""

if [ "$name" = "asm" ]; then
    name="asm.out"
    files="./asm/main.c ./asm/scanner.c ./asm/compiler.c util.c"
    incl="-I/home/fosseddy/programming/c-sandbox/mem"
    libs="-L/home/fosseddy/programming/c-sandbox/mem -lmem"
fi

if [ "$name" = "preproc" ] || [ "$name" = "ld" ]; then
    incl="-I/home/fosseddy/programming/c-sandbox/mem"
    libs="-L/home/fosseddy/programming/c-sandbox/mem -lmem"
    files="$name.c util.c"
fi

if [ "$2" = "prod" ]; then
    flags=${flags/-g/-O2}
fi

gcc $flags -o $name $files $incl $libs
