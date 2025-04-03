#!/bin/bash

set -e

files=vm.c

flags="-Werror=declaration-after-statement \
       -Wall -Wextra -Werror \
       -pedantic -std=c99"

if [[ $1 = "prod" ]]; then
    flags+=" -s -O3"
else
    flags+=" -ggdb"
fi

gcc $flags -o vm $files
