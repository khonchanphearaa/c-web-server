#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/* String helpers */
void str_trim(char *s);
char *str_dup(const char *s);
int str_starts_with(const char *s, const char *prefix);

void url_decode(const char *src, char *dst, size_t dst_len);

char *file_read(const char *path, size_t *out_len);

int file_exists(const char *path);

#endif 