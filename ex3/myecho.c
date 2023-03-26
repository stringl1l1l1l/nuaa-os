#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

int main(int argc, char* argv[])
{
    char* str = malloc(sizeof(char) * 30 * argc);
    for (int i = 1; i < argc; i++) {
        strcat(str, argv[i]);
        if (i != argc - 1)
            strcat(str, " ");
        else
            strcat(str, "\n");
    }

    write(STD_OUT, str, sizeof(char) * strlen(str));
    return 0;
}