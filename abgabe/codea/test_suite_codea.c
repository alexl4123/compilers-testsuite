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
int main_test_loop(char * input_codea, char * input_testfile, int fail_case);

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

int main_test_loop(char * input_codea, char * input_testfile, int fail_case) {

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

            char * path_string = "../../../abgabe/codea/codea";
            execlp(path_string, "codea", NULL);

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

    ret = test_loop(parent_child_pipe[1], child_parent_pipe[0], input_codea, output);

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
        fprintf(stdout, "<<<<<TEST_CASE_RESULT: PASSED>>>>>>>>\n");
        return 1;
    } else if (ret == 0) {
        fprintf(stdout, "<<<<<TEST_CASE_RESULT: FAILED>>>>>>>>\n");
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

    fprintf(stdout, "Exit status: %d \n", ret_status);
    return ret_status;
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
    int test_cases_executed = 0;
    int test_cases_successful = 0;

    {
        // Test 1 (simple normal testcase):
        test_cases_successful += main_test_loop(
                "f(a) return a; end;", 
                "long a = f(5l);\nif(a != 5) {exit(1);}", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 2:
        test_cases_successful += main_test_loop(
                "f(a) return 6; end;", 
                "long a = f(5l);\nif(a != 6) {exit(1);}", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 3:
        test_cases_successful += main_test_loop(
                "f(a) return a+6; end;", 
                "long a = f(5l);\nif(a != 11) {exit(1);}", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 4:
        test_cases_successful += main_test_loop(
                "f(a) return 4+a+6; end;", 
                "long a = f(5l);\nif(a != 15) {exit(1);}", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 5:
        test_cases_successful += main_test_loop(
                "f(a,b,c,d,e,f) return a+b+c+d+e+f; end;", 
                "long a = f(1l,2l,3l,4l,5l,6l);\nif(a != 21) {exit(1);}", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 6:
        test_cases_successful += main_test_loop(
                "f(a,b,c,d,e,f) return (a+b+c+d+e+f)*5; end;", 
                "long a = f(1l,2l,3l,4l,5l,6l);\nif(a != 105) {exit(1);}", SHOULD_WORK);
        test_cases_executed += 1;
    }
    {
        // Test 7:
        test_cases_successful += main_test_loop(
                "f(a) return a; end; g(b) return b; end;", 
                "long a = f(5l) + g(6l);\nif(a != 11) {exit(1);}", SHOULD_WORK);
        test_cases_executed += 1;
    }





    fprintf(stdout, "Total test cases executed: %d, successful: %d.\n",
            test_cases_executed,
            test_cases_successful);

    return 0;
}
