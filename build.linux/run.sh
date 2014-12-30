#!/bin/bash

make

if [ "$1" == "1-2" ]; then
    ./nachos -e ../test/test1 -e ../test/test2
elif [ "$1" == "2l" ]; then
    ./nachos -ep ../test/test2 40 -ep ../test/test3 80
elif [ "$1" == "3l" ]; then
    ./nachos -ep ../test/test4 105 -ep ../test/test5 100
fi

