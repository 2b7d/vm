#!/bin/bash

set -xe

name="$1"
flags="-Werror=declaration-after-statement -Wall -Wextra -pedantic -std=c99 -g"

if [ "$2" = "prod" ]; then
    flags=${flags/-g/-O2}
fi

gcc $flags -o $name $name.c
