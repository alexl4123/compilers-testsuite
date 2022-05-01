#/bin/bash

make -C ../../../abgabe/codeb clean
gcc test_suite_codeb.c -o test_suite_codeb
make -C ../../../abgabe/codeb
./test_suite_codeb
make -C ../../../abgabe/codeb clean

rm -f test_suite_codeb
rm test
rm test.c
rm test.o
rm test.s
rm test_asm.o
