/*
 * Copyright (c) 2017  Joachim Nilsson <troglobit@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "fmemopen.h"

FILE* fmemopen(void* buf, size_t len, const char* type)
{
	int fd;
	FILE* fp;
	char tp[MAX_PATH - 13];
	char fn[MAX_PATH + 1];

	if (!GetTempPathA(sizeof(tp), tp))
		return NULL;

	if (!GetTempFileNameA(tp, "eptmp", 0, fn))
		return NULL;

	_sopen_s(&fd, fn,
		_O_CREAT | _O_RDWR | _O_SHORT_LIVED | _O_TEMPORARY | _O_BINARY,
		_SH_DENYRW,
		_S_IREAD | _S_IWRITE);
	if (fd == -1)
		return NULL;

	fp = _fdopen(fd, "w+");
	if (!fp) {
		_close(fd);
		return NULL;
	}

	fwrite(buf, len, 1, fp);
	rewind(fp);

	return fp;
}