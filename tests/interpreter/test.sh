#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
PARSER=$ROOT/parser/mitscript
TOTAL=0
SUCCESS=0
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

for filename in $ROOT/tests/interpreter/bad*.mit; do
    echo "==== TEST - $(basename $filename) ===="
    TOTAL=$((TOTAL+1))
    if $PARSER $filename > tmp.out; then
        echo -e "${RED}Test Failed${NC}"
    else
        SUCCESS=$((SUCCESS+1))
        echo -e "${GREEN}Test Passed${NC}"
    fi
    echo ""
    rm tmp.out
done

for filename in $ROOT/tests/interpreter/good*.mit; do
    echo "==== TEST - $(basename $filename) ===="
    TOTAL=$((TOTAL+1))
    $PARSER $filename > tmp.out
    if diff tmp.out $ROOT/tests/interpreter/$(basename $filename .mit).output; then
        SUCCESS=$((SUCCESS+1))
        echo -e "${GREEN}Test Passed${NC}"
    else
        echo -e "${RED}Test Failed${NC}"
    fi
    echo ""
    rm tmp.out
done

echo "Finished tests - ${SUCCESS}/${TOTAL} passed"
