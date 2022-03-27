#/bin/bash

make -C ../../../abgabe/parser clean
gcc test_suite_parser.c -o test_suite_parser
make -C ../../../abgabe/parser
./test_suite_parser
make -C ../../../abgabe/parser clean
rm -f test_suite_parser
