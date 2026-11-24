#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"

void c_e1(unsigned char *ptr)
{
    for (int i = 0; i < 2; i++)
    {
        unsigned char tmp = ptr[i];
        ptr[i] = ptr[3 - i];
        ptr[3 - i] = tmp;
    }
}

void view(void)
{
    FILE *fpt = fopen("sample.mp3", "rb");

    if (fpt == NULL)
    {
        printf("Can't access file sample.mp3!\n");
        return;
    }

    // Skip ID3 header
    fseek(fpt, 10, SEEK_SET);

    int found = 0;

    while (found < 7)
    {
        char tag[6] = {0};
        unsigned int size = 0;

        // Read tag
        for (int i = 0; i < 4; i++)
        {
            int ch = getc(fpt);

            if (ch == EOF)
            {
                fclose(fpt);
                return;
            }

            tag[i] = ch;
        }

        // ID3v2.3 frame size
        unsigned char size_bytes[4];

        for (int i = 0; i < 4; i++)
        {
            int ch = getc(fpt);

            if (ch == EOF)
            {
                fclose(fpt);
                return;
            }

            size_bytes[i] = (unsigned char)ch;
        }

        c_e1(size_bytes);

        memcpy(&size, size_bytes, sizeof(size));

        // Skip flags
        fseek(fpt, 2, SEEK_CUR);

        char *info = malloc(size + 1);

        if (info == NULL)
        {
            fclose(fpt);
            return;
        }

        for (unsigned int i = 0; i < size; i++)
        {
            info[i] = getc(fpt);
        }

        info[size] = '\0';

        if (strcmp(tag, "TIT2") == 0)
        {
            printf("Title  : %s\n", info + 1);
            found++;
        }
        else if (strcmp(tag, "TYER") == 0)
        {
            printf("Year   : %s\n", info + 1);
            found++;
        }
        else if (strcmp(tag, "TALB") == 0)
        {
            printf("Album  : %s\n", info + 1);
            found++;
        }
        else if (strcmp(tag, "TPE1") == 0)
        {
            printf("Artist : %s\n", info + 1);
            found++;
        }
        else if (strcmp(tag, "TCON") == 0)
        {
            printf("Genre  : %s\n", info + 1);
            found++;
        }
        else if (strcmp(tag, "COMM") == 0)
        {
            printf("Comment: %s\n", info + 5);
            found++;
        }

        free(info);
    }

    fclose(fpt);
}