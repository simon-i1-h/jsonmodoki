#define _GNU_SOURCE

#include "jsonmodoki.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void *
xmalloc(size_t size)
{
	void *ret = malloc(size);
	if (ret == NULL) {
		logmsg("malloc failed.\n");
		exit(1);
	}

	return ret;
}

/*
 * 常に有効なアドレスを返すので、呼び出し元は一時変数を使わずに次のよ
 * うに呼び出せる。
 *
 *   ptr = xrealloc(ptr, newsize)
 *
 */
void *
xrealloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);
	if (ret == NULL) {
		logmsg("realloc failed\n");
		exit(1);
	}

	return ret;
}

int
xvasprintf(char **strp, const char *fmt, va_list ap)
{
	int ret = vasprintf(strp, fmt, ap);
	if (ret == -1) {
		logmsg("vasprintf failed.\n");
		exit(1);
	}
	return ret;
}

int
xasprintf(char **strp, const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = xvasprintf(strp, fmt, ap);
	va_end(ap);
	return ret;
}

void
vlogmsg2(const char *progfile, const char *pos, const char *fmt, va_list ap)
{
	if (progfile != NULL)
		fprintf(stderr, "%s: ", progfile);
	if (pos != NULL)
		fprintf(stderr, "%s: ", pos);

	vfprintf(stderr, fmt, ap);
}

void
logmsg2(const char *progfile, const char *pos, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vlogmsg2(progfile, pos, fmt, ap);
	va_end(ap);
}

int
vstrprintf(string_t *dst, const char *fmt, va_list ap)
{
	int len;
	char *tmp;
	len = xvasprintf(&tmp, fmt, ap);
	string_add_string(dst, tmp);
	free(tmp);
	return len;
}

int
strprintf(string_t *dst, const char *fmt, ...)
{
	va_list ap;
	int len;
	va_start(ap, fmt);
	len = vstrprintf(dst, fmt, ap);
	va_end(ap);
	return len;
}
