#include "jsonmodoki.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * 目的は2つ
 * - テストの冗長な出力
 * - ダンパーのデバッグ
 */
int debug_dump = 0;

#define test_expected(expr) \
	do { \
		const char *glue(RESERVED, s) = #expr; \
		if (!(expr)) { \
			logmsg("\"%s\" failed.\n", glue(RESERVED, s)); \
			exit(1); \
		} \
	} while (0)

#define debug_token_dump(tok, text) \
	do { \
		const char *glue(RESERVED, text_) = (text); \
		if (debug_dump) { \
			if (glue(RESERVED, text_) != NULL) \
				logmsg( \
				    "text: \"%s\"\n", glue(RESERVED, text_)); \
			token_dump(tok); \
		} \
	} while (0)

#define debug_node_dump(nod, text) \
	do { \
		const char *glue(RESERVED, text_) = (text); \
		if (debug_dump) { \
			if (glue(RESERVED, text_) != NULL) \
				logmsg( \
				    "text: \"%s\"\n", glue(RESERVED, text_)); \
			node_dump(nod); \
		} \
	} while (0)

static void
test_lex_null(void)
{
	{
		char *text = "null";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_NULL);
		test_expected(lexer.tokenhead->next == NULL);
	}
}

static void
test_whitespace(void)
{
	{
		char *text = " \tnull\r\n";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_NULL);
		test_expected(lexer.tokenhead->next == NULL);
	}
}

static void
test_lex_bool(void)
{
	{
		char *text = "true";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_BOOL);
		test_expected(lexer.tokenhead->boolean == 1);
		test_expected(lexer.tokenhead->next == NULL);
	}

	{
		char *text = "false";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_BOOL);
		test_expected(lexer.tokenhead->boolean == 0);
		test_expected(lexer.tokenhead->next == NULL);
	}
}

static void
test_lex_number(void)
{
	/* zero */
	{
		char *text = "0";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_NUMBER);
		test_expected(lexer.tokenhead->number == 0);
		test_expected(lexer.tokenhead->next == NULL);
	}

	/* positive number */
	{
		char *text = "42";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_NUMBER);
		test_expected(lexer.tokenhead->number == 42);
		test_expected(lexer.tokenhead->next == NULL);
	}

	/* negative number */
	{
		char *text = "-42";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_NUMBER);
		test_expected(lexer.tokenhead->number == -42);
		test_expected(lexer.tokenhead->next == NULL);
	}

	/* fraction */
	{
		char *text = "1.5";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_NUMBER);
		test_expected(lexer.tokenhead->number == 1.5);
		test_expected(lexer.tokenhead->next == NULL);
	}

	/* exponent */
	{
		char *text = "2e4";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_NUMBER);
		test_expected(lexer.tokenhead->number == 20000);
		test_expected(lexer.tokenhead->next == NULL);
	}

	/* complex */
	{
		char *text = "-6.25e-2";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_NUMBER);
		test_expected(lexer.tokenhead->number == -0.0625);
		test_expected(lexer.tokenhead->next == NULL);
	}
}

static void
test_lex_string(void)
{
	/* empty string */
	{
		char *text = "\"\"";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_STRING);
		test_expected(strcmp(lexer.tokenhead->string.bytes, "") == 0);
		test_expected(lexer.tokenhead->next == NULL);
	}

	/* normal string */
	{
		char *text = "\"foobar\"";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_STRING);
		test_expected(
		    strcmp(lexer.tokenhead->string.bytes, "foobar") == 0);
		test_expected(lexer.tokenhead->next == NULL);
	}

	/* normal with multibyte characters */
	{
		char *text = "\"こんにちは\"";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_STRING);
		test_expected(
		    strcmp(lexer.tokenhead->string.bytes, "こんにちは") == 0);
		test_expected(lexer.tokenhead->next == NULL);
	}

	/* string with a escape sequence */
	{
		char *text = "\"hello,\\nworld\"";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_STRING);
		test_expected(strcmp(lexer.tokenhead->string.bytes,
		                  "hello,\nworld") == 0);
		test_expected(lexer.tokenhead->next == NULL);
	}

	/* string with unicode escape sequences */
	{
		char *text =
		    "\"1byte: \\u0061, 2byte: \\u00fc, 3byte: \\u3042, 4byte: "
		    "\\ud834\\udd1e\"";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		test_expected(lexer.error.kind == SUCCESS);
		test_expected(lexer.tokenhead != NULL);
		test_expected(lexer.tokenhead->tag == TOKEN_TAG_STRING);
		test_expected(
		    strcmp(lexer.tokenhead->string.bytes,
		        "1byte: a, 2byte: ü, 3byte: あ, 4byte: 𝄞") == 0);
		test_expected(lexer.tokenhead->next == NULL);
	}
}

