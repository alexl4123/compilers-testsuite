#/bin/bash

make -C ../../../abgabe/gesamt clean
gcc test_suite_gesamt.c -o test_suite_gesamt 2> /dev/null
make -C ../../../abgabe/gesamt
./test_suite_gesamt 2> /dev/null
make -C ../../../abgabe/gesamt clean

rm -f test_suite_gesamt
rm test
rm test.c
rm test.o
rm test.s
rm test_asm.o
