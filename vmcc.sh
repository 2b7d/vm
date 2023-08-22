if [[ -z $1 ]]; then
    echo "provide files to build"
    exit 1
fi

files=
objs=

for f in $@; do
    in=$(basename $f .asm)

    ./ppc $f
    files+=" $in.i"
    if [[ $? -ne 0 ]]; then
        rm $files
        exit 1
    fi

    ./asm "$in.i"
    files+=" $in.o"
    objs+=" $in.o"
    if [[ $? -ne 0 ]]; then
        rm $files
        exit 1
    fi
done

./ln $objs
if [[ $? -ne 0 ]]; then
    rm out.vm $files
    exit 1
fi

./vm out.vm
if [[ $? -ne 0 ]]; then
    rm $files
    exit 1
fi

rm $files
exit 0
