#include "jsonmodoki.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/*
 * file
 */

file_t
file_new_with_string(char *str)
{
	return (file_t){.tag = FILE_TAG_STRING,
	    .file = NULL,
	    .str = str,
	    .str_len = strlen(str),
	    .str_index = 0,
	    .buf = {0},
	    .buf_len = 0};
}

/*
 * return: unsigned charとしての文字かEOF
 */
int
file_read(file_t *f)
{
	if (f->buf_len > 0) {
		BUG(f->buf_len > array_len(f->buf));
		f->ordinal++;
		return f->buf[--f->buf_len];
	}

	int c;

	switch (f->tag) {
	case FILE_TAG_FILE:
		c = fgetc(f->file);
		break;
	case FILE_TAG_STRING:
		BUG(f->str_len < f->str_index);

		if (f->str_len == f->str_index)
			c = EOF;
		else
			c = (unsigned char)f->str[f->str_index++];
		break;
	}

	f->ordinal++;

	return c;
}

void
file_unread(file_t *f, int c)
{
	BUG(f->buf_len >= array_len(f->buf));
	BUG(f->ordinal == 0);
	f->buf[f->buf_len++] = c;
	f->ordinal--;
}

/*
 * lexer
 */

#define case_whitespace \
	case ' ': \
	case '\t': \
	case '\n': \
	case '\r'

#define case_digit_1_to_9 \
	case '1': \
	case '2': \
	case '3': \
	case '4': \
	case '5': \
	case '6': \
	case '7': \
	case '8': \
	case '9'

#define case_digit \
	case_digit_1_to_9: \
	case '0'

#define case_end_value \
	case_whitespace: \
	case ',': \
	case ']': \
	case '}': \
	case ':': \
	case EOF

void
lexer_set_general_error(lexer_t *l)
{
	l->error = (error_t){ERROR_GENERAL, l->file.ordinal};
}

/*
 * args: e: unsigned charとしての文字かEOF
 */
#define lexer_expected(l, expected) \
	do { \
		lexer_t *glue(RESERVED, l_) = (l); \
		int glue(RESERVED, expected_) = (expected); \
		int glue(RESERVED, actual) = \
		    file_read(&glue(RESERVED, l_)->file); \
		if (glue(RESERVED, actual) != glue(RESERVED, expected_)) { \
			logmsg( \
			    "lexer_expected: error: expected char: %c, " \
			    "actual char: %c\n", \
			    glue(RESERVED, expected_), \
			    glue(RESERVED, actual)); \
			lexer_set_general_error(glue(RESERVED, l_)); \
			return NULL; \
		} \
	} while (0)

#define lexer_peek_end_value(l) \
	do { \
		lexer_t *glue(RESERVED, l_) = (l); \
		int glue(RESERVED, actual) = \
		    file_read(&glue(RESERVED, l_)->file); \
		switch (glue(RESERVED, actual)) { \
		case_end_value: \
			break; \
		default: \
			logmsg( \
			    "lexer_peek_end_value: error: expected char: " \
			    "end of " \
			    "value, " \
			    "actual char: %c\n", \
			    glue(RESERVED, actual)); \
			lexer_set_general_error(glue(RESERVED, l_)); \
			return NULL; \
		} \
		file_unread( \
		    &glue(RESERVED, l_)->file, glue(RESERVED, actual)); \
	} while (0)

lexer_t
lexer_new_with_string(char *str)
{
	return (lexer_t){.tokbuf = string_new(),
	    .tokenhead = NULL,
	    .tokentail = NULL,
	    .error = (error_t){.kind = ERROR_GENERAL, .ordinal = 0},

	    .file = file_new_with_string(str),
	    .tokencurr = NULL,
	    .ordinal = 0,
	    .buf = {NULL},
	    .buf_len = 0};
}

void
lexer_add_token(lexer_t *l, token_t *tok)
{
	if (l->tokentail != NULL)
		l->tokentail->next = tok;
	else
		l->tokenhead = l->tokencurr = tok;
	l->tokentail = tok;
}

token_t *
token_new(token_t tok)
{
	token_t *ret = xmalloc(sizeof(token_t));
	*ret = tok;
	return ret;
}

