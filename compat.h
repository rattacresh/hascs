#ifndef COMPAT_H
#define COMPAT_H
#include <ctype.h>
#include <string.h>
#define CAP(c) toupper(c)
#define HIGH(s) 1024 /* greater than any string */
#define TRUE 1
#define FALSE 0
#define INSet(a,b) strchr(b,a)
#endif