static void
test_lex_array(void)
{
	/* empty array */
	{
		char *text = "[]";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		token_t *tok = lexer.tokenhead;

		test_expected(lexer.error.kind == SUCCESS);

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_BEGIN_ARRAY);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_END_ARRAY);
		test_expected(tok->next == NULL);
	}

	/* normal array */
	{
		char *text = "[null, true, false]";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		token_t *tok = lexer.tokenhead;

		test_expected(lexer.error.kind == SUCCESS);

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_BEGIN_ARRAY);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_NULL);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_VALUE_SEP);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_BOOL);
		test_expected(tok->boolean == 1);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_VALUE_SEP);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_BOOL);
		test_expected(tok->boolean == 0);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_END_ARRAY);
		test_expected(tok->next == NULL);
	}
}

static void
test_lex_object(void)
{
	/* empty object */
	{
		char *text = "{}";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		token_t *tok = lexer.tokenhead;

		test_expected(lexer.error.kind == SUCCESS);

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_BEGIN_OBJECT);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_END_OBJECT);
		test_expected(tok->next == NULL);
	}

	/* normal object */
	{
		char *text = "{\"foo\": null, \"bar\": true, \"baz\": false}";
		lexer_t lexer = lexer_new_with_string(text);
		lexer_lex(&lexer);
		debug_token_dump(lexer.tokenhead, text);
		token_t *tok = lexer.tokenhead;

		test_expected(lexer.error.kind == SUCCESS);

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_BEGIN_OBJECT);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_STRING);
		test_expected(strcmp(tok->string.bytes, "foo") == 0);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_NAME_SEP);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_NULL);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_VALUE_SEP);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_STRING);
		test_expected(strcmp(tok->string.bytes, "bar") == 0);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_NAME_SEP);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_BOOL);
		test_expected(tok->boolean == 1);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_VALUE_SEP);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_STRING);
		test_expected(strcmp(tok->string.bytes, "baz") == 0);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_NAME_SEP);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_BOOL);
		test_expected(tok->boolean == 0);
		tok = tok->next;

		test_expected(tok != NULL);
		test_expected(tok->tag == TOKEN_TAG_END_OBJECT);
		test_expected(tok->next == NULL);
	}
}

static void
test_parse_null(void)
{
	{
		char *text = "null";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);
		test_expected(parser.error.kind == SUCCESS);
		test_expected(parser.noderoot != NULL);
		test_expected(parser.noderoot->tag == NODE_TAG_NULL);
	}
}

static void
test_parse_bool(void)
{
	{
		char *text = "true";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);
		test_expected(parser.error.kind == SUCCESS);
		test_expected(parser.noderoot != NULL);
		test_expected(parser.noderoot->tag == NODE_TAG_BOOL);
		test_expected(parser.noderoot->boolean == 1);
	}
}

static void
test_parse_number(void)
{
	{
		char *text = "12345";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);
		test_expected(parser.error.kind == SUCCESS);
		test_expected(parser.noderoot != NULL);
		test_expected(parser.noderoot->tag == NODE_TAG_NUMBER);
		test_expected(parser.noderoot->num == 12345);
	}
}

static void
test_parse_string(void)
{
	{
		char *text = "\"foobarbaz\"";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);
		test_expected(parser.error.kind == SUCCESS);
		test_expected(parser.noderoot != NULL);
		test_expected(parser.noderoot->tag == NODE_TAG_STRING);
		test_expected(
		    strcmp(parser.noderoot->str.bytes, "foobarbaz") == 0);
	}
}

static void
test_parse_array(void)
{
	/* empty */
	{
		char *text = "[]";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);
		test_expected(parser.error.kind == SUCCESS);
		test_expected(parser.noderoot != NULL);
		test_expected(parser.noderoot->tag == NODE_TAG_ARRAY);
		test_expected(parser.noderoot->head == NULL);
	}

	/* a element */
	{
		char *text = "[null]";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);
		node_t *node = parser.noderoot;

		test_expected(parser.error.kind == SUCCESS);

		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY);

		node = node->head;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_NULL);
		test_expected(node->next == NULL);
	}

	/* elements */
	{
		char *text = "[true, true, false]";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);
		node_t *node = parser.noderoot;

		test_expected(parser.error.kind == SUCCESS);

		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY);

		node = parser.noderoot->head;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_BOOL);
		test_expected(node->val->boolean == 1);

		node = parser.noderoot->head->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 1);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_BOOL);
		test_expected(node->val->boolean == 1);

		node = parser.noderoot->head->next->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 2);
		test_expected(node->next == NULL);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_BOOL);
		test_expected(node->val->boolean == 0);
	}

	/* nest (depth-first search) */
	{
		char *text =
		    "[         \n"
		    "  [],     \n"
		    "  [       \n"
		    "    [],   \n"
		    "    [],   \n"
		    "    [     \n"
		    "      []  \n"
		    "    ]     \n"
		    "  ],      \n"
		    "  []      \n"
		    "]         \n";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);

		test_expected(parser.error.kind == SUCCESS);

		/* JSON */
		node_t *node = parser.noderoot;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY);

		/* JSON[0] */
		node = parser.noderoot->head;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_ARRAY);
		test_expected(node->val->head == NULL);

		/* JSON[1] */
		node = parser.noderoot->head->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 1);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_ARRAY);

		/* JSON[1][0] */
		node = parser.noderoot->head->next->val->head;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_ARRAY);
		test_expected(node->val->head == NULL);

		/* JSON[1][1] */
		node = parser.noderoot->head->next->val->head->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 1);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_ARRAY);
		test_expected(node->val->head == NULL);

		/* JSON[1][2] */
		node = parser.noderoot->head->next->val->head->next->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 2);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_ARRAY);

		/* JSON[1][2][0] */
		node = parser.noderoot->head->next->val->head->next->next->val
		           ->head;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_ARRAY);
		test_expected(node->val->head == NULL);

		/* JSON[2] */
		node = parser.noderoot->head->next->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 2);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_ARRAY);
		test_expected(node->val->head == NULL);
	}
}

