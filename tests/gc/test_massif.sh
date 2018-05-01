#!/bin/bash
ROOT=$(git rev-parse --show-toplevel)
DIR=$ROOT/tests/gc/
LIMIT="4"  # in MB
PROG="${ROOT}/mitscript --opt=all -mem $LIMIT -s"
TEST_FILE_EXT=".mit"
THIS_FILE="test_massif.sh"

TOTAL=0
SUCCESS=0
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color
TOTAL_USED=0
mem_regex="([\.0-9]+)\^"

run_test() {
    filename=$1
    echo "==== TEST - $(basename $filename) ===="
    TOTAL=$((TOTAL+1))
    valgrind --tool=massif --log-file=$DIR/tmp2.txt --massif-out-file=$DIR/tmp.txt --threshold=100 $PROG $filename > /dev/null
    ms_print --threshold=100 $DIR/tmp.txt > $DIR/graph.txt
    if [[ $(cat $DIR/graph.txt) =~ $mem_regex ]]; then
        mem_used="${BASH_REMATCH[1]}"
    fi
    if [[ $(cat $DIR/graph.txt) =~ "KB" ]]; then
        mem_used=$(echo "$mem_used / 1024" | bc -l)
    elif [[ $(cat $DIR/graph.txt) =~ "GB" ]]; then
        mem_used=$(echo "$mem_used * 1024" | bc -l)
    fi
    sed -ne '7,30p' $DIR/graph.txt
    rm $DIR/tmp.txt
    rm $DIR/tmp2.txt
    rm $DIR/graph.txt
    printf "\nMax mem used: %0.2f (limit was %d MB)\n" $mem_used $LIMIT
    TOTAL_USED=$(echo "$TOTAL_USED + $mem_used" | bc -l)
    if (( $(echo "$mem_used < $LIMIT" | bc -l) )); then
        SUCCESS=$((SUCCESS+1))
        echo -e "${GREEN}Test Passed${NC}\n"
        return 0
    else
        echo -e "${RED}Test Failed${NC}\n"
        return 0
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
        count=0
        for filename in *${filepattern}; do
            count=$((count+1))
        done
        for filename in *${filepattern}; do
            if [ ! -f $filename ]; then
                echo "No files found"
                exit 0
            fi
            run_test $filename
        done
    fi
else
    for filename in $DIR*$TEST_FILE_EXT; do
        run_test $filename
    done
fi

echo -e "\n\n----------"
echo "Finished tests - ${SUCCESS}/${TOTAL} passed"
average=$(echo "$TOTAL_USED / $TOTAL" | bc -l)
printf "Total memory used: %0.2f MB, average used: %0.2f MB\n" $TOTAL_USED $average
