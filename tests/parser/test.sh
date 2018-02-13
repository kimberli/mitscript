#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
PARSER=$ROOT/parser/mitscript-parser

for filename in $ROOT/tests/parser/input/*; do
    echo "Test - $(basename $filename)"
    cat $filename | $PARSER > tmp.out
    if diff tmp.out $ROOT/tests/parser/output/$(basename $filename).out; then
        echo "Test Passed"
    else
        echo "Test Failed"
    fi
    echo ""
    rm tmp.out
done
