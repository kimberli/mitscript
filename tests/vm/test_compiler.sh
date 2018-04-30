#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
DIR=$ROOT/tests/vm/
PROG="${ROOT}/mitscriptc"
TEST_FILE_EXT=".mit"
TARGET_FILE_EXT=".mitbc"
THIS_FILE="test_compiler.sh"
DIFF=$DIR/diff.txt
TEST_OUT=$DIR/tmp.out

VERBOSE=false

TOTAL=0
SUCCESS=0
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "Testing compiler from MITScript to bytecode...\n"

run_test() {
    # filename is the filename of the test with the appropriate file ext
    filename=$1
    
    if [[ $filename =~ "DirectOutput" ]]; then
        TARGET_FILE_EXT=".output"
    else
        TARGET_FILE_EXT=".mitbc"
    fi

    TOTAL=$((TOTAL+1))
    $PROG $filename > $TEST_OUT
    if [[ $? -eq 139 ]]; then
        echo -e "TEST $TOTAL - $(basename $filename): ${RED}Failed${NC}"
        return 0
    fi
    target=$DIR$(basename $filename $TEST_FILE_EXT)$TARGET_FILE_EXT
    if [ ! -e $target ]; then
        echo -e "TEST $TOTAL - $(basename $filename): ${RED}Failed${NC} (target file not found)"
        if [ $count == 1 ]; then
            cat $TEST_OUT > $DIFF
        fi
        return 1
    else
        diff $TEST_OUT $target > $DIFF
        if [ ! -s $DIFF ]; then
            SUCCESS=$((SUCCESS+1))
            if $VERBOSE; then
                echo -e "TEST $TOTAL - $(basename $filename): ${GREEN}Passed${NC}"
            fi
            return 0
        else
            echo -e "TEST $TOTAL - $(basename $filename): ${RED}Failed${NC}"
            return 1
        fi
    fi
}

if [ -n "$1" ]; then
    if [ "$1" == "-h" ]; then
        echo "Usage: $THIS_FILE (optional test name pattern)"
        echo "running without a test name will run all tests"
        echo "running with an argument will run all tests matching the argument's pattern"
        exit 0
    else
        filepattern=$1
        if ! [[ $filepattern =~ $TEST_FILE_EXT ]]; then
            filepattern=$filepattern*$TEST_FILE_EXT
        fi
        echo "Testing all matches to $filepattern..."
    fi
else
    filepattern=$TEST_FILE_EXT
    echo "Testing all files..."
fi

count=0
for filename in $DIR/*${filepattern}; do
    count=$((count+1))
done
for filename in $DIR/*${filepattern}; do
    if [ ! -f $filename ]; then
        echo "No files found"
        exit 0
    fi
    if run_test $filename; then
        rm $TEST_OUT
    elif [ $VERBOSE ] || [ $count == 1 ]; then
        cat $DIFF
        echo -ne "${BLUE}IMPORTANT: if you believe the interpreter is in fact behaving correctly, do you want to update the ${TARGET_FILE_EXT} file? (Y/n):${NC} "
        read resp
        if [ "$resp" == "Y" ]; then
            new_file=$DIR$(basename $filename $TEST_FILE_EXT)$TARGET_FILE_EXT
            echo -e "\t${BLUE}writing new output to $new_file${NC}"
            mv $TEST_OUT $new_file
        else
            rm $TEST_OUT
        fi
    fi
done

echo -e "\n----------"
echo "Finished $THIS_FILE - ${SUCCESS}/${TOTAL} passed"
