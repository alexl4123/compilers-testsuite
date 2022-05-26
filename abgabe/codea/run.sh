#/bin/bash

make -C ../../../abgabe/codea clean
gcc test_suite_codea.c -o test_suite_codea 2> /dev/null
make -C ../../../abgabe/codea
./test_suite_codea  2> /dev/null
make -C ../../../abgabe/codea clean

rm -f test_suite_codea
rm test
rm test.c
rm test.o
rm test.s
rm test_asm.o
