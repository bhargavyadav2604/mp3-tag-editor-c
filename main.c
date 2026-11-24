#include <stdio.h>
#include <string.h>
#include "header.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage:\n");
        printf("./a.out -v\n");
        printf("./a.out -e -t \"Title\"\n");
        printf("./a.out -e -a \"Artist\"\n");
        printf("./a.out -e -A \"Album\"\n");
        printf("./a.out -e -g \"Genre\"\n");
        printf("./a.out -e -y \"Year\"\n");
        printf("./a.out -e -c \"comment\"\n");
        return 0;
    }

    if (strcmp(argv[1], "-v") == 0)
    {
        view();
    }
    else if (strcmp(argv[1], "-e") == 0)
    {
        if (argc < 4)
        {
            printf("Invalid arguments\n");
            return 0;
        }

        edit(argv);
    }
    else
    {
        printf("Invalid option\n");
    }

    return 0;
}