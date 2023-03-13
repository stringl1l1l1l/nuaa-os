#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUF_SIZE 1024
int main(int argc, char* argv[])
{
    if (argc != 2)
        puts("Illegal command.");
    char* filePath = argv[1];
    int file = open(filePath, O_RDONLY);
    if (file == -1)
        printf("%s: %s: No such file or directory.", argv[0], argv[1]);
    else {
        char* buffer = (char*)malloc(BUF_SIZE);
        int flag = read(file, buffer, BUF_SIZE);
        while (flag > 0) {
            fputs(buffer, stdout);
            flag = read(file, buffer, BUF_SIZE);
        }
        if (flag == -1)
            puts("Fail to read file.");
    }
    close(file);
    return 0;
}