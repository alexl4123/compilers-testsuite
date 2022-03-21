#/bin/bash

make clean
gcc test_suite_scanner.c -o test_suite_scanner
make
./test_suite_scanner
make clean