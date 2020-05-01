#ifndef UTIL_H
#define UTIL_H

char *
safe_strdup(const char *s);

#define strdup(s) safe_strdup(s)

#endif//UTIL_H
