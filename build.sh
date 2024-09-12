#!/bin/bash

set -e

files=
outname=

flags="-g -Werror=declaration-after-statement -Werror=switch-default -Wall -Wextra -pedantic -std=c99"
incl=
libs=

if [[ -z $1 ]]; then
    echo "provide build option"
    exit 1
fi

case $1 in
    vm)
        files=vm.c
        outname=vm
        ;;
    *)
        echo "unknown build option $1"
        exit 1
        ;;
esac

if [[ $2 = "prod" ]]; then
    flags=${flags/-g/-O2}
fi

gcc $flags -o $outname $files $incl $libs
