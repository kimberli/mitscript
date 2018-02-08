#!/bin/bash

for filename in tests/*; do
    echo "Test - $(basename $filename)"
    cat $filename | ./mitscript-parser > tmp.out
    if diff tmp.out output/$(basename $filename).out; then
        echo "Test Passed"
    else
        echo "Test Failed"
    fi
    echo ""
    rm tmp.out
done
