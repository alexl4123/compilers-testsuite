# compilers-testsuite

Testsuite for Compilers (185.A48) - Summer term 2022 - Vienna University of Technology

# File structure

The file structure resembles closely the hand in file structure (`abgabe`), which is basically the following:

`
|
|-abgabe
\
 |-asma
 |-asmb
 |-scanner
 \
  |-test_suite_scanner.c
  |-README.md
 |-parser
 ...
`


# how to contribute?

If you want to add additional test suites, etc. you may do so - just make a pull request. When you do so, please adhere to the following conventions:

- **Commit messages**: Commit messages should begin with a `[` followed by the exercise name/what the commit is about in short, followed by a closing `]`. Then one needs to provide details about the commit, e.g. `[scanner] - included the test cases from the server`
- **Assignment code**: Do not include your code for the asssignment (e.g. the `scanner.c` file in the scanner exercise) and further do not include code which might go against the rules of the lecture.
- **Add build howto as readme**: Add a readme to your testsuite, where you describe how one may build and execute your test cases. This file should be located next to your test suite, e.g.: `abgabe/scanner/README.md` describes how one can build and execute the test cases for the scanner.
- **Common sense**: Use your brain before creating pull requests :-)

# Using the test cases does not guarantee, that you will get 100 points on the exercises!
