#!/bin/bash

set -e

name=$1
flags="-g -Werror=declaration-after-statement -Wall -Wextra -pedantic -std=c99"
libs=
incl=
files=

if [[ $name = "assembler" ]]; then
    name="asm"
    files="./assembler/*.c util.c"
    incl="-I$HOME/programming/c-libs/include"
    libs="-L$HOME/programming/c-libs/lib -lartmem"
fi

if [[ $name = "linker" ]]; then
    name="ld"
    files="./linker/*.c util.c"
    incl="-I$HOME/programming/c-libs/include"
    libs="-L$HOME/programming/c-libs/lib -lartmem"
fi

if [[ $name = "preproc" ]]; then
    files="$name.c util.c"
    incl="-I$HOME/programming/c-libs/include"
    libs="-L$HOME/programming/c-libs/lib -lartmem"
fi

if [[ $name = "vm" ]]; then
    files="vm.c"
fi

if [[ $2 = "prod" ]]; then
    flags=${flags/-g/-O2}
fi

gcc $flags -o $name $files $incl $libs
