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

if [[ $1 = "assembler" ]]; then
    files="assembler/*.c lib/os.c lib/mem.c lib/path.c lib/sstring.c"
    outname=asm
fi

if [[ $1 = "preprocessor" ]]; then
    files="preprocessor/main.c lib/os.c lib/mem.c lib/path.c lib/sstring.c"
    outname=ppc
fi

if [[ $1 = "all" ]]; then
    ./build.sh vm $2 &
    ./build.sh assembler $2 &
    ./build.sh preprocessor $2 &
    wait
    exit
fi

if [[ $2 = "prod" ]]; then
    flags=${flags/-g/-O2}
fi

gcc $flags -o $outname $files $incl $libs
