#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
DIR=$ROOT/tests/vm/
PROG="${ROOT}/vm/mitscript -s"
TEST_FILE_EXT=".mit"
TARGET_FILE_EXT=".output"
THIS_FILE="test_entire_interpreter.sh"

TOTAL=0
SUCCESS=0
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "Testing interpreter from MITScript to program output...\n"

run_test() {
    # filename is the filename of the test with the appropriate file ext
    filename=$1

    echo "==== TEST - $(basename $filename) ===="
    TOTAL=$((TOTAL+1))
    $PROG $filename > tmp.out
    target=$DIR$(basename $filename $TEST_FILE_EXT)$TARGET_FILE_EXT
    if [ ! -e $target ]; then
        echo -e "Target file $target not found"
        echo -e "${RED}Test Failed${NC}\n"
        echo "GOT:"
        cat tmp.out
        echo ""
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
        filename=$1
        if ! [[ $filename =~ $TEST_FILE_EXT ]]; then
            echo "sub"
            filename="$filename$TEST_FILE_EXT"
        fi
        if [ ! -e $filename ]; then
            echo "Test file $filename not found"
            exit 0
        fi
        if run_test $filename; then
            rm tmp.out
        else
            echo -ne "${BLUE}IMPORTANT: if you believe the interpreter is in fact behaving correctly, do you want to update the ${TARGET_FILE_EXT} file? (Y/n):${NC} "
            read resp
            if [ "$resp" == "Y" ]; then
                new_file=$(basename $filename $TEST_FILE_EXT)$TARGET_FILE_EXT
                echo -e "\t${BLUE}writing new output to $new_file${NC}"
                mv tmp.out $new_file
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
