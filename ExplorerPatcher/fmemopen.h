#ifndef _H_FMEMOPEN_H_
#define _H_FMEMOPEN_H_
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <sys/stat.h>
FILE* fmemopen(void* buf, size_t len, const char* type);
#endif