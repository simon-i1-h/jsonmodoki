#ifndef JSONMODOKI_H
#define JSONMODOKI_H

#include <stdio.h>

/* string.c */

typedef struct string {
	/*
	 * nul文字終端。
	 *
	 * 文字列型を変化させるような関数呼び出しを行うと参照が無効に
	 * なる可能性がある。reallocにより確保し直される可能性があるた
	 * め。
	 * https://cpprefjp.github.io/reference/string/basic_string/c_str.html
	 */
	char *bytes;

	size_t len;
	size_t capacity;
} string_t;

/* util.c */

/* reserved prefix of identify */
#define RESERVED __jm_reserved_prefix__

/* https://www.jpcert.or.jp/sc-rules/c-pre05-c.html */
#define glue2(a, b) a##b
#define glue(a, b) glue2(a, b)
#define stringify2(s) #s
#define stringify(s) stringify2(s)

#define array_len(a) (sizeof(a) / sizeof((a)[0]))

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
__attribute__((format(printf, 2, 3))) int xasprintf(
    char **strp, const char *fmt, ...);

#define logmsg(...) \
	logmsg2(__FILE__ ":" stringify(__LINE__), NULL, __VA_ARGS__)
__attribute__((format(printf, 3, 4))) void logmsg2(
    const char *progfile, const char *pos, const char *fmt, ...);

__attribute__((format(printf, 2, 3))) int strprintf(
    string_t *dst, const char *fmt, ...);

/* 省略したガード節を書きたいわけだから、条件が真ならエラー */
#define BUG(err) \
	do { \
		if (err) { \
			logmsg("BUG\n"); \
			exit(1); \
		} \
	} while (0)

/* string.c */

string_t string_new(void);
void string_add_char(string_t *s, int c);
void string_add_string(string_t *s, const char *str);

/* types */

enum token_tag {
	TOKEN_TAG_NULL,
	TOKEN_TAG_BOOL,
	TOKEN_TAG_NUMBER,
	TOKEN_TAG_STRING,
	TOKEN_TAG_BEGIN_ARRAY,
	TOKEN_TAG_BEGIN_OBJECT,
	TOKEN_TAG_END_ARRAY,
	TOKEN_TAG_END_OBJECT,
	TOKEN_TAG_NAME_SEP,
	TOKEN_TAG_VALUE_SEP
};

typedef struct token {
	size_t ordinal;
	enum token_tag tag;
	struct token *next;

	/* for bool */
	int boolean;

	/* for number */
	double number;

	/* for string */
	string_t string;
} token_t;

enum node_tag {
	NODE_TAG_NULL,
	NODE_TAG_BOOL,
	NODE_TAG_OBJECT,
	NODE_TAG_ARRAY,
	NODE_TAG_STRING,
	NODE_TAG_NUMBER,
	NODE_TAG_OBJECT_ELEM,
	NODE_TAG_ARRAY_ELEM
};

typedef struct node_t {
	size_t ordinal;

	enum node_tag tag;
	int boolean; /* for bool */
	double num; /* for number */
	string_t str; /* for string */

	size_t index; /* for array element */
	string_t name; /* for object element */

	/* for array element and object element */
	struct node_t *val;
	struct node_t *next;

	struct node_t *head; /* for array and object */
} node_t;

enum file_tag { FILE_TAG_FILE, FILE_TAG_STRING };

typedef struct file {
	/* 初期化時(未入力のとき)は0。n文字目がn。 */
	size_t ordinal;

	enum file_tag tag;

	/* for (real) file */
	FILE *file;

	/* for str */
	char *str;
	size_t str_len;
	size_t str_index;

	int buf[1]; /* stack */
	size_t buf_len; /* <= array_len(buf) */
} file_t;

enum error {
	ERROR_GENERAL,
	SUCCESS,
};

typedef struct error_ {
	enum error kind;
	size_t ordinal;
} error_t;

typedef struct lexer {
	string_t tokbuf;
	token_t *tokenhead;
	token_t *tokentail;

	/* file */
	file_t file;

	/* for reading */
	token_t *tokencurr;
	size_t ordinal;
	token_t *buf[1]; /* stack */
	size_t buf_len; /* <= array_len(buf) */

	/* etc */
	error_t error;
} lexer_t;

typedef struct parser {
	node_t *noderoot;

	/* lexer */
	lexer_t lexer;

	/* etc */
	error_t error;
} parser_t;

/* jsonmodoki.c */

void lexer_lex(lexer_t *t);
lexer_t lexer_new_with_string(char *str);
void parser_parse(parser_t *p);
parser_t parser_new_with_string(char *str);

/* debug.c */

char *token_stringify_tag(enum token_tag tag);
char *token_dump_str(token_t *first);
void token_dump(token_t *first);
void node_dump(node_t *root);

#endif /* JSONMODOKI_H */
