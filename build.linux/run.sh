#!/bin/bash

make

if [ "$1" == "1-2" ]; then
    ./nachos -e ../test/test1 -e ../test/test2
elif [ "$1" == "2L" ]; then
    ./nachos -ep ../test/test2 40 -ep ../test/test3 80
fi
