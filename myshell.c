#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
// red : 31, green : 32, yellow : 33, light blue : 36
#define MAX_BUF 64
#define DO_MAX_BUF 64

// Get : Next Token
int getNextToken(char* command, char** argv) {
	int cmd_len = 0;
	int cur_offset = 0;

	for(cur_offset; cur_offset < strlen(command) && command[cur_offset] != '\0' && isspace(command[cur_offset]) == 0; cur_offset++, cmd_len++) {}

	// cmd_len + null space
	*argv = (char*)malloc(sizeof(char) * (cmd_len + 1));
	strncpy(*argv, command, cmd_len);

	return cur_offset;
}

// Internal Command : cat
// fail : -1, success : 1
int mycat(int argc, char* argv[]) {

	int fd, read_size, write_size;
	char buf[MAX_BUF];
	
	// Error : argument count
	if(argc != 2) {
		printf("%c[1;33m", 27);
		printf("USAGE: %s file_name\n", argv[0]);
		printf("%c[0m\n", 27);

		return -1;
	}

	// Error : file open
	fd = open(argv[1], O_RDONLY);
	if(fd < 0) {
		printf("%c[1;31m", 27);
		printf("ERROR: Can't open %s file with errno %d\n", argv[1], errno);
		printf("%c[0m\n", 27);

		return -1;
	}

	// Process : cat
	read_size = read(fd, buf, MAX_BUF);
	while(read_size == MAX_BUF) {
		write_size = write(STDOUT_FILENO, buf, read_size);
		read_size = read(fd, buf, MAX_BUF);
	}
	write_size = write(STDOUT_FILENO, buf, read_size);

	close(fd);

	return 1;

}

// Internal Command : cp
// fail : -1, success : 1
int mycp(int argc, char* argv[]) {

	int fd, fd2, read_size, write_size;
	char buf[MAX_BUF];

	// Error : argument count
	if(argc != 3) {
		printf("%c[1;33m", 27);
		printf("USAGE: %s file_name1 file_name2\n", argv[0]);
		printf("%c[0m\n", 27);

		return -1;
	}

	// Error : file open
	fd = open(argv[1], O_RDONLY);
	if(fd < 0) {
		printf("%c[1;31m", 27);
		printf("ERROR: Can't open %s file with errno %d\n", argv[1], errno);
		printf("%c[0m\n", 27);
		return -1;
	}
	fd2 = open(argv[2], O_CREAT | O_WRONLY, 0664);
	if(fd2 < 0) {
		printf("%c[1;31m", 27);
		printf("ERROR: Can't open %s file with errno %d\n", argv[2], errno);
		printf("%c[0m\n", 27);
		return -1;
	}

	// Process : copy
	read_size = read(fd, buf, MAX_BUF);
	while(read_size == MAX_BUF) {
		write_size = write(fd2, buf, read_size);
		read_size = read(fd, buf, MAX_BUF);
	}
	write_size = write(fd2, buf, read_size);

	close(fd);
	close(fd2);
	
	return 1;
}

// Execute : Command Line -> used execl()
int do_command(int argc, char* argv[]) {
	
	pid_t pid, d_pid;
	int exit_status;
	char path[100];
	sprintf(path, "/bin/%s", argv[0]);

	if((pid = fork()) == -1) {
		// fork error
		printf("%c[1;31m", 27);
		printf("ERROR: Can't fork process");
		printf("%c[0m\n", 27);

		return -1;
	}
	else if(pid == 0) {
		// execl("/bin/ls", "ls", "-l", (char*)0);
		execl(path, argv[0], argv[1], (char*)0);
		// if execl not execute well, will execute below statements..
		printf("%c[1;36m", 27);
		printf("SORRY: This command is not supported");
		printf("%c[0m\n", 27);

		//return -1;
		exit(1);
	}
	else {
		//printf("pid = %d\n", pid);
		d_pid = wait(&exit_status);
		printf("exit status of task %d is %d\n", d_pid, exit_status);
	}
	
	return 1;
}

int main(void) {
    char prompt[] = "mysh> ";
    char* command;
	int read_size;
	int cmd_offset;
	int cur_argc;
	int offset_size;
	int argc;
	char* argv[10];
	int result;

    // Preprocess : output shell description
	char shell_desc[] = "Welcome to MyShell! :)\nmax argument : 10\nmax command length : 64\n";
    write(STDOUT_FILENO, shell_desc, strlen(shell_desc));

    while(1) {
        // Output : Shell Prompt
        write(STDOUT_FILENO, prompt, strlen(prompt));

        // Read : Command Line from Standard Input
		command = (char*)malloc(sizeof(char)*DO_MAX_BUF);
		read_size = read(0, command, DO_MAX_BUF);
        if(read_size <= 0) {
            printf("Have a nice day^^\n");
            exit(0);
        }

        // Divide : Command Token
        cmd_offset = 0;
        cur_argc = 0;

		offset_size = getNextToken(&command[cmd_offset], &argv[cur_argc]);
        while(offset_size > 0) {
            cmd_offset += offset_size;
			// include space at offset!
			cmd_offset++;
			cur_argc++;
			
			offset_size = getNextToken(&command[cmd_offset], &argv[cur_argc]);
        }
		argc = cur_argc;

        // Process : Internal Command
        // Internal Command 1 : cat
        if(strcmp(argv[0], "cat") == 0) {
			result = mycat(argc, argv);
			continue;
        }
        
        // Internal Command 2 : cp
        if(strcmp(argv[0], "cp") == 0) {
			result = mycp(argc, argv);
			continue;
        }

		// Process : Shell Exit
        if(strcmp(argv[0], "exit") == 0) {
            printf("Have a nice day^^\n");
            exit(0);
        }
		
		
        // Process : Pipe, I/O redirection, Background, Multi-command Handling

        // Process : Shell Script

        // Process : External Command
        result = do_command(argc, argv);   // fork() and execve(), slide 13p
    }

}
