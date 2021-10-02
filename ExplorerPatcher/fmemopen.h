#ifndef _H_FMEMOPEN_H_
#define _H_FMEMOPEN_H_
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
FILE* fmemopen(void* buf, size_t size, const char* mode);
#endif