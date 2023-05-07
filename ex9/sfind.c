/**
 * @file sfind.c
 * @author 陈立文
 * @brief 串行查找，在当前文件或当前目录中的所有文件中递归地找到指定的字符串所在行
 * @date 2023-05-07
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

void find_file(char *path, char *target)
{
    FILE *file = fopen(path, "r");
    if(file == NULL)
    {
        perror(path);
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, target))
            printf("%s: %s", path, line);
    }

    fclose(file);
}

void find_dir(char *path, char *target)
{
    DIR *dir = opendir(path);
    if(dir == NULL)
    {
        perror(path);
        return;
    }
    struct dirent *entry;
    // readdir用于读取目录所有项，成功读取则返回下一个入口点，否则返回NULL
    while (entry = readdir(dir)) {
        if (strcmp(entry->d_name, ".") == 0)
            continue;

        if (strcmp(entry->d_name, "..") == 0)
            continue;

        if (entry->d_type == DT_DIR) {
            // printf("dir  %s\n", entry->d_name);

            char newPath[256] = {0};
            strcpy(newPath, path);
            strcat(newPath, "/");
            strcat(newPath, entry->d_name);
            find_dir(newPath, target);
        }

        if (entry->d_type == DT_REG) {
            // printf("file %s\n", entry->d_name);

            char newPath[256] = {0};
            strcpy(newPath, path);
            strcat(newPath, "/");
            strcat(newPath, entry->d_name);
            find_file(newPath, target);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        puts("Usage: sfind file string");
        return 0;
    }

    char *path = argv[1];
    char *string = argv[2];

    struct stat info;
    stat(path, &info);

    if (S_ISDIR(info.st_mode))
        find_dir(path, string);
    else
        find_file(path, string);

    return 0;
}
