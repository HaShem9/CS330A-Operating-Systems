#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>


// Checking if pattern_string exists in target_string
int check_pattern(char const* pattern_string, int pattern_length, char* target_string, int target_length){
    for(int i=0; i<=(target_length-pattern_length); i++){
        int flag=1;
        for(int j=i;j<(i+pattern_length);j++){
            if(pattern_string[j-i] != target_string[j]){
                flag=0;
                break;
            }
        }
        if(flag==1){
            return 1;
        }
    }

    return 0;
}


// Search for pattern in regular file
void search_pattern_in_regular_file(char const *pattern_string, int pattern_length, char *file_path, int out_fd, int file_flag){
    char buf[2048], line[2048];
    int count = 0;
    int fd = open(file_path, O_RDONLY);
    int line_length=0;

    // Reading a fixed sized buffer from file 
    while( (count = read(fd, buf, sizeof(buf))) > 0 ){
        for(int i=0;i<count;i++){
            if(buf[i]=='\n'){
                line[line_length]='\n';
                line_length++;
                line[line_length]='\0';

                // Checking each line (terminated by \n) for pattern
                if(check_pattern(pattern_string, pattern_length, line, line_length)){
                    if(file_flag){
                        write(out_fd, line, line_length);   // No need to print file name in front
                    }
                    else{
                        char output[2408];
                        snprintf(output, sizeof(output), "%s:%s", file_path, line);   // Concatenating file_path and line
                        write(out_fd, output, line_length+1+strlen(file_path));
                    }
                }
                line_length=0;  // As now a new line would start
            }
            else{
                line[line_length]=buf[i];
                line_length++;
            }
        }
    }
    close(fd);
}


// Search for the "pattern_string" in given "file_path" recursively and output the results to file pointed by "out_fd"
// file_flag is to check if the original path was a file or a directory
void search_pattern_recursively(char const *pattern_string, int pattern_length, char *file_path, int out_fd, int file_flag){
    struct stat buf;    
    if(stat(file_path, &buf) < 0){
        fprintf(stderr,"File doesn't exist\n");
        return;
    }

    if(S_ISDIR(buf.st_mode)){
        // Directory
        DIR *dir;
        struct dirent *entry;

        if (!(dir = opendir(file_path)))
            return;

        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char new_path[1024];
            snprintf(new_path, sizeof(new_path), "%s/%s", file_path, entry->d_name);
            search_pattern_recursively(pattern_string, pattern_length, new_path, out_fd, 0);
        }

        closedir(dir);
    }
    else if(S_ISREG(buf.st_mode)) {
        // Regular file
        search_pattern_in_regular_file(pattern_string, pattern_length, file_path, out_fd, file_flag);
    }

    return;
}


// Main function - program entry point
int main(int argc, char const *argv[]){
    if(argc < 3){
        fprintf(stderr, "Invalid input.\n");
        fprintf(stderr, "Usage: ./mygrep pattern_string dir_name \n");
        exit(0);
    }

    char file_path[1024];
    char const *pattern_string = argv[1];
    strcpy(file_path, argv[2]);
    int pattern_length = strlen(pattern_string);

    // Removing extra "/" characters on right
    int i=0;
    for(i=0;file_path[i]!='\0';i++);
    i--;
    while(file_path[i]=='/'){
        i--;
    }
    file_path[i+1]='\0';

    search_pattern_recursively(pattern_string, pattern_length, file_path, 1, 1);
    return 0;
}