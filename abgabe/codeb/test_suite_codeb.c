#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <time.h>

struct String {
    int size;
    char * value;
} String;

int test_cases_executed;

enum fail_state {SHOULD_WORK = 0, SCANNER_SHOULD_FAIL = 1, PARSER_SHOULD_FAIL = 2, SEMANTIC_SHOULD_FAIL = 3};

/**
 * This function is used for forking, creating pipes and shutting down the scanner again.
 *  
 * TODO - add new description
 *
 * @output - The second string is the ''should-output'', so what the response of the scanner, given the input should be (can be left blank in case of an error test case)
 * @should_fail - If set to one it is tested if the scanner program terminates with exit code 1
 * @return - 1 if test case successful, 0 otherwise
 */
int main_test_loop(char * input_codeb, char * input_testfile, int fail_case);

/**
 * This function is used for input-output-checking.
 */
int test_loop(int parent_child_pipe, int child_parent_pipe, char * input, struct String * output);

/**
 * Main is just for test cases -> How to include test cases?
 * Look below to see some examples, you can basically just copy paste and then edit the two strings.
 */
int execute_test_cases();

int createFile(char * path, char * content);


int compile(int type);


//----------------------------------------------------------------------------------------------
/**
 * Just calls execute_test_cases();
 */
int main(int argc, char * argv[]) {
    execute_test_cases();
}

int main_test_loop(char * input_codeb, char * input_testfile, int fail_case) {
    test_cases_executed++;

    int parent_child_pipe[2], child_parent_pipe[2];
    int ret = pipe(parent_child_pipe);
    if (ret < 0) {
        fprintf(stderr, "Cannot create parent_child_pipe! \n");
    }
    ret = pipe(child_parent_pipe);
    if (ret < 0) {
        fprintf(stderr, "Cannot create child_parent_pipe! \n");
    }   

    fprintf(stdout, "\n<<<<<test_case: %d; started>>>>>>\n", test_cases_executed);
    fflush(stdout);
    pid_t pid = fork();

    switch(pid) {
        case -1:
            fprintf(stderr, "Cannot fork!\n");
            exit(EXIT_FAILURE);
        case 0:
            // child -> start parser
            close(parent_child_pipe[1]); // close write end
            close(child_parent_pipe[0]); // close read end

            dup2(parent_child_pipe[0], STDIN_FILENO);
            dup2(child_parent_pipe[1], STDOUT_FILENO);

            char * path_string = "../../../abgabe/codeb/codeb";
            execlp(path_string, "codeb", NULL);

            fprintf(stderr, "execlp failed with path: %s\n", path_string);
            exit(-1);
        default:
            break;
    }
    // parent tasks
    close(parent_child_pipe[0]); // close read end
    close(child_parent_pipe[1]); // close write end

    struct String * output = malloc(sizeof(int) + sizeof(char*));
    output->size = 4096;
    output->value = malloc(sizeof(char) * output->size);
    memset(output->value, 0, output->size);

    ret = test_loop(parent_child_pipe[1], child_parent_pipe[0], input_codeb, output);

    int status;
    pid_t cur_pid;

    while ((cur_pid = wait(&status)) != pid) {
        if (cur_pid != -1) continue;
        if (errno == EINTR) continue;
        fprintf(stderr, "Wait failed!\n");
        break;
    }
    fprintf(stdout, "Fail case: %d, exit status: %d \n",fail_case, WEXITSTATUS(status));
    if (cur_pid != -1 && WEXITSTATUS(status) == fail_case) {
        ret = 1;
    } else {
        ret = 0;
    }

    if (ret == 1 && WEXITSTATUS(status) != SHOULD_WORK) {
        fprintf(stdout, "<<<<<TEST_CASE: %d; RESULT: PASSED>>>>>>>>\n", test_cases_executed);
        return 1;
    } else if (ret == 0) {
        fprintf(stdout, "<<<<<TEST_CASE: %d; RESULT: FAILED>>>>>>>>\n", test_cases_executed);
        return 0;
    }

    // ------------------------ CONTINUE - IF - PARSER - WORKED ---------------------

    char * testfile = malloc(sizeof(char) * ((strlen(input_testfile) + 1) + 1000));
    sprintf(testfile, "#include <stdlib.h>\n#include <string.h>\n#include <stdlib.h>\n#include <stdio.h>\n\nint main(int argc, char ** argv) {\n%s\n}", input_testfile);
    createFile("test.c", testfile);
    createFile("test.s", output->value);

    ret = compile(0);
    if (ret < 0) {
        fprintf(stdout, "<<<<<TEST_CASE_RESULT: FAILED>>>>>>>>\n");
        return 0;
    }
    ret = compile(1);
    if (ret < 0) {
        fprintf(stdout, "<<<<<TEST_CASE_RESULT: FAILED>>>>>>>>\n");
        return 0;
    }
    ret = compile(2);
    if (ret < 0) {
        fprintf(stdout, "<<<<<TEST_CASE_RESULT: FAILED>>>>>>>>\n");
        return 0;
    }
    ret = compile(3);
    if (ret != 0) {
        fprintf(stdout, "<<<<<TEST_CASE_RESULT: FAILED>>>>>>>>\n");
        return 0;
    } else {
        fprintf(stdout, "<<<<<TEST_CASE_RESULT: PASSED>>>>>>>>\n");
        return 1;
    }

}    

