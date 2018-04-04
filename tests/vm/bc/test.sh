#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
PROG=$ROOT/vm/interpreter
TOTAL=0
SUCCESS=0
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

for filename in $ROOT/tests/vm/bc/*.mitbc; do
    echo "==== TEST - $(basename $filename) ===="
    TOTAL=$((TOTAL+1))
    $PROG -b $filename > tmp.out
    if diff tmp.out $ROOT/tests/vm/bc/$(basename $filename .mitbc).output; then
        SUCCESS=$((SUCCESS+1))
        echo -e "${GREEN}Test Passed${NC}"
    else
        echo -e "${RED}Test Failed${NC}"
    fi
    echo ""
    rm tmp.out
done

echo "Finished tests - ${SUCCESS}/${TOTAL} passed"
