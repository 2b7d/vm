#!/bin/bash

set -e

name="$1"
flags="-Werror=declaration-after-statement -Wall -Wextra -pedantic -std=c99 -g"
libs=""
incl=""
files=""

if [ "$name" = "assembler" ]; then
    name="asm"
    files="./assembler/*.c util.c"
    incl="-I/home/fosseddy/programming/c-sandbox/mem"
    libs="-L/home/fosseddy/programming/c-sandbox/mem -lmem"
fi

if [ "$name" = "linker" ]; then
    name="ld"
    files="./linker/*.c"
    #incl="-I/home/fosseddy/programming/c-sandbox/mem"
    #libs="-L/home/fosseddy/programming/c-sandbox/mem -lmem"
fi

if [ "$name" = "preproc" ]; then
    incl="-I/home/fosseddy/programming/c-sandbox/mem"
    libs="-L/home/fosseddy/programming/c-sandbox/mem -lmem"
    files="$name.c util.c"
fi

if [ "$2" = "prod" ]; then
    flags=${flags/-g/-O2}
fi

gcc $flags -o $name $files $incl $libs
