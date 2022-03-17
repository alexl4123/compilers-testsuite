#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <signal.h>

int main_test_loop(char * input, char * output);
int test_loop(int parent_child_pipe, int child_parent_pipe, char * input, char * output);

int main(int argc, char * argv[]) {
    int test_cases_executed = 0;
    int test_cases_successful = 0;
    
    {
        // Test 1:
	test_cases_successful += main_test_loop("test", "id test\n");
	test_cases_executed += 1;
    }
    {
        // Test 2:
	test_cases_successful += main_test_loop("39end", "num 39\nend\n");
	test_cases_executed += 1;
    }
    {
        // Test 3:
	test_cases_successful += main_test_loop("// kommentar", "");
	test_cases_executed += 1;
    }
    {
        // Test 4:
	test_cases_successful += main_test_loop(";(){},:=[]+*>-@", ";\n(\n)\n{\n}\n,\n:\n=\n[\n]\n+\n*\n>\n-\n@\n");
	test_cases_executed += 1;
    }







    fprintf(stdout, "Total test cases executed: %d, successful: %d.\n",
		    test_cases_executed,
		    test_cases_successful);
}

int main_test_loop(char * input, char * output) {

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

            ret = test_loop(parent_child_pipe[1], child_parent_pipe[0], input, output);           

	    if (kill(pid, SIGTERM) < 0) {
	        fprintf(stderr, "sigterm failed! \n");
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

int test_loop(int parent_child_fd, int child_parent_fd, char * input, char * output) {

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
