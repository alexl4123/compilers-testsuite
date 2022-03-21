#/bin/bash

make -C ../../../abgabe/scanner clean
gcc test_suite_scanner.c -o test_suite_scanner
make -C ../../../abgabe/scanner
./test_suite_scanner
make -C ../../../abgabe/scanner clean
rm -f test_suite_scanner