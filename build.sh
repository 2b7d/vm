#!/bin/bash

set -xe

name="vm"
flags="-Werror=declaration-after-statement -Wall -Wextra -pedantic -std=c99 -g"

if [ "$1" = "prod" ]; then
    flags=${flags/-g/-O2}
fi

gcc $flags -o $name *.c
