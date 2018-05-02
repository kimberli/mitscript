#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
DIR=$ROOT/tests/vm/

input_dir=$1

diff_file() {
    file=$1
    filename=$(basename -- "$file")
    extension="${filename##*.}"
    for target in $DIR/*${extension}; do
        result=$(diff $file $target)
        if [ "$result" == "" ]; then
            echo -e "\tmatches $target"
        fi
    done
}

for f in $input_dir/*.mit; do
    echo "checking $f"
    diff_file $f
done