token_t *
token_new_with_tag(size_t ordinal, enum token_tag tag)
{
	return token_new(
	    (token_t){.ordinal = ordinal, .tag = tag, .next = NULL});
}

token_t *
token_new_with_bool(size_t ordinal, int boolean)
{
	return token_new((token_t){.ordinal = ordinal,
	    .tag = TOKEN_TAG_BOOL,
	    .boolean = boolean,
	    .next = NULL});
}

token_t *
token_new_with_number(size_t ordinal, double number)
{
	return token_new((token_t){.ordinal = ordinal,
	    .tag = TOKEN_TAG_NUMBER,
	    .number = number,
	    .next = NULL});
}

token_t *
token_new_with_string(size_t ordinal, string_t string)
{
	return token_new((token_t){.ordinal = ordinal,
	    .tag = TOKEN_TAG_STRING,
	    .string = string,
	    .next = NULL});
}

token_t *
lexer_lex_null(lexer_t *l)
{
	lexer_expected(l, 'n');
	size_t ordinal = l->file.ordinal;
	lexer_expected(l, 'u');
	lexer_expected(l, 'l');
	lexer_expected(l, 'l');
	lexer_peek_end_value(l);
	return token_new_with_tag(ordinal, TOKEN_TAG_NULL);
}

token_t *
lexer_lex_true(lexer_t *l)
{
	lexer_expected(l, 't');
	size_t ordinal = l->file.ordinal;
	lexer_expected(l, 'r');
	lexer_expected(l, 'u');
	lexer_expected(l, 'e');
	lexer_peek_end_value(l);
	return token_new_with_bool(ordinal, 1);
}

token_t *
lexer_lex_false(lexer_t *l)
{
	lexer_expected(l, 'f');
	size_t ordinal = l->file.ordinal;
	lexer_expected(l, 'a');
	lexer_expected(l, 'l');
	lexer_expected(l, 's');
	lexer_expected(l, 'e');
	lexer_peek_end_value(l);
	return token_new_with_bool(ordinal, 0);
}

