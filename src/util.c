#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *
safe_strdup(const char *s) {
    size_t len = strlen(s);
    char *ret = malloc(len + 1);
    if (ret == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(ret, s);
    return ret;
}
