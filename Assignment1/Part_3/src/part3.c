/* 
Name     : Vaibhav Thakkar
Roll No. : 170778

Idea: 

In the main function we first create a pipe between fd[0] and fd[1]
with fd[0] as read end and fd[1] as write end for pipe.

Then we traverse on the path passed as argument, where if we encounter a non 
directory we just increment the ans with size of the non directory file,

If we encounter a directory
-> Fork a process,
-> In child,
-----> Calculate result for that directory in child process using get_directory_size_without_fork
       function,
-----> Push the sum as integer on pipe using write fxn on fd[1],
-----> Print the sum with Directory name on stdout,
-----> Close fd[0] and fd[1] for child,
-----> Exit from child process
-> In Parent,
-----> Increment the child_count variable

Finally after this in the parent process, read from fd[0] - child_count number of times
adding the readed integer value to ans.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


// Function to return size of a folder recursively without forking any processes
int get_directory_size_without_fork(char* dir_path){
    DIR *dir;
    struct dirent *entry;
    size_t ans = 0;

    if (!(dir = opendir(dir_path)))
        return 0;
    
    // Traversing the current dir_path
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char new_path[1024];
        snprintf(new_path, sizeof(new_path), "%s/%s", dir_path, entry->d_name);
        
        // Getting stats of current file in dir_path
        struct stat buf;    
        if(stat(new_path, &buf) < 0){
            fprintf(stderr,"File doesn't exist\n");
            continue;
        }

        if(S_ISDIR(buf.st_mode)){
            ans += get_directory_size_without_fork(new_path);      // Recursively getting directory size for sub-directories
        } else{
            ans += buf.st_size;
        }
    }

    closedir(dir);
    return ans;
}


// Main function - program entry point
int main(int argc, char const* argv[]){
    if(argc < 2){
        fprintf(stderr, "Invalid input.\n");
        fprintf(stderr, "Usage: ./part3 dir_name \n");
        exit(0);
    }
    struct stat buf;    
    if(stat(argv[1], &buf) < 0){
        fprintf(stderr,"File doesn't exist\n");
        return 0;
    }

    if(S_ISDIR(buf.st_mode)){
        // Creating pipe in parent process
        int fd[2], child_count=0;
        if(pipe(fd) < 0)
            perror("pipe");            

        // Opening and allocating memory for directory traversal
        DIR *dir;
        struct dirent *entry;
        size_t ans = 0;
        if (!(dir = opendir(argv[1]))){
            perror("Error in opening directory");
            exit(-1);
        }

        // Traversing the current dir_path
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char new_path[1024];
            snprintf(new_path, sizeof(new_path), "%s/%s", argv[1], entry->d_name);
            
            // Getting stats of current file in dir_path
            struct stat buf;    
            if(stat(new_path, &buf) < 0){
                fprintf(stderr,"File doesn't exist\n");
                continue;
            }

            if(S_ISDIR(buf.st_mode)){
                pid_t pid = fork();
                if(pid == 0){   // Child
                    ans = get_directory_size_without_fork(new_path);      // Recursively getting directory size for sub-directories
                    write(fd[1], &ans, sizeof(ans));    // child pushes the answer to pipe
                    printf("%s %d\n", entry->d_name, ans);  // child prints it ans to stdout
                    close(fd[0]);
                    close(fd[1]);
                    exit(0);    // child's work finished
                }
                else{
                    child_count++;
                }
            } else{
                ans += buf.st_size;
            }
        }
        closedir(dir);

        for(int i=0; i<child_count; i++){
            int temp;
            read(fd[0],&temp,sizeof(temp));  // parent fetches answers from pipe
            ans += temp;
        }

        printf("%s %d\n", argv[1], ans);

    } else{
        size_t ans = buf.st_size;
        printf("%s %d\n", argv[1], ans);
    }
    return 0;
}