int compile(int type) {

    pid_t pid = fork();

    switch(pid) {
        case -1:
            fprintf(stderr, "Cannot fork!\n");
            exit(EXIT_FAILURE);
        case 0:
            if (type == 0) {
                execlp("gcc", "gcc", "-o", "test.o", "-c", "test.c", NULL);
            } else if (type == 1) {
                execlp("gcc", "gcc", "-o", "test_asm.o", "-c", "test.s", NULL);
            } else if (type == 2) {
                execlp("gcc", "gcc", "-o", "test", "test.o", "test_asm.o", NULL);
            } else if (type == 3) {
                execlp("test", "test", NULL);
            }

            exit(-1);
        default:
            break;
    }
    // parent tasks
    int status;
    pid_t cur_pid;

    while ((cur_pid = wait(&status)) != pid) {
        if (cur_pid != -1) continue;
        if (errno == EINTR) continue;
        fprintf(stderr, "Wait failed!\n");
        break;
    }

    int ret_status = WEXITSTATUS(status);
    int sig_status = WTERMSIG(status);

    fprintf(stdout, "Exit status: %d \n", ret_status);
    fprintf(stdout, "Signal status: %d \n", sig_status);
    if (ret_status == 0 && sig_status != 0) {
        return sig_status;
    } else {
        return ret_status;
    }
}



