#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <time.h>

enum fail_state {SHOULD_WORK = 0, SCANNER_SHOULD_FAIL = 1, PARSER_SHOULD_FAIL = 2, SEMANTIC_SHOULD_FAIL = 3};

/**
 * This function is used for forking, creating pipes and shutting down the scanner again.
 *
 * @input - The first string is the ''input'', so this is given to the scanner program.
 * @output - The second string is the ''should-output'', so what the response of the scanner, given the input should be (can be left blank in case of an error test case)
 * @should_fail - If set to one it is tested if the scanner program terminates with exit code 1
 * @return - 1 if test case successful, 0 otherwise
 */
int main_test_loop(char * input, int fail_case);

/**
 * This function is used for input-output-checking.
 */
int test_loop(int parent_child_pipe, int child_parent_pipe, char * input);

/**
 * Main is just for test cases -> How to include test cases?
 * Look below to see some examples, you can basically just copy paste and then edit the two strings.
 */
int execute_test_cases();

/**
 * Just calls execute_test_cases();
 */
int main(int argc, char * argv[]) {
    execute_test_cases();
}

int main_test_loop(char * input, int fail_case) {

    int parent_child_pipe[2], child_parent_pipe[2];
    int ret = pipe(parent_child_pipe);
    if (ret < 0) {
        fprintf(stderr, "Cannot create parent_child_pipe! \n");
    }
    ret = pipe(child_parent_pipe);
    if (ret < 0) {
        fprintf(stderr, "Cannot create child_parent_pipe! \n");
    }   

    fprintf(stdout, "\n<<<<<test_case_started>>>>>>\n");
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

            char * path_string = "../../../abgabe/ag/ag";
            execlp(path_string, "ag", NULL);

            fprintf(stderr, "execlp failed with path: %s\n", path_string);
            exit(-1);
        default:
            // parent tasks
            close(parent_child_pipe[0]); // close read end
            close(child_parent_pipe[1]); // close write end

            ret = test_loop(parent_child_pipe[1], child_parent_pipe[0], input);

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

            if (ret == 1) {
                fprintf(stdout, "<<<<<TEST_CASE_RESULT: PASSED>>>>>>>>\n");
                ret = 1;
            } else {
                fprintf(stdout, "<<<<<TEST_CASE_RESULT: FAILED>>>>>>>>\n");
                ret = 0;
            }
            return ret;

    }
}    

int test_loop(int parent_child_fd, int child_parent_fd, char * input) {

    int ret_state = 0;
    fprintf(stdout, "reached test_loop state. \n");
    dprintf(parent_child_fd, "%s\n", input);

    fsync(parent_child_fd);

    int retpc = close(parent_child_fd);
    if (retpc != 0) {
        fprintf(stderr, "failed to close file-descriptor! \n");
        ret_state = -1;
    }

    int size = 512;    
    char line[size];
    memset(line, 0, sizeof line);

    ssize_t ret = read(child_parent_fd, line, size);
    if (ret < 0) {
        fprintf(stderr, "error while reading from child! \n");
        ret_state = -1;
    }

    fprintf(stdout, "input: <%s>, is-output: <%s>\n", input, line);

    int retcp = close(child_parent_fd);
    if (retcp != 0) {
        fprintf(stderr, "failed to close file-stream! \n");
        ret_state = -1;
    }

    return ret_state;
}


int execute_test_cases() {
    int test_cases_executed = 0;
    int test_cases_successful = 0;

    {
        // Test 1 (simple normal testcase):
        test_cases_successful += main_test_loop("hallo (variable) end ;", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 2 (only id on file end should fail):
        test_cases_successful += main_test_loop("hallo", PARSER_SHOULD_FAIL);
        test_cases_executed += 1;
    }
    {
        // Test 3 (test 1 of assignment):
        test_cases_successful += main_test_loop("f(x) end;", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 4 (test 2 of assignment):
        test_cases_successful += main_test_loop("\n <%&\n", SCANNER_SHOULD_FAIL);
        test_cases_executed += 1;
    }
    {
        // Test 5 (test 3 of assignment):
        test_cases_successful += main_test_loop("f() end;", PARSER_SHOULD_FAIL);
        test_cases_executed += 1;
    }
    {
        // Test 6 (tests stats and single stat term):
        test_cases_successful += main_test_loop("f(x,y) hallo : hallo : hallo : $10; end;", SEMANTIC_SHOULD_FAIL);
        test_cases_executed += 1;
    }
    {
        // Test 7 (tests stat statements):
        test_cases_successful += main_test_loop("f(x,y) $10; 11; end;", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 8 (tests multiple stat statements):
        test_cases_successful += main_test_loop("f(x,y) hallo: return $10; goto hallo; if 1 goto hallo; var test = y; test = x; test[x] = 5; end;", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 9 (tests expr and multiple term):
        test_cases_successful += main_test_loop("f(x,y) return not - - not not $10; return 10 + 10 + 12; return 10 * 10 * 12; return 10 and 10 and 12; return 10 > 12; return 10 = 12;  end;", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 10 (tests multiple term):
        test_cases_successful += main_test_loop("f(hallo,a,b,c,d,e) return 10; return hallo[5]; return hallo(a,b,c); return hallo{a,b,c,d}; return hallo@(a,b,c,d,e);  end;", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 11 (higher order function):
        test_cases_successful += main_test_loop("f{x,y,z}(x,y) end;", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 12 (normal function fail):
        test_cases_successful += main_test_loop("f(x,y,) end;", PARSER_SHOULD_FAIL);
        test_cases_executed += 1;
    }
    {
        // Test 13 (special name scoping tests):
        test_cases_successful += main_test_loop("f(x) var x = 5; end;", SEMANTIC_SHOULD_FAIL);
        test_cases_executed += 1;
    }
    {
        // Test 14 (special name scoping tests):
        test_cases_successful += main_test_loop("f(x) x: return 5; end;", SEMANTIC_SHOULD_FAIL);
        test_cases_executed += 1;
    }
    {
        // Test 15 (special name scoping tests):
        test_cases_successful += main_test_loop("f(x) var hallo = hallo; end;", SEMANTIC_SHOULD_FAIL);
        test_cases_executed += 1;
    }
    {
        // Test 16 (special name scoping tests):
        test_cases_successful += main_test_loop("f(x) hallo: goto hallo; end;", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 17 (special name scoping tests):
        test_cases_successful += main_test_loop("f(x) goto hallo; hallo: return 5; end;", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 18 (special name scoping tests):
        test_cases_successful += main_test_loop("f(x) hallo = 7; var hallo = 5; end;", SEMANTIC_SHOULD_FAIL);
        test_cases_executed += 1;
    }
    {
        // Test 19 (special name scoping tests):
        test_cases_successful += main_test_loop("f(x) var hallo = 5; hallo = 7; end;", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 20 (special name scoping tests):
        test_cases_successful += main_test_loop("f(x) hallo: var hallo = 5; end;", SEMANTIC_SHOULD_FAIL);
        test_cases_executed += 1;
    }
    {
        // Test 21 (special name scoping tests):
        test_cases_successful += main_test_loop("f(x) var hallo = 5; end; f(x) var hallo = 5; end; ", SHOULD_WORK);
        test_cases_executed += 1;
    }


    fprintf(stdout, "Total test cases executed: %d, successful: %d.\n",
            test_cases_executed,
            test_cases_successful);

    return 0;
}
