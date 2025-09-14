#!/bin/bash

set -e

files=main.c
flags="-Werror=declaration-after-statement -std=c99"
outfile=vm

if [[ $1 = "prod" ]]; then
    flags+=" -Wall -Wextra -Werror -pedantic -s -O3"
elif [[ $1 = "test" ]]; then
    flags+=" -D TEST"
else
    flags+=" -ggdb"
fi

gcc $flags -o $outfile $files

if [[ $1 = "run" || $2 == "run" ]]; then
    ./$outfile
fi