token_t *
lexer_lex_number(lexer_t *l)
{
	int c;
	size_t ordinal;

	/* for strtod */
	char *rest;
	double d;

	l->tokbuf = string_new();

	enum state {
		STATE_BEGIN,
		STATE_AFTER_MINUS,
		STATE_AFTER_ZERO,
		STATE_INT_DIGIT_REST,
		STATE_FRAC_DEGIT_FIRST,
		STATE_FRAC_DIGIT_REST,
		STATE_EXP_BEGIN,
		STATE_EXP_DIGIT_FIRST,
		STATE_EXP_DIGIT_REST
	} st = STATE_BEGIN;

	for (;;) {
		c = file_read(&l->file);

		switch (st) {
		case STATE_BEGIN: {
			ordinal = l->file.ordinal;

			switch (c) {
			case '-':
				string_add_char(&l->tokbuf, c);
				st = STATE_AFTER_MINUS;
				break;
			case '0':
				string_add_char(&l->tokbuf, c);
				st = STATE_AFTER_ZERO;
				break;
			case_digit_1_to_9:
				string_add_char(&l->tokbuf, c);
				st = STATE_INT_DIGIT_REST;
				break;
			default:
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_AFTER_MINUS: {
			switch (c) {
			case '0':
				string_add_char(&l->tokbuf, c);
				st = STATE_AFTER_ZERO;
				break;
			case_digit_1_to_9:
				string_add_char(&l->tokbuf, c);
				st = STATE_INT_DIGIT_REST;
				break;
			default:
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_AFTER_ZERO: {
			switch (c) {
			case '.':
				string_add_char(&l->tokbuf, c);
				st = STATE_FRAC_DEGIT_FIRST;
				break;
			case 'e':
			case 'E':
				string_add_char(&l->tokbuf, c);
				st = STATE_EXP_BEGIN;
				break;
			case_end_value:
				file_unread(&l->file, c);
				goto parse;
			default:
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_INT_DIGIT_REST: {
			switch (c) {
			case_digit:
				string_add_char(&l->tokbuf, c);
				break;
			case '.':
				string_add_char(&l->tokbuf, c);
				st = STATE_FRAC_DEGIT_FIRST;
				break;
			case 'e':
			case 'E':
				string_add_char(&l->tokbuf, c);
				st = STATE_EXP_BEGIN;
				break;
			case_end_value:
				file_unread(&l->file, c);
				goto parse;
			default:
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_FRAC_DEGIT_FIRST: {
			switch (c) {
			case_digit:
				string_add_char(&l->tokbuf, c);
				st = STATE_FRAC_DIGIT_REST;
				break;
			default:
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_FRAC_DIGIT_REST: {
			switch (c) {
			case_digit:
				string_add_char(&l->tokbuf, c);
				break;
			case 'e':
			case 'E':
				string_add_char(&l->tokbuf, c);
				st = STATE_EXP_BEGIN;
				break;
			case_end_value:
				file_unread(&l->file, c);
				goto parse;
			default:
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_EXP_BEGIN: {
			switch (c) {
			case '+':
			case '-':
				string_add_char(&l->tokbuf, c);
				st = STATE_EXP_DIGIT_FIRST;
				break;
			case_digit:
				string_add_char(&l->tokbuf, c);
				st = STATE_EXP_DIGIT_REST;
				break;
			default:
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_EXP_DIGIT_FIRST: {
			switch (c) {
			case_digit:
				string_add_char(&l->tokbuf, c);
				st = STATE_EXP_DIGIT_REST;
				break;
			default:
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_EXP_DIGIT_REST: {
			switch (c) {
			case_digit:
				string_add_char(&l->tokbuf, c);
				break;
			case_end_value:
				file_unread(&l->file, c);
				goto parse;
			default:
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		}
	}

parse:
	errno = 0;
	d = strtod(l->tokbuf.bytes, &rest);
	if (errno != 0 || l->tokbuf.bytes == rest || *rest != '\0') {
		logmsg("strtod failed.\n");
		lexer_set_general_error(l);
		return NULL;
	}

	return token_new_with_number(ordinal, d);
}

int
is_high_surrogate(uint16_t utf16)
{
	return utf16 >= 0xd800 && utf16 <= 0xdbff;
}

int
is_low_surrogate(uint16_t utf16)
{
	return utf16 >= 0xdc00 && utf16 <= 0xdfff;
}

int
utf16_to_utf32(const uint16_t *pair, uint32_t *codepoint)
{
	if (!is_high_surrogate(pair[0])) {
		*codepoint = pair[0];
		return 1;
	}

	if (!is_low_surrogate(pair[1])) {
		return -1;
	}

	*codepoint = 0;
	*codepoint |= (pair[0] & 0x3ff) << 10;
	*codepoint |= pair[1] & 0x3ff;
	*codepoint += 0x10000;
	return 2;
}

int
utf32_to_utf8(uint32_t codepoint, char *bytes)
{
	if (codepoint <= 0x7f) {
		bytes[0] = codepoint;
		return 1;
	} else if (codepoint >= 0x80 && codepoint <= 0x7ff) {
		bytes[0] = 0xc0 | (codepoint >> 6);
		bytes[1] = 0x80 | (codepoint & 0x3f);
		return 2;
	} else if (codepoint >= 0x800 && codepoint <= 0xffff) {
		if (is_high_surrogate(codepoint) ||
		    is_low_surrogate(codepoint))
			return -1;
		bytes[0] = 0xe0 | (codepoint >> 12);
		bytes[1] = 0x80 | ((codepoint >> 6) & 0x3f);
		bytes[2] = 0x80 | (codepoint & 0x3f);
		return 3;
	} else if (codepoint >= 0x10000 && codepoint <= 0x10ffff) {
		bytes[0] = 0xf0 | (codepoint >> 18);
		bytes[1] = 0x80 | ((codepoint >> 12) & 0x3f);
		bytes[2] = 0x80 | ((codepoint >> 6) & 0x3f);
		bytes[3] = 0x80 | (codepoint & 0x3f);
		return 4;
	}

	return -1;
}

/*
 * Usage:
 *
 *   int len;
 *   uint16_t pair[2] = {utf16_high, utf16_low};
 *   char bytes[4];
 *   len = utf16_to_utf8(pair, bytes);
 */
int
utf16_to_utf8(const uint16_t *pair, char *bytes)
{
	uint32_t codepoint;

	if (utf16_to_utf32(pair, &codepoint) == -1)
		return -1;

	return utf32_to_utf8(codepoint, bytes);
}

int
hexctoi(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return -1;
}

token_t *
lexer_lex_string(lexer_t *l)
{
	int c;
	size_t ordinal;
	l->tokbuf = string_new();

	lexer_expected(l, '"');
	ordinal = l->file.ordinal;

	enum state {
		STATE_NORMAL,
		STATE_ESCAPE,
		STATE_ESCAPE_UNICODE_1,
		STATE_ESCAPE_UNICODE_2,
		STATE_ESCAPE_UNICODE_3,
		STATE_ESCAPE_UNICODE_4,
		STATE_SURROGATE_NORMAL,
		STATE_SURROGATE_ESCAPE,
		STATE_SURROGATE_ESCAPE_UNICODE_1,
		STATE_SURROGATE_ESCAPE_UNICODE_2,
		STATE_SURROGATE_ESCAPE_UNICODE_3,
		STATE_SURROGATE_ESCAPE_UNICODE_4
	} st = STATE_NORMAL;

	uint16_t utf16_high = 0;
	uint16_t utf16_low = 0;

	for (;;) {
		c = file_read(&l->file);
		if (c == EOF) {
			logmsg("unexpected EOF\n");
			lexer_set_general_error(l);
			return NULL;
		}

		switch (st) {
		case STATE_NORMAL: {
			if (c == (unsigned char)'\x20' ||
			    c == (unsigned char)'\x21' ||
			    (c >= (unsigned char)'\x23' &&
			        c <= (unsigned char)'\x5b') ||
			    (c >= (unsigned char)'\x5d' &&
			        c <= (unsigned char)'\xbf') ||
			    (c >= (unsigned char)'\xc2' &&
			        c <= (unsigned char)'\xf4')) {
				/*
				 * signed charの正の値より大きいオクテットを範
				 * 囲比較に使うため、文字リテラルをunsigned
				 * charに キャストしている。
				 *
				 * 0x20: ' '
				 * 0x21: '!'
				 * ---
				 * 0x22: '"'
				 * 0x5c: '\\'
				 * 0xc0, 0xc1, 0xf5 ~ 0xff: UTF-8's invalid
				 * octet
				 */

				string_add_char(&l->tokbuf, c);
			} else if (c == '\\') {
				st = STATE_ESCAPE;
			} else if (c == '"') {
				goto finish;
			} else {
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_ESCAPE: {
			switch (c) {
			case '"':
				string_add_char(&l->tokbuf, '"');
				st = STATE_NORMAL;
				break;
			case '\\':
				string_add_char(&l->tokbuf, '\\');
				st = STATE_NORMAL;
				break;
			case '/':
				string_add_char(&l->tokbuf, '/');
				st = STATE_NORMAL;
				break;
			case 'b':
				string_add_char(&l->tokbuf, '\b');
				st = STATE_NORMAL;
				break;
			case 'f':
				string_add_char(&l->tokbuf, '\f');
				st = STATE_NORMAL;
				break;
			case 'n':
				string_add_char(&l->tokbuf, '\n');
				st = STATE_NORMAL;
				break;
			case 'r':
				string_add_char(&l->tokbuf, '\r');
				st = STATE_NORMAL;
				break;
			case 't':
				string_add_char(&l->tokbuf, '\t');
				st = STATE_NORMAL;
				break;
			case 'u':
				st = STATE_ESCAPE_UNICODE_1;
				break;
			default:
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_ESCAPE_UNICODE_1: {
			int hex = hexctoi(c);
			if (hex == -1) {
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			utf16_high = 0;
			utf16_high |= hex << 12;
			st = STATE_ESCAPE_UNICODE_2;
			break;
		}
		case STATE_ESCAPE_UNICODE_2: {
			int hex = hexctoi(c);
			if (hex == -1) {
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			utf16_high |= hex << 8;
			st = STATE_ESCAPE_UNICODE_3;
			break;
		}
		case STATE_ESCAPE_UNICODE_3: {
			int hex = hexctoi(c);
			if (hex == -1) {
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			utf16_high |= hex << 4;
			st = STATE_ESCAPE_UNICODE_4;
			break;
		}
		case STATE_ESCAPE_UNICODE_4: {
			int hex = hexctoi(c);
			if (hex == -1) {
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}

			utf16_high |= hex;
			if (is_high_surrogate(utf16_high)) {
				/* utf16_high is high surrogate */
				st = STATE_SURROGATE_NORMAL;
				break;
			}

			int len;
			uint16_t pair[2] = {utf16_high, 0};
			char bytes[4];
			len = utf16_to_utf8(pair, bytes);
			if (len == -1) {
				logmsg("utf16_to_utf8 failed.\n");
				lexer_set_general_error(l);
				return NULL;
			}
			for (int i = 0; i < len; i++)
				string_add_char(&l->tokbuf, bytes[i]);
			st = STATE_NORMAL;
			break;
		}
		case STATE_SURROGATE_NORMAL: {
			if (c == '\\') {
				st = STATE_SURROGATE_ESCAPE;
			} else {
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_SURROGATE_ESCAPE: {
			if (c == 'u') {
				st = STATE_SURROGATE_ESCAPE_UNICODE_1;
			} else {
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			break;
		}
		case STATE_SURROGATE_ESCAPE_UNICODE_1: {
			int hex = hexctoi(c);
			if (hex == -1) {
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			utf16_low = 0;
			utf16_low |= hex << 12;
			st = STATE_SURROGATE_ESCAPE_UNICODE_2;
			break;
		}
		case STATE_SURROGATE_ESCAPE_UNICODE_2: {
			int hex = hexctoi(c);
			if (hex == -1) {
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			utf16_low |= hex << 8;
			st = STATE_SURROGATE_ESCAPE_UNICODE_3;
			break;
		}
		case STATE_SURROGATE_ESCAPE_UNICODE_3: {
			int hex = hexctoi(c);
			if (hex == -1) {
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}
			utf16_low |= hex << 4;
			st = STATE_SURROGATE_ESCAPE_UNICODE_4;
			break;
		}
		case STATE_SURROGATE_ESCAPE_UNICODE_4: {
			int hex = hexctoi(c);
			if (hex == -1) {
				logmsg("unexpected character: %c\n", c);
				lexer_set_general_error(l);
				return NULL;
			}

			utf16_low |= hex;

			int len;
			uint16_t pair[2] = {utf16_high, utf16_low};
			char bytes[4];
			len = utf16_to_utf8(pair, bytes);
			if (len == -1) {
				logmsg("utf16_to_utf8 failed.\n");
				lexer_set_general_error(l);
				return NULL;
			}
			for (int i = 0; i < len; i++)
				string_add_char(&l->tokbuf, bytes[i]);
			st = STATE_NORMAL;
			break;
		}
		}
	}

finish:
	lexer_peek_end_value(l);
	return token_new_with_string(ordinal, l->tokbuf);
}

void
lexer_lex(lexer_t *l)
{
	for (;;) {
		int c = file_read(&l->file);
		size_t ordinal = l->file.ordinal;
		token_t *tok;

		switch (c) {
		case EOF:
			l->error =
			    (error_t){.kind = SUCCESS, .ordinal = ordinal};
			return;
		case_whitespace:
			/* whitespace */
			break;
		case 'n':
			file_unread(&l->file, c);
			if ((tok = lexer_lex_null(l)) == NULL)
				return;
			lexer_add_token(l, tok);
			break;
		case 't':
			file_unread(&l->file, c);
			if ((tok = lexer_lex_true(l)) == NULL)
				return;
			lexer_add_token(l, tok);
			break;
		case 'f':
			file_unread(&l->file, c);
			if ((tok = lexer_lex_false(l)) == NULL)
				return;
			lexer_add_token(l, tok);
			break;
		case_digit:
		case '-':
			file_unread(&l->file, c);
			if ((tok = lexer_lex_number(l)) == NULL)
				return;
			lexer_add_token(l, tok);
			break;
		case '"':
			file_unread(&l->file, c);
			if ((tok = lexer_lex_string(l)) == NULL)
				return;
			lexer_add_token(l, tok);
			break;
		case '[':
			lexer_add_token(l,
			    token_new_with_tag(
			        ordinal, TOKEN_TAG_BEGIN_ARRAY));
			break;
		case '{':
			lexer_add_token(l,
			    token_new_with_tag(
			        ordinal, TOKEN_TAG_BEGIN_OBJECT));
			break;
		case ']':
			lexer_add_token(l,
			    token_new_with_tag(ordinal, TOKEN_TAG_END_ARRAY));
			break;
		case '}':
			lexer_add_token(l,
			    token_new_with_tag(ordinal, TOKEN_TAG_END_OBJECT));
			break;
		case ':':
			lexer_add_token(l,
			    token_new_with_tag(ordinal, TOKEN_TAG_NAME_SEP));
			break;
		case ',':
			lexer_add_token(l,
			    token_new_with_tag(ordinal, TOKEN_TAG_VALUE_SEP));
			break;
		default:
			logmsg("unexpected character: %c\n", c);
			l->error = (error_t){
			    .kind = ERROR_GENERAL, .ordinal = ordinal};
			return;
		}
	}
}

token_t *
lexer_read(lexer_t *l)
{
	if (l->buf_len > 0) {
		BUG(l->buf_len > array_len(l->buf));
		l->ordinal++;
		return l->buf[--l->buf_len];
	}

	token_t *ret = l->tokencurr;
	if (l->tokencurr != NULL)
		l->tokencurr = l->tokencurr->next;
	l->ordinal++;
	return ret;
}

void
lexer_unread(lexer_t *l, token_t *tok)
{
	BUG(l->buf_len >= array_len(l->buf));
	BUG(l->ordinal == 0);
	l->buf[l->buf_len++] = tok;
	l->ordinal--;
}

/*
 * node
 */

node_t *
node_new(node_t n)
{
	node_t *ret = xmalloc(sizeof(node_t));
	*ret = n;
	return ret;
}

node_t *
node_new_null(size_t ordinal)
{
	return node_new((node_t){.ordinal = ordinal, .tag = NODE_TAG_NULL});
}

node_t *
node_new_array(size_t ordinal)
{
	return node_new(
	    (node_t){.ordinal = ordinal, .tag = NODE_TAG_ARRAY, .head = NULL});
}

node_t *
node_new_object(size_t ordinal)
{
	return node_new((node_t){
	    .ordinal = ordinal, .tag = NODE_TAG_OBJECT, .head = NULL});
}

node_t *
node_new_aelem(size_t ordinal, size_t index, node_t *value)
{
	return node_new((node_t){.ordinal = ordinal,
	    .tag = NODE_TAG_ARRAY_ELEM,
	    .index = index,
	    .val = value,
	    .next = NULL});
}

node_t *
node_new_oelem(size_t ordinal, string_t name, node_t *value)
{
	return node_new((node_t){.ordinal = ordinal,
	    .tag = NODE_TAG_OBJECT_ELEM,
	    .name = name,
	    .val = value,
	    .next = NULL});
}

node_t *
node_new_with_bool(size_t ordinal, int boolean)
{
	return node_new((node_t){
	    .ordinal = ordinal, .tag = NODE_TAG_BOOL, .boolean = boolean});
}

node_t *
node_new_with_number(size_t ordinal, double num)
{
	return node_new(
	    (node_t){.ordinal = ordinal, .tag = NODE_TAG_NUMBER, .num = num});
}

node_t *
node_new_with_string(size_t ordinal, string_t str)
{
	return node_new(
	    (node_t){.ordinal = ordinal, .tag = NODE_TAG_STRING, .str = str});
}

/*
 * parser
 */

#define parser_expected(p, actual, expected) \
	do { \
		parser_t *glue(RESERVED, p_) = (p); \
		enum token_tag glue(RESERVED, expected_) = (expected); \
		token_t **glue(RESERVED, actual_) = (actual); \
		*glue(RESERVED, actual_) = \
		    lexer_read(&glue(RESERVED, p_)->lexer); \
		if (*glue(RESERVED, actual_) == NULL) { \
			logmsg( \
			    "parser_expected: error: expected token: %s, " \
			    "actual token: (EOF)\n", \
			    token_stringify_tag(glue(RESERVED, expected_))); \
			return NULL; \
		} \
		if ((*glue(RESERVED, actual_))->tag != \
		    glue(RESERVED, expected_)) { \
			logmsg( \
			    "parser_expected: error: expected token: %s, " \
			    "actual token: %s\n", \
			    token_stringify_tag(glue(RESERVED, expected_)), \
			    token_stringify_tag( \
			        (*glue(RESERVED, actual_))->tag)); \
			return NULL; \
		} \
	} while (0)

parser_t
parser_new_with_string(char *str)
{
	return (parser_t){.noderoot = NULL,
	    .lexer = lexer_new_with_string(str),
	    .error = (error_t){.kind = ERROR_GENERAL, .ordinal = 0}};
}

#define case_token_tag_like_value \
	case TOKEN_TAG_NULL: \
	case TOKEN_TAG_BOOL: \
	case TOKEN_TAG_NUMBER: \
	case TOKEN_TAG_STRING: \
	case TOKEN_TAG_BEGIN_ARRAY: \
	case TOKEN_TAG_BEGIN_OBJECT

node_t *parser_parse_value(parser_t *p);

void
parser_set_general_error(parser_t *p, token_t *tok)
{
	p->error = (error_t){
	    .kind = ERROR_GENERAL, .ordinal = tok == NULL ? 0 : tok->ordinal};
}

node_t *
parser_parse_array(parser_t *p)
{
	token_t *t;
	parser_expected(p, &t, TOKEN_TAG_BEGIN_ARRAY);
	node_t *array = node_new_array(t->ordinal);

	enum state {
		STATE_AFTER_BEGIN_ARRAY,
		STATE_AFTER_VALUE,
		STATE_AFTER_VALUE_SEP
	} st = STATE_AFTER_BEGIN_ARRAY;

	node_t *tail = NULL;
	size_t index = 0;
	for (;;) {
		t = lexer_read(&p->lexer);
		if (t == NULL) {
			logmsg("unexpected EOF.\n");
			parser_set_general_error(p, t);
			return NULL;
		}

		switch (st) {
		case STATE_AFTER_BEGIN_ARRAY: {
			switch (t->tag) {
			case TOKEN_TAG_END_ARRAY:
				return array;
			case_token_tag_like_value : {
				lexer_unread(&p->lexer, t);
				node_t *node_value = parser_parse_value(p);
				if (node_value == NULL)
					return NULL;
				node_t *node_elem = node_new_aelem(
				    t->ordinal, index++, node_value);
				if (tail != NULL)
					tail->next = node_elem;
				else
					array->head = node_elem;
				tail = node_elem;
				st = STATE_AFTER_VALUE;
				break;
			}
			default:
				logmsg("unexpected token: %s\n",
				    token_stringify_tag(t->tag));
				parser_set_general_error(p, t);
				return NULL;
			}
			break;
		}
		case STATE_AFTER_VALUE: {
			switch (t->tag) {
			case TOKEN_TAG_END_ARRAY:
				return array;
			case TOKEN_TAG_VALUE_SEP:
				st = STATE_AFTER_VALUE_SEP;
				break;
			default:
				logmsg("unexpected token: %s\n",
				    token_stringify_tag(t->tag));
				parser_set_general_error(p, t);
				return NULL;
			}
			break;
		}
		case STATE_AFTER_VALUE_SEP: {
			switch (t->tag) {
			case_token_tag_like_value : {
				lexer_unread(&p->lexer, t);
				node_t *node_value = parser_parse_value(p);
				if (node_value == NULL)
					return NULL;
				node_t *node_elem = node_new_aelem(
				    t->ordinal, index++, node_value);
				if (tail != NULL)
					tail->next = node_elem;
				else
					array->head = node_elem;
				tail = node_elem;
				st = STATE_AFTER_VALUE;
				break;
			}
			default:
				logmsg("unexpected token: %s\n",
				    token_stringify_tag(t->tag));
				parser_set_general_error(p, t);
				return NULL;
			}
			break;
		}
		}
	}
}

node_t *
parser_parse_object(parser_t *p)
{
	token_t *t;
	parser_expected(p, &t, TOKEN_TAG_BEGIN_OBJECT);
	node_t *object = node_new_object(t->ordinal);

	enum state {
		STATE_AFTER_BEGIN_OBJECT,
		STATE_AFTER_NAME,
		STATE_AFTER_NAME_SEP,
		STATE_AFTER_VALUE,
		STATE_AFTER_VALUE_SEP
	} st = STATE_AFTER_BEGIN_OBJECT;
	string_t name;

	node_t *tail = NULL;
	for (;;) {
		t = lexer_read(&p->lexer);
		if (t == NULL) {
			logmsg("unexpected EOF.\n");
			parser_set_general_error(p, t);
			return NULL;
		}

		switch (st) {
		case STATE_AFTER_BEGIN_OBJECT: {
			switch (t->tag) {
			case TOKEN_TAG_END_OBJECT:
				return object;
			case TOKEN_TAG_STRING:
				name = t->string;
				st = STATE_AFTER_NAME;
				break;
			default:
				logmsg("unexpected token: %s\n",
				    token_stringify_tag(t->tag));
				parser_set_general_error(p, t);
				return NULL;
			}
			break;
		}
		case STATE_AFTER_NAME: {
			switch (t->tag) {
			case TOKEN_TAG_NAME_SEP:
				st = STATE_AFTER_NAME_SEP;
				break;
			default:
				logmsg("unexpected token: %s\n",
				    token_stringify_tag(t->tag));
				parser_set_general_error(p, t);
				return NULL;
			}
			break;
		}
		case STATE_AFTER_NAME_SEP: {
			switch (t->tag) {
			case_token_tag_like_value : {
				lexer_unread(&p->lexer, t);
				node_t *node_value = parser_parse_value(p);
				if (node_value == NULL)
					return NULL;
				node_t *node_elem = node_new_oelem(
				    t->ordinal, name, node_value);
				if (tail != NULL)
					tail->next = node_elem;
				else
					object->head = node_elem;
				tail = node_elem;
				st = STATE_AFTER_VALUE;
				break;
			}
			default:
				logmsg("unexpected token: %s\n",
				    token_stringify_tag(t->tag));
				parser_set_general_error(p, t);
				return NULL;
			}
			break;
		}
		case STATE_AFTER_VALUE: {
			switch (t->tag) {
			case TOKEN_TAG_END_OBJECT:
				return object;
			case TOKEN_TAG_VALUE_SEP:
				st = STATE_AFTER_VALUE_SEP;
				break;
			default:
				logmsg("unexpected token: %s\n",
				    token_stringify_tag(t->tag));
				parser_set_general_error(p, t);
				return NULL;
			}
			break;
		}
		case STATE_AFTER_VALUE_SEP: {
			switch (t->tag) {
			case TOKEN_TAG_STRING:
				name = t->string;
				st = STATE_AFTER_NAME;
				break;
			default:
				logmsg("unexpected token: %s\n",
				    token_stringify_tag(t->tag));
				parser_set_general_error(p, t);
				return NULL;
			}
			break;
		}
		}
	}
}

node_t *
parser_parse_value(parser_t *p)
{
	token_t *t = lexer_read(&p->lexer);
	if (t == NULL)
		return NULL;

	switch (t->tag) {
	case TOKEN_TAG_NULL:
		return node_new_null(t->ordinal);
	case TOKEN_TAG_BOOL:
		return node_new_with_bool(t->ordinal, t->boolean);
	case TOKEN_TAG_NUMBER:
		return node_new_with_number(t->ordinal, t->number);
	case TOKEN_TAG_STRING:
		return node_new_with_string(t->ordinal, t->string);
	case TOKEN_TAG_BEGIN_ARRAY: {
		lexer_unread(&p->lexer, t);
		return parser_parse_array(p);
	}
	case TOKEN_TAG_BEGIN_OBJECT: {
		lexer_unread(&p->lexer, t);
		return parser_parse_object(p);
	}
	default:
		logmsg("unexpected token: %s\n", token_stringify_tag(t->tag));
		parser_set_general_error(p, t);
		return NULL;
	}
}

void
parser_parse(parser_t *p)
{
	lexer_lex(&p->lexer);
	if (p->lexer.error.kind != SUCCESS) {
		p->error = p->lexer.error;
		return;
	}

	p->noderoot = parser_parse_value(p);
	if (p->lexer.tokencurr != NULL)
		logmsg("unexpected token: %s\n",
		    token_stringify_tag(p->lexer.tokencurr->tag));
	else
		p->error.kind = SUCCESS;
}
