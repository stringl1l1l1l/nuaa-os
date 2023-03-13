#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    mode_t mode = 0777;
    if (argc != 2 && argc != 3)
        puts("Illegal command.");

    char *source = argv[1], *target = argv[2];
    int file_s = open(source, O_RDONLY);
    int file_t = creat(target, mode);
    if (file_s == -1)
        printf("%s: %s: No such file or directory.", argv[0], argv[1]);
    else {
        char ch;
        int flag1 = read(file_s, &ch, sizeof(ch));
        int flag2 = 1;
        while (flag1 && flag2) {
            flag2 = write(file_t, &ch, sizeof(ch));
            flag1 = read(file_s, &ch, sizeof(ch));
        }
        if (flag1 == -1) {
            puts("Fail to read file.");
        }
        if (flag2 == -1) {
            puts("Fail to write file.");
        }
    }
    close(file_s);
    close(file_t);
    return 0;
}