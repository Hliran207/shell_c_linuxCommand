#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH 1024

void print_tree(const char *path, int level)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (!dir)
    {
        perror("open dir");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        for (int i = 0; i < level; i++)
        {
            printf("|   ");
        }
        printf("├── %s\n", entry->d_name);

        // build full path
        char fullpath[MAX_PATH];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat item_stat;
        if (stat(fullpath, &item_stat) == 0 && S_ISDIR(item_stat.st_mode))
        {
            print_tree(fullpath, level + 1);
        }
    }
    closedir(dir);
}
int main(int argc, char *argv[])
{
    const char *start_path = (argc > 1) ? argv[1] : ".";
    printf("%s\n", start_path);
    print_tree(start_path, 0);
    return 0;
}