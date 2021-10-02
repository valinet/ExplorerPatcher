/*-
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Novell nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "fmemopen.h"

FILE*
fmemopen(void* buf, size_t size, const char* mode)
{
    char temppath[MAX_PATH + 1];
    char tempnam[MAX_PATH + 1];
    DWORD l;
    HANDLE fh;
    FILE* fp;

    if (strcmp(mode, "r") != 0 && strcmp(mode, "r+") != 0)
        return 0;
    l = GetTempPathA(MAX_PATH, temppath);
    if (!l || l >= MAX_PATH)
        return 0;
    if (!GetTempFileNameA(temppath, "solvtmp", 0, tempnam))
        return 0;
    fh = CreateFileA(tempnam, DELETE | GENERIC_READ | GENERIC_WRITE, 0,
        NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
        NULL);
    if (fh == INVALID_HANDLE_VALUE)
        return 0;
    fp = _fdopen(_open_osfhandle((intptr_t)fh, 0), "w+b");
    if (!fp)
    {
        CloseHandle(fh);
        return 0;
    }
    if (buf && size && fwrite(buf, size, 1, fp) != 1)
    {
        fclose(fp);
        return 0;
    }
    rewind(fp);
    return fp;
}