#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
DIR=$ROOT/tests/vm/
PROG="${ROOT}/vm/mitscript -b"
TEST_FILE_EXT=".mitbc"
TARGET_FILE_EXT=".output"
THIS_FILE="test_interpreter.sh"

TOTAL=0
SUCCESS=0
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "Testing interpreter from bytecode to program output...\n"

run_test() {
    filename=$1
    echo "==== TEST - $(basename $filename) ===="
    TOTAL=$((TOTAL+1))
    $PROG $filename > tmp.out
    target=$DIR$(basename $filename $TEST_FILE_EXT)$TARGET_FILE_EXT
    if [ ! -e $target ]; then
        echo -e "Target file $target not found"
        echo -e "${RED}Test Failed${NC}\n"
        return 1
    else
        if diff tmp.out $target; then
            SUCCESS=$((SUCCESS+1))
            echo -e "${GREEN}Test Passed${NC}\n"
            return 0
        else
            echo -e "${RED}Test Failed${NC}\n"
            return 1
        fi
    fi
}

if [ -n "$1" ]; then
    if [ "$1" == "-h" ]; then
        echo "Usage: $THIS_FILE (optional test name)"
        echo "running without a test name will run all tests"
        exit 0
    else
        if [ ! -e "${1}${TEST_FILE_EXT}" ]; then
            echo "Test file ${1}${TEST_FILE_EXT} not found"
            exit 0
        fi
        if run_test "${1}${TEST_FILE_EXT}"; then
            rm tmp.out
        else
            echo -ne "${BLUE}IMPORTANT: if you believe the interpreter is in fact behaving correctly, do you want to update the ${TARGET_FILE_EXT} file? (Y/n):${NC} "
            read resp
            if [ "$resp" == "Y" ]; then
                mv tmp.out "${1}${TARGET_FILE_EXT}"
            else
                rm tmp.out
            fi
        fi
    fi
else
    for filename in $DIR*$TEST_FILE_EXT; do
        run_test $filename
    done
    rm tmp.out
fi

echo -e "\n\n----------"
echo "Finished tests - ${SUCCESS}/${TOTAL} passed"
