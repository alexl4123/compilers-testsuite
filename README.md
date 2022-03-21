# Testsuite for Compiler TU Wien SS22

This repo contains some test cases that can be used to verify the assignment solutions and
to get some feedback in the case of errors. The test cases try to handle as many cases as we could
think of but if you feel like there are some missing test cases please consider contributing to this
repository.

<span style="color:red">Using the test cases in this repo will not guarantee 100 points on the exercises! </span>

<br>

## Usage

Put the git-repo (compilers-testsuite) on the same level as the `abgabe` folder on the complang machine.
(see section: file structure)


Some of the test cases have a `run.sh` file in their respective folder.
This script will build both the test file and your file and then test your solution against
the defined test cases. The script will also cleanup after it is done.

If a folder does not provide a script then there should be at least a README.md
in the respective folder that contains instructions on how to execute the test
cases.

<br>

## File structure

The file structure closely resembles the hand-in file structure (`abgabe`) which is schematically depicted below:

```
compiler
│
└───abgabe (your files)
│   │
│   └───asma
│   |   │ ...
|   |
|   └───asmb
|   |   | ...
|   |
│   | ...
|
└───compilers-testsuite (this repo)
    │
    └───abgabe
        │
        └───asma
        |   │ ...
        |
        └───asmb
        |   | ...
        |
        | ...
```
<br>

## Contribute

If you want to add additional test suites, than please adhere to the following
conventions:

- **Commit messages**: Commit messages should begin with a `[` followed by the exercise name/a keyword describing what the commit is about followed by a closing `]`. Furthermore, one needs to provide details about the commit <br>
e.g. `[scanner] - included the test cases from the server`

- **Assignment code**: Do not include your code for the assignment. (e.g. the `scanner.c` file in the scanner exercise) Moreover, do not include code which might go against the rules of the lecture.

- **Add build howto as readme**: Add a readme to your testsuite, where you describe how one may build and execute your test cases. This file should be located on the same level as your test suite, e.g.: `abgabe/scanner/README.md` describes how one can build and execute the test cases for the scanner. You can additionaly add a `run.sh` file that handles everything automatically. (see scanner as a reference)
