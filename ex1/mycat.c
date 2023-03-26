#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUF_SIZE 512
int main(int argc, char* argv[])
{
    char buffer[BUF_SIZE] = { 0 };
    char* filePath = argv[1];
    int count = 0;
    int fd = 0;

    if (argc != 2) {
        puts("Illegal command arguments");
        exit(EXIT_FAILURE);
    }

    fd = open(filePath, O_RDONLY);
    if (fd < 0) {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    while (1) {
        count = read(fd, buffer, sizeof(buffer));
        if (count == 0)
            break;
        write(1, buffer, count * sizeof(char));
    }

    close(fd);
    return 0;
}