int createFile(char * path, char * content) {
    mode_t S_rwx = 0700;
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, S_rwx);

    if (fd < 0) {
        fprintf(stderr, "failed to open file %s\n", path);
        exit(EXIT_FAILURE);
    }

    dprintf(fd, "%s", content);
    fsync(fd);

    int retcp = close(fd);
    if (retcp != 0) {
        fprintf(stderr, "failed to close file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    return 0;
}

int test_loop(int parent_child_fd, int child_parent_fd, char * input, struct String * output) {

    int ret_state = 0;
    fprintf(stdout, "reached test_loop state. \n");
    dprintf(parent_child_fd, "%s\n", input);

    fsync(parent_child_fd);

    int retpc = close(parent_child_fd);
    if (retpc != 0) {
        fprintf(stderr, "failed to close file-descriptor! \n");
        ret_state = -1;
    }


    ssize_t ret = read(child_parent_fd, output->value, output->size);
    if (ret < 0) {
        fprintf(stderr, "error while reading from child! \n");
        ret_state = -1;
    }

    fprintf(stdout, "input: <%s>, is-output: <%s>\n", input, output->value);

    int retcp = close(child_parent_fd);
    if (retcp != 0) {
        fprintf(stderr, "failed to close file-stream! \n");
        ret_state = -1;
    }

    return ret_state;
}


int execute_test_cases() {
    test_cases_executed = 0;
    int test_cases_successful = 0;

    {
        // Test 1 (simple normal testcase):
        test_cases_successful += main_test_loop(
                "f(a) return a; end;", 
                "long a = f(5l);\nif(a != 5) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 2:
        test_cases_successful += main_test_loop(
                "f(a) return 6; end;", 
                "long a = f(5l);\nif(a != 6) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 3:
        test_cases_successful += main_test_loop(
                "f(a) return a+6; end;", 
                "long a = f(5l);\nif(a != 11) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 4:
        test_cases_successful += main_test_loop(
                "f(a) return 4+a+6; end;", 
                "long a = f(5l);\nif(a != 15) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 5:
        test_cases_successful += main_test_loop(
                "f(a,b,c,d,e,f) return a+b+c+d+e+f; end;", 
                "long a = f(1l,2l,3l,4l,5l,6l);\nif(a != 21) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 6:
        test_cases_successful += main_test_loop(
                "f(a,b,c,d,e,f) return (a+b+c+d+e+f)*5; end;", 
                "long a = f(1l,2l,3l,4l,5l,6l);\nif(a != 105) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 7:
        test_cases_successful += main_test_loop(
                "f(a) return a; end; g(b) return b; end;", 
                "long a = f(5l) + g(6l);\nif(a != 11) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 8:
        test_cases_successful += main_test_loop(
                "f(a) return a > 5; end;", 
                "long a = f(4);\nif(a != 0) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 9:
        test_cases_successful += main_test_loop(
                "f(a) return 7 > 5; end;", 
                "long a = f(4);\nif(a != 1) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 10:
        test_cases_successful += main_test_loop(
                "f(a) return 7 > 7; end;", 
                "long a = f(4);\nif(a != 0) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 11:
        test_cases_successful += main_test_loop(
                "f(a) return 7 = 7; end;", 
                "long a = f(4);\nif(a != 1) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 12:
        test_cases_successful += main_test_loop(
                "f(a) return 7 = 8; end;", 
                "long a = f(4);\nif(a != 0) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 13:
        test_cases_successful += main_test_loop(
                "f(a) return 6 = 7; end;", 
                "long a = f(4);\nif(a != 0) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 14:
        test_cases_successful += main_test_loop(
                "f(a) return 1 and 0; end;", 
                "long a = f(4);\nif(a != 0) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 15:
        test_cases_successful += main_test_loop(
                "f(a) return 1 and 3; end;", 
                "long a = f(4);\nif(a != 1) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 16:
        test_cases_successful += main_test_loop(
                "f(a) return 31 and 8; end;", 
                "long a = f(4);\nif(a != 8) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 17:
        test_cases_successful += main_test_loop(
                "f(a) return -a; end;", 
                "long a = f(4);\nif(a != -4) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 18:
        test_cases_successful += main_test_loop(
                "f(a) return 4 + (---a); end;", 
                "long a = f(4);\nif(a != 0) {exit(1);}", SHOULD_WORK);
    }
    {
        // Test 19:
        test_cases_successful += main_test_loop(
                "f(a) return not 0; end;", 
                "long a = f(4);\nif(a != -1) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 20:
        test_cases_successful += main_test_loop(
                "f(a) return a[0]; end;", 
                "long arr[1] = {3};\nlong a = f(arr);\nif(a != 3) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 21:
        test_cases_successful += main_test_loop(
                "f(a) return a[3]; end;", 
                "long arr[5] = {3,4,5,6,7};\nlong a = f(arr);\nif(a != 6) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 22:
        test_cases_successful += main_test_loop(
                "f{a,b}(c,d) return a; end;", 
                "struct f_stage1 {\n    long (*func)(long, long, struct f_stage1 *);\n    long a;\n    long b;\n};\nextern long f(long c, long d, struct f_stage1 *fs1);\nstruct f_stage1 f1 = {&f, 5, 6};\nlong test = f(7,8,&f1);\nif(test != 5) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 23:
        test_cases_successful += main_test_loop(
                "f{a,b}(c,d) return b; end;", 
                "struct f_stage1 {\n    long (*func)(long, long, struct f_stage1 *);\n    long a;\n    long b;\n};\nextern long f(long c, long d, struct f_stage1 *fs1);\nstruct f_stage1 f1 = {&f, 5, 6};\nlong test = f(7,8,&f1);\nif(test != 6) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 24:
        test_cases_successful += main_test_loop(
                "f{a,b}(c,d) return c; end;", 
                "struct f_stage1 {\n    long (*func)(long, long, struct f_stage1 *);\n    long a;\n    long b;\n};\nextern long f(long c, long d, struct f_stage1 *fs1);\nstruct f_stage1 f1 = {&f, 5, 6};\nlong test = f(7,8,&f1);\nif(test != 7) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 25:
        test_cases_successful += main_test_loop(
                "f{a,b}(c,d) return d; end;", 
                "struct f_stage1 {\n    long (*func)(long, long, struct f_stage1 *);\n    long a;\n    long b;\n};\nextern long f(long c, long d, struct f_stage1 *fs1);\nstruct f_stage1 f1 = {&f, 5, 6};\nlong test = f(7,8,&f1);\nif(test != 8) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 26:
        test_cases_successful += main_test_loop("f(x,x) var hallo = 5; end; f(x) var hallo = 5; end; ", "", SEMANTIC_SHOULD_FAIL);
    }
    {
        // Test 27:
        test_cases_successful += main_test_loop("f(x,y,x) var hallo = 5; end; f(x) var hallo = 5; end; ", "", SEMANTIC_SHOULD_FAIL);
    }
    {
        // Test 28 :
        test_cases_successful += main_test_loop("f(x) hallo: var hallo = 5; end;", "", SEMANTIC_SHOULD_FAIL);
    }
    {
        // Test 29:
        test_cases_successful += main_test_loop("f(x) hallo = 7; var hallo = 5; end;", "", SEMANTIC_SHOULD_FAIL);
    }
    {
        // Test 30:
        test_cases_successful += main_test_loop("f(x,y,) end;", "", PARSER_SHOULD_FAIL);
    }
    {
        // Test 31:
        test_cases_successful += main_test_loop("f(x) var x = 5; end;", "", SEMANTIC_SHOULD_FAIL);
    }
    {
        // Test 32:
        test_cases_successful += main_test_loop("f(x) x: return 5; end;", "", SEMANTIC_SHOULD_FAIL);
    }
    {
        // Test 33:
        test_cases_successful += main_test_loop("f(x) var hallo = hallo; end;", "", SEMANTIC_SHOULD_FAIL);
    }
    {
        // Test 34:
        test_cases_successful += main_test_loop("\n <%&\n", "", SCANNER_SHOULD_FAIL);
    }
    {
        // Test 35:
        test_cases_successful += main_test_loop("f() end;", "", PARSER_SHOULD_FAIL);
    }
    {
        // Test 36:
        test_cases_successful += main_test_loop("f(x,y) hallo : hallo : hallo : \$10; end;", "", SEMANTIC_SHOULD_FAIL);
    }
    {
        // Test 37:
        test_cases_successful += main_test_loop("hallo", "", PARSER_SHOULD_FAIL);
    }
    {
        // Test 38:
        test_cases_successful += main_test_loop(
                "f(x,y,z) return x; end;", 
                "long a = f(5,6,7);\nif(a != 5) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 39:
        test_cases_successful += main_test_loop(
                "f(x,y,z) return z; end;", 
                "long a = f(5,6,7);\nif(a != 7) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 40:
        test_cases_successful += main_test_loop(
                "f(a) return g@(x); end;", 
                "long a = f(5,6,7);\nif(a != 7) { exit(1);}", SEMANTIC_SHOULD_FAIL);
    }
    // ------------------ONLY-CODEB-TESTS
    {
        // Test 41:
        test_cases_successful += main_test_loop(
                "f(x) goto test; return 4; test: return 5; end;", 
                "long a = f(0);\nif(a != 5) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 42:
        test_cases_successful += main_test_loop(
                "f(x) if (3) goto test; return 4; test: return 5; end;", 
                "long a = f(0);\nif(a != 5) { exit(1);}", SHOULD_WORK);
    }    
    {
        // Test 43:
        test_cases_successful += main_test_loop(
                "f(x) if (2) goto test; return 4; test: return 5; end;", 
                "long a = f(0);\nif(a != 4) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 44:
        test_cases_successful += main_test_loop(
                "f(x) var y = 5; return y; end;", 
                "long a = f(0);\nif(a != 5) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 45:
        test_cases_successful += main_test_loop(
                "f(x) if (2 = x) goto test; return 4; test: var y = 6; var z = 7; return (x + y + z); end;", 
                "long a = f(2);\nif(a != 15) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 46:
        test_cases_successful += main_test_loop(
                "f(x) x = 5; return x; end;", 
                "long a = f(2);\nif(a != 5) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 47:
        test_cases_successful += main_test_loop(
                "f(x) x[1] = 5; return x; end;", 
                "long test[5] = {0,0,0,0,0};\nlong a = f(test);\nif(test[1] != 5) { exit(1);}", SHOULD_WORK);
    }
    {
        // Test 48:
        test_cases_successful += main_test_loop(
                "f(x) if (x = 5) goto test; return 1; test: return 0; end; g(x) if (x = 5) goto test; return 0; test: return 1; end;", 
                "long a = f(5);\nif(a != 0) { exit(1);}\nlong b = g(6);\nif(b != 0) { exit(1);}\n", SHOULD_WORK);
    }
    {
        // Test 49:
        // Initial idea for this (and the stage_1-test-cases) test case is from: https://github.com/flofriday/UEB-Testsuite
        test_cases_successful += main_test_loop(
                "f(x) return g{x}; end; g(x) return x; end;", 
                "struct f_stage1 { long (*func)(long); long a; }; extern long g(long); extern long* f(long); long * fs = f(1); struct f_stage1 fc = { (long (*)(long)) fs, *(fs + 1)}; if(fc.a == 1 && fc.func(1) == 1) { exit(0);} exit(1);\n", SHOULD_WORK);
    }
    {
        // Test 50:
        test_cases_successful += main_test_loop(
                "f(x,y,z) return g{x,y,z}; end; g(x) return x; end;", 
                "struct f_stage1 { long (*func)(long); long a; long b; long c; }; extern long* f(long,long,long); long * fs = f(1,2,3); struct f_stage1 fc = { (long (*) (long)) fs, *(fs + 1), *(fs + 2), *(fs + 3)}; if(fc.a == 1 && fc.b == 2 && fc.c == 3 && fc.func(1) == 1) { exit(0);} exit(1);\n", SHOULD_WORK);
    }

    fprintf(stdout, "Total test cases executed: %d, successful: %d.\n",
            test_cases_executed,
            test_cases_successful);

    return 0;
}
