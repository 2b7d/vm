#!/bin/bash

set -xe

files=vm.c
outname=vm

flags="-g -Werror=declaration-after-statement -Wall -Wextra -pedantic -std=c99"
incl=
libs=

if [[ $1 = "prod" ]]; then
    flags=${flags/-g/-O2}
fi

gcc $flags -o $outname $files $incl $libs