static void
test_parse_object(void)
{
	/* empty */
	{
		char *text = "{}";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);
		test_expected(parser.error.kind == SUCCESS);
		test_expected(parser.noderoot != NULL);
		test_expected(parser.noderoot->tag == NODE_TAG_OBJECT);
		test_expected(parser.noderoot->head == NULL);
	}

	/* a element */
	{
		char *text = "{\"hi\": true}";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);
		node_t *node = parser.noderoot;

		test_expected(parser.error.kind == SUCCESS);

		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT);

		node = parser.noderoot->head;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT_ELEM);
		test_expected(strcmp(node->name.bytes, "hi") == 0);
		test_expected(node->val->tag == NODE_TAG_BOOL);
		test_expected(node->val->boolean = 1);
		test_expected(node->next == NULL);
	}

	/* elements */
	{
		char *text = "{\"hey\": null, \"foo\": 252.25}";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);
		node_t *node = parser.noderoot;

		test_expected(parser.error.kind == SUCCESS);

		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT);

		node = parser.noderoot->head;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT_ELEM);
		test_expected(strcmp(node->name.bytes, "hey") == 0);
		test_expected(node->val->tag == NODE_TAG_NULL);

		node = parser.noderoot->head->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT_ELEM);
		test_expected(strcmp(node->name.bytes, "foo") == 0);

		test_expected(node->val->tag == NODE_TAG_NUMBER);
		test_expected(node->val->num == 252.25);
		test_expected(node->next == NULL);
	}

	/* nest (depth-first search) */
	{
		char *text =
		    "{                     \n"
		    "  \"bar\": 1.5,       \n"
		    "  \"step\": {         \n"
		    "    \"hello\": null,  \n"
		    "    \"world\": {}     \n"
		    "  },                  \n"
		    "  \"jump\": [         \n"
		    "    false,            \n"
		    "    true              \n"
		    "  ],                  \n"
		    "  \"yo\": null        \n"
		    "}                     \n";
		parser_t parser = parser_new_with_string(text);
		parser_parse(&parser);
		debug_node_dump(parser.noderoot, text);
		node_t *node = parser.noderoot;

		test_expected(parser.error.kind == SUCCESS);

		/* JSON */
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT);

		/* JSON["bar"]*/
		node = parser.noderoot->head;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT_ELEM);
		test_expected(strcmp(node->name.bytes, "bar") == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_NUMBER);
		test_expected(node->val->num == 1.5);

		/* JSON["step"]*/
		node = parser.noderoot->head->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT_ELEM);
		test_expected(strcmp(node->name.bytes, "step") == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_OBJECT);

		/* JSON["step"]["hello"] */
		node = parser.noderoot->head->next->val->head;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT_ELEM);
		test_expected(strcmp(node->name.bytes, "hello") == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_NULL);

		/* JSON["step"]["world"] */
		node = parser.noderoot->head->next->val->head->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT_ELEM);
		test_expected(strcmp(node->name.bytes, "world") == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_OBJECT);
		test_expected(node->val->head == NULL);
		test_expected(node->next == NULL);

		/* JSON["jump"]*/
		node = parser.noderoot->head->next->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT_ELEM);
		test_expected(strcmp(node->name.bytes, "jump") == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_ARRAY);

		/* JSON["jump"][0] */
		node = parser.noderoot->head->next->next->val->head;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_BOOL);
		test_expected(node->val->boolean == 0);

		/* JSON["jump"][1] */
		node = parser.noderoot->head->next->next->val->head->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_ARRAY_ELEM);
		test_expected(node->index == 1);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_BOOL);
		test_expected(node->val->boolean == 1);
		test_expected(node->next == NULL);

		/* JSON["yo"]*/
		node = parser.noderoot->head->next->next->next;
		test_expected(node != NULL);
		test_expected(node->tag == NODE_TAG_OBJECT_ELEM);
		test_expected(strcmp(node->name.bytes, "yo") == 0);
		test_expected(node->val != NULL);
		test_expected(node->val->tag == NODE_TAG_NULL);
		test_expected(node->next == NULL);
	}
}

int
main(void)
{
	test_lex_null();
	test_whitespace();
	test_lex_bool();
	test_lex_number();
	test_lex_string();
	test_lex_array();
	test_lex_object();
	test_parse_null();
	test_parse_bool();
	test_parse_number();
	test_parse_string();
	test_parse_array();
	test_parse_object();

	printf("done.\n");
}
