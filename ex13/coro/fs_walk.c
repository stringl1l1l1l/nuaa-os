#include "fs_walk.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include "coro.h"

char *global_dir_path;

void find_dir(char *dir_path)
{
    DIR *dir = opendir(dir_path);
    struct dirent* entry;
    while (entry = readdir(dir)) {
        if (strcmp(entry->d_name, ".") == 0)
            continue;
        if (strcmp(entry->d_name, "..") == 0)
            continue;
            
        if (entry->d_type == DT_DIR) {
            char newPath[256];
            strcpy(newPath, dir_path);
            strcat(newPath, "/");
            strcat(newPath, entry->d_name);
        }

        if (entry->d_type == DT_REG) {
            char newPath[256];
            strcpy(newPath, dir_path);
            strcat(newPath, "/");
            strcat(newPath, entry->d_name);
            coro_yield((unsigned long) newPath);
        }
    }
    closedir(dir);
}

void fs_walk()
{
    find_dir(global_dir_path);
    coro_yield(0);
}
