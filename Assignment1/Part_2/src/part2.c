#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Main function - program entry point
int main(int argc, char const *argv[]){
    if(argc < 4){
        fprintf(stderr, "Invalid input.\n");
        fprintf(stderr, "Usage: ./part2 operator pattern_string dir_name [file] [command] \n");
        exit(0);
    }

    char *pattern_string = (char*)argv[2];
    char *file_path      = (char*)argv[3];

    if(!(strcmp(argv[1], "@"))){
        // Implementing @ operator
        int fd[2];
        if(pipe(fd) < 0)
            perror("pipe");

        pid_t pid = fork();
        if(pid == 0){   // Child will run wc -l
            dup2(fd[0], 0);     // as wc -l will take it's input from pipe
            close(fd[0]);
            close(fd[1]);
            char* arguments[] = {"wc", "-l", NULL};
            execvp("wc", arguments);
        }
        else{  // Parent will run grep command
            dup2(fd[1], 1);         // as grep -rF will push it's output to pipe
            close(fd[1]);
            close(fd[0]);
            char* arguments[] = {"grep", "-rF", pattern_string, file_path, (char*)NULL} ;
            execvp("grep", arguments);
        }
    }
    else if(!(strcmp(argv[1], "$"))){
        // Implementing $ operator
        int fd[2];
        if(pipe(fd) < 0)
            perror("pipe");

        pid_t pid = fork();
        if(pid != 0){   // Parent will run grep command
            dup2(fd[1], 1);     // as grep -rF will push it's output to pipe
            close(fd[0]);
            close(fd[1]);
            char* arguments[] = {"grep", "-rF", pattern_string, file_path, (char*)NULL} ;
            execvp("grep", arguments);
        }
        else{   // Child will store result to file as well send it to pipe for next command
            close(fd[1]);   // as child will read from pipe
            int file_fd = open(argv[4], O_RDWR);
            char buf[2048];
            int count;
            int new_pipe_fd[2]; // for another pipe for transmitting ouput to new command
            if(pipe(new_pipe_fd) < 0)
                perror("second pipe error");

            // Reading from pipe then sending to file passed as argument
            while( (count = read(fd[0], &buf, sizeof(buf)-1)) > 0 ){
                write(file_fd, buf, count);         // writing to file passed 
                write(new_pipe_fd[1], buf, count);  // writing to new pipe for next command
            }
            
            dup2(new_pipe_fd[0], 0);    // As now the next command passed as argument will take input from the file
            close(fd[0]);
            close(new_pipe_fd[0]);
            close(new_pipe_fd[1]);

            if(argc == 7){
                char* arguments[] = {(char*)argv[5], (char*)argv[6], (char*)NULL} ;
                execvp(argv[5], arguments);
            }
            else if(argc == 6){
                char* arguments[] = {(char*)argv[5], (char*)NULL} ;
                execvp(argv[5], arguments);
            }
        }
    }
    
    printf("Only @ and $ operators are allowed\n");
    return 0;
}