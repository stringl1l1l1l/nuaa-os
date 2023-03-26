#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUF_SIZE 512

int main(int argc, char* argv[])
{
    if (argc != 2 && argc != 3) {
        puts("Illegal command arguments");
        exit(EXIT_FAILURE);
    }

    char* source = argv[1];
    char* target = argv[2];
    int fd_s = open(source, O_RDONLY);
    // 0b100读 0b010写 0b001执行   拥有者、群组、其他组
    int fd_t = creat(target, 0666);
    if (fd_s == -1) {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }
    if (fd_t == -1) {
        perror(argv[2]);
        exit(EXIT_FAILURE);
    }

    char buffer[BUF_SIZE];
    int count_read = 0;
    while (1) {
        count_read = read(fd_s, buffer, sizeof(buffer));
        if (count_read == 0)
            break;
        write(fd_t, buffer, count_read * sizeof(char));
    }
    if (count_read == -1) {
        puts("Fail to read file.");
        exit(EXIT_FAILURE);
    }

    close(fd_s);
    close(fd_t);
    return 0;
}