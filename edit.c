#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "header.h"

void c_e(unsigned char *ptr)
{
    for (int i = 0; i < 2; i++)
    {
        unsigned char tmp = ptr[i];
        ptr[i] = ptr[3 - i];
        ptr[3 - i] = tmp;
    }
}

void edit(char *argv[])
{
    FILE *src;
    FILE *temp;

    char target[6] = {0};

    /* Select frame */

    if (strcmp(argv[2], "-t") == 0)
    {
        strcpy(target, "TIT2");
    }
    else if (strcmp(argv[2], "-a") == 0)
    {
        strcpy(target, "TPE1");
    }
    else if (strcmp(argv[2], "-A") == 0)
    {
        strcpy(target, "TALB");
    }
    else if (strcmp(argv[2], "-g") == 0)
    {
        strcpy(target, "TCON");
    }
    else if (strcmp(argv[2], "-y") == 0)
    {
        strcpy(target, "TYER");
    }
    else if (strcmp(argv[2], "-c") == 0)
    {
        strcpy(target, "COMM");
    }
    else
    {
        printf("Invalid tag option\n");
        return;
    }

    /* Open source */

    src = fopen("sample.mp3", "rb");

    if (src == NULL)
    {
        printf("Can't access sample.mp3\n");
        return;
    }

    /* Create temporary file */

    temp = fopen("temp.mp3", "wb");

    if (temp == NULL)
    {
        printf("Can't create temporary file\n");
        fclose(src);
        return;
    }

    /* Read ID3 header */

    unsigned char header[10];

    if (fread(header, 1, 10, src) != 10)
    {
        printf("Invalid MP3 file\n");

        fclose(src);
        fclose(temp);

        remove("temp.mp3");

        return;
    }

    /* Check ID3 */

    if (header[0] != 'I' ||
        header[1] != 'D' ||
        header[2] != '3')
    {
        printf("No ID3 tag found\n");

        fclose(src);
        fclose(temp);

        remove("temp.mp3");

        return;
    }

    /* Check version */

    if (header[3] != 3)
    {
        printf("Only ID3v2.3 is supported\n");

        fclose(src);
        fclose(temp);

        remove("temp.mp3");

        return;
    }

    /* Get ID3 tag size */

    unsigned int tag_size;

    tag_size = ((unsigned int)(header[6] & 0x7F) << 21) |((unsigned int)(header[7] & 0x7F) << 14) |((unsigned int)(header[8] & 0x7F) << 7) |(header[9] & 0x7F);

    /* Copy header */

    fwrite(header, 1, 10, temp);

    unsigned int bytes_read = 0;
    int found = 0;

    /* Read frames only inside ID3 area */

    while (bytes_read < tag_size)
    {
        char tag[6] = {0};

        unsigned char size_bytes[4];

        unsigned int size;

        /* Read tag */

        if (fread(tag, 1, 4, src) != 4)
            break;

        bytes_read += 4;

        /* Padding */

        if (tag[0] == 0)
        {
            fwrite(tag, 1, 4, temp);

            while (bytes_read < tag_size)
            {
                int ch = fgetc(src);

                if (ch == EOF)
                    break;

                fputc(ch, temp);

                bytes_read++;
            }

            break;
        }

        /* Read size */

        if (fread(size_bytes, 1, 4, src) != 4)
            break;

        bytes_read += 4;

        /*
           Keep original size_bytes unchanged.
           converted is used only for calculating size.
        */

        unsigned char converted[4];

        memcpy(converted, size_bytes, 4);

        c_e(converted);

        memcpy(&size, converted, sizeof(size));

        /* Read flags */

        unsigned char flags[2];

        if (fread(flags, 1, 2, src) != 2)
            break;

        bytes_read += 2;

        /* Check size */

        if (size > tag_size - bytes_read)
            break;

        /* Read frame data */

        unsigned char *data = NULL;

        if (size > 0)
        {
            data = malloc(size);

            if (data == NULL)
            {
                printf("Memory allocation failed\n");

                fclose(src);
                fclose(temp);

                remove("temp.mp3");

                return;
            }

            if (fread(data, 1, size, src) != size)
            {
                free(data);

                fclose(src);
                fclose(temp);

                remove("temp.mp3");

                return;
            }
        }

        bytes_read += size;

        /* Target frame */

        if (strcmp(tag, target) == 0 && found == 0)
        {
            printf("Found tag: %s\n", tag);

            unsigned int text_length = strlen(argv[3]);

            unsigned int new_size;

            /* COMM frame */

            if (strcmp(target, "COMM") == 0)
            {
                new_size = 1 + 3 + 1 + text_length;

                /* Write tag */

                fwrite(tag, 1, 4, temp);

                /* Convert new size */

                unsigned char new_size_bytes[4];

                memcpy(new_size_bytes, &new_size, 4);

                c_e(new_size_bytes);

                /* Write new size */

                fwrite(new_size_bytes, 1, 4, temp);

                /* Write flags */

                fwrite(flags, 1, 2, temp);

                /* Encoding */

                fputc(0, temp);

                /* Language */

                fwrite("eng", 1, 3, temp);

                /* Empty description */

                fputc(0, temp);

                /* New comment */

                fwrite(argv[3], 1, text_length, temp);
            }
            else
            {
                /* Normal text frame */

                new_size = text_length + 1;

                /* Write tag */

                fwrite(tag, 1, 4, temp);

                /* Convert new size */

                unsigned char new_size_bytes[4];

                memcpy(new_size_bytes, &new_size, 4);

                c_e(new_size_bytes);

                /* Write new size */

                fwrite(new_size_bytes, 1, 4, temp);

                /* Write flags */

                fwrite(flags, 1, 2, temp);

                /* Encoding */

                fputc(0, temp);

                /* New text */

                fwrite(argv[3], 1, text_length, temp);
            }

            found = 1;
        }
        else
        {
            /* Copy original frame */

            fwrite(tag, 1, 4, temp);

            fwrite(size_bytes, 1, 4, temp);

            fwrite(flags, 1, 2, temp);

            if (size > 0)
            {
                fwrite(data, 1, size, temp);
            }
        }

        free(data);
    }

    /* Copy remaining MP3 audio data */

    int ch;

    while ((ch = fgetc(src)) != EOF)
    {
        fputc(ch, temp);
    }

    fclose(src);
    fclose(temp);

    /* Tag not found */

    if (found == 0)
    {
        printf("Tag not found\n");

        remove("temp.mp3");

        return;
    }

    /* Replace original file */

    if (remove("sample.mp3") != 0)
    {
        printf("Unable to remove original file\n");
        remove("temp.mp3");
        return;
    }

    if (rename("temp.mp3", "sample.mp3") != 0)
    {
        printf("Unable to rename temporary file\n");
        return;
    }

    printf("Tag edited successfully!\n");
}