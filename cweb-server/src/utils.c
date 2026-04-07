#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/utils.h"


/* String Helpers */
void str_trim(char *s)
{
    if (!s)
        return;
    /* Trim leading */
    char *start = s;
    while (*start && isspace((unsigned char)*start))
        start++;
    if (start != s)
        memmove(s, start, strlen(start) + 1);
    /* Trim trailing */
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end))
        *end-- = '\0';
}

char *str_dup(const char *s)
{
    if (!s)
        return NULL;
    size_t len = strlen(s);
    char *copy = malloc(len + 1);
    if (copy)
        memcpy(copy, s, len + 1);
    return copy;
}

int str_starts_with(const char *s, const char *prefix)
{
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

/* URL Decode */
static int hex_val(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0;
}

void url_decode(const char *src, char *dst, size_t dst_len)
{
    size_t i = 0;
    while (*src && i < dst_len - 1)
    {
        if (*src == '%' && *(src + 1) && *(src + 2))
        {
            dst[i++] = (char)(hex_val(*(src + 1)) * 16 + hex_val(*(src + 2)));
            src += 3;
        }
        else if (*src == '+')
        {
            dst[i++] = ' ';
            src++;
        }
        else
        {
            dst[i++] = *src++;
        }
    }
    dst[i] = '\0';
}

/* File Helpers */
char *file_read(const char *path, size_t *out_len)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size <= 0)
    {
        fclose(f);
        return NULL;
    }

    char *buf = malloc((size_t)size + 1);
    if (!buf)
    {
        fclose(f);
        return NULL;
    }

    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);

    if (out_len)
        *out_len = (size_t)size;
    return buf;
}

int file_exists(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f)
    {
        fclose(f);
        return 1;
    }
    return 0;
}