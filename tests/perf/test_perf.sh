#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
DIR=$ROOT/tests/perf/
PROG="${ROOT}/mitscript --opt=all -s"
TEST_FILE_EXT=".mit"
INPUT_FILE_EXT=".input"
TARGET_FILE_EXT=".output"
THIS_FILE="test_perf.sh"
DIFF=$DIR/diff.txt
TEST_OUT=$DIR/tmp.out

VERBOSE=false
NUM_TIMES=5

TOTAL=0
SUCCESS=0
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "Testing performance of interpreter from MITScript to program output..."

run_test() {
    # filename is the filename of the test with the appropriate file ext
    filename=$1

    TOTAL=$((TOTAL+1))
    $PROG $filename > $TEST_OUT
    if [[ $? -eq 139 ]]; then
        echo -e "TEST $TOTAL - $(basename $filename): ${RED}Failed${NC}"
        return 0
    fi
}

repeat_tests() {
    for i in $(NUM_TIMES); do
        run_test $1
    done
}

time_tests() {
    filename=$1
    run_test $filename  # warm up cache
    time repeat_tests $filename

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
    time_tests $filename
    rm -f $TEST_OUT
    rm -f $DIFF
done

echo -e "\n----------"
echo -e "Finished $THIS_FILE - ${SUCCESS}/${TOTAL} passed\n"
