#!/bin/bash

set -xe

files=
outname=

flags="-g -Werror=declaration-after-statement -Werror=switch-enum -Werror=switch-default -Wall -Wextra -pedantic -std=c99"
incl=
libs=

if [[ $1 = "vm" ]]; then
    files=vm.c
    outname=vm
fi

if [[ $1 = "asm" ]]; then
    files="assembler/*.c lib/os.c lib/mem.c"
    outname=asm
fi

if [[ $2 = "prod" ]]; then
    flags=${flags/-g/-O2}
fi

gcc $flags -o $outname $files $incl $libs
