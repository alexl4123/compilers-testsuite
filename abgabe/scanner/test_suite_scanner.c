#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * This function is used for forking, creating pipes and shutting down the scanner again.
 *
 * @input - The first string is the ''input'', so this is given to the scanner program.
 * @output - The second string is the ''should-output'', so what the response of the scanner, given the input should be (can be left blank in case of an error test case)
 * @should_fail - If set to one it is tested if the scanner program terminates with exit code 1
 */
int main_test_loop(char * input, char * output, int should_fail);

/**
 * This function is used for input-output-checking.
 */
int test_loop(int parent_child_pipe, int child_parent_pipe, char * input, char * output, int should_fail);

/**
 * Main is just for test cases -> How to include test cases?
 * Look below to see some examples, you can basically just copy paste and then edit the two strings.
 */
int main(int argc, char * argv[]) {
    int test_cases_executed = 0;
    int test_cases_successful = 0;
    
    {
        // Test 1:
	test_cases_successful += main_test_loop("test", "id test\n",0);
	test_cases_executed += 1;
    }
    {
        // Test 2:
	test_cases_successful += main_test_loop("39end end39", "num 39\nend\nid end39\n",0);
	test_cases_executed += 1;
    }
    {
        // Test 3:
	test_cases_successful += main_test_loop("// kommentar", "",0);
	test_cases_executed += 1;
    }
    {
        // Test 4:
	test_cases_successful += main_test_loop(";(){},:=[]+*>-@end return goto if var not and", ";\n(\n)\n{\n}\n,\n:\n=\n[\n]\n+\n*\n>\n-\n@\nend\nreturn\ngoto\nif\nvar\nnot\nand\n",0);
	test_cases_executed += 1;
    }
    {
        // Test 5:
	test_cases_successful += main_test_loop("endreturngotoifvarnotand", "id endreturngotoifvarnotand\n",0);
	test_cases_executed += 1;
    }
    {
        // Test 6:
	test_cases_successful += main_test_loop("$10 10", "num 16\nnum 10\n",0);
	test_cases_executed += 1;
    }
    {
        // Test 7:
	test_cases_successful += main_test_loop("%", "",1);
	test_cases_executed += 1;
    }
    
    fprintf(stdout, "Total test cases executed: %d, successful: %d.\n",
		    test_cases_executed,
		    test_cases_successful);
}

int main_test_loop(char * input, char * output, int should_fail) {

    int parent_child_pipe[2], child_parent_pipe[2];
    int ret = pipe(parent_child_pipe);
    if (ret < 0) {
        fprintf(stderr, "Cannot create parent_child_pipe! \n");
    }
    ret = pipe(child_parent_pipe);
    if (ret < 0) {
        fprintf(stderr, "Cannot create child_parent_pipe! \n");
    }   

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

            execlp("scanner", "scanner", NULL);
            
            break;
        default:
            // parent tasks
            close(parent_child_pipe[0]); // close read end
            close(child_parent_pipe[1]); // close write end

            ret = test_loop(parent_child_pipe[1], child_parent_pipe[0], input, output, should_fail);           

	    if (should_fail == 0) {
		if (kill(pid, SIGTERM) < 0) {
	            fprintf(stderr, "sigterm failed! \n");
                }
	    } else {
	        int status;
		pid_t cur_pid;
		while ((cur_pid = wait(&status)) != pid) {
	            if (cur_pid != -1) continue;
		    if (errno == EINTR) continue;
		    fprintf(stderr, "Wait failed!\n");
		    break;
		}
		if (cur_pid != -1 && WEXITSTATUS(status) == 1) {
		    ret = 0;
		}

	    }
	               


	    if (ret == 0) {
                fprintf(stdout, "Test case successful! \n");
		return 1;
	    } else if (ret < 0) {
	        fprintf(stdout, "[ERROR] - Some error occured while executing this test-case! \n");
		return 0;
	    } else {
                fprintf(stdout, "Input-output doesn't match! \n");
		return 0;
	    }

            break;
    }
}    

int test_loop(int parent_child_fd, int child_parent_fd, char * input, char * output, int should_fail) {

    int ret_state = 0;
    fprintf(stdout, "reached test_loop state. \n");
    dprintf(parent_child_fd, "%s", input);
    
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

    fprintf(stdout, "input: <%s>, should-output: <%s>, is-output: <%s>\n", input, output, line);
    
    int len_should = strlen(output);
    int len_is = strlen(line);
    if (len_should != len_is) {
        ret_state = 1;
    }
    if (strcmp(output, line) != 0) {
	ret_state = 1;
    }

    int retcp = close(child_parent_fd);
    if (retcp != 0) {
        fprintf(stderr, "failed to close file-stream! \n");
	ret_state = -1;
    }
 
    return ret_state;
}
