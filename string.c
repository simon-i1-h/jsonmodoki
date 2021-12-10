#include "jsonmodoki.h"
#include <stdio.h>
#include <string.h>

string_t
string_new(void)
{
	size_t capacity = 16;
	char *bytes = xmalloc(capacity);
	bytes[0] = '\0';

	return (string_t){.bytes = bytes, .len = 0, .capacity = capacity};
}

void
string_add_char(string_t *s, int c)
{
	if (s->len + 1 >= s->capacity) {
		s->capacity *= 2;
		s->bytes = xrealloc(s->bytes, s->capacity);
	}

	s->bytes[s->len++] = c;
	s->bytes[s->len] = '\0';
}

void
string_add_string(string_t *s, const char *str)
{
	size_t len = strlen(str);

	for (size_t i = 0; i < len; i++)
		string_add_char(s, str[i]);
}
