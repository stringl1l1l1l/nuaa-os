#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2
#define BUF_SIZE 512

int main(int argc, char* argv[])
{
    int fd = -1;
    if (argc == 1)
        fd = STD_IN;
    else if (argc == 2)
        fd = open(argv[1], O_RDONLY);
    else {
        puts("Illegal command arguments");
        exit(EXIT_FAILURE);
    }
    if (fd < 0) {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    char buffer[BUF_SIZE] = { 0 };
    int count = -1;
    while (1) {
        count = read(fd, buffer, sizeof(buffer));
        if (count == 0)
            break;
        write(STD_OUT, buffer, count * sizeof(char));
    }
    return 0;
}