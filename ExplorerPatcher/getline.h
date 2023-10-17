#ifndef _H_GETLINE_H_
#define _H_GETLINE_H_
#include <stdio.h>
#include <stdlib.h>
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;

#ifdef __cplusplus
extern "C" {
#endif

ssize_t getdelim(char** buf, size_t* bufsiz, int delimiter, FILE* fp);

ssize_t getline(char** buf, size_t* bufsiz, FILE* fp);

#ifdef __cplusplus
}
#endif

#endif