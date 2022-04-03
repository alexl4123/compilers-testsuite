#/bin/bash

make -C ../../../abgabe/ag clean
gcc test_suite_ag.c -o test_suite_ag
make -C ../../../abgabe/ag
./test_suite_ag
make -C ../../../abgabe/ag clean
rm -f test_suite_ag
