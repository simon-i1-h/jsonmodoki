#include "jsonmodoki.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char *
token_stringify_tag(enum token_tag tag)
{
	static char *table[] = {[TOKEN_TAG_NULL] = "null",
	    [TOKEN_TAG_BOOL] = "bool",
	    [TOKEN_TAG_NUMBER] = "number",
	    [TOKEN_TAG_STRING] = "string",
	    [TOKEN_TAG_BEGIN_ARRAY] = "begin array",
	    [TOKEN_TAG_BEGIN_OBJECT] = "begin object",
	    [TOKEN_TAG_END_ARRAY] = "end array",
	    [TOKEN_TAG_END_OBJECT] = "end object",
	    [TOKEN_TAG_NAME_SEP] = "name separator",
	    [TOKEN_TAG_VALUE_SEP] = "value separator"};

	BUG(tag >= array_len(table));
	return table[tag];
}

char *
token_dump_str(token_t *first)
{
	string_t ret;
	ret = string_new();

	strprintf(&ret, "dump token:\n");
	strprintf(&ret, "--------------------\n");

	for (token_t *cur = first; cur != NULL; cur = cur->next) {
		strprintf(&ret, "- ordinal: %zd\n", cur->ordinal);
		strprintf(&ret, "  tag: %s\n", token_stringify_tag(cur->tag));
		if (cur->tag == TOKEN_TAG_BOOL)
			strprintf(&ret, "  boolean: %d\n", cur->boolean);
		else if (cur->tag == TOKEN_TAG_NUMBER)
			strprintf(&ret, "  number: %lf\n", cur->number);
		else if (cur->tag == TOKEN_TAG_STRING)
			strprintf(&ret, "  string: %s\n", cur->string.bytes);
	}

	strprintf(&ret, "--------------------\n");

	return ret.bytes;
}

void
token_dump(token_t *first)
{
	printf("%s", token_dump_str(first));
}

char *
node_stringify_tag(enum node_tag tag)
{
	static char *table[] = {[NODE_TAG_NULL] = "null",
	    [NODE_TAG_BOOL] = "bool",
	    [NODE_TAG_ARRAY] = "array",
	    [NODE_TAG_STRING] = "string",
	    [NODE_TAG_NUMBER] = "number",
	    [NODE_TAG_OBJECT] = "object",
	    [NODE_TAG_OBJECT_ELEM] = "object element",
	    [NODE_TAG_ARRAY_ELEM] = "array element"};

	BUG(tag >= array_len(table));
	return table[tag];
}

void
node_dump_str_recur(node_t *cur, string_t *buf, size_t depth)
{
	string_t s = string_new();
	for (size_t i = 0; i < depth; i++)
		string_add_string(&s, "  ");

	if (cur == NULL) {
		strprintf(buf, "%s- NULL POINTER\n", s.bytes);
		return;
	}

	strprintf(buf, "%s- ordinal: %zd\n", s.bytes, cur->ordinal);
	strprintf(buf, "%s  tag: %s\n", s.bytes, node_stringify_tag(cur->tag));
	switch (cur->tag) {
	case NODE_TAG_NULL:
		break;
	case NODE_TAG_BOOL:
		strprintf(buf, "%s  boolean: %d\n", s.bytes, cur->boolean);
		break;
	case NODE_TAG_STRING:
		strprintf(buf, "%s  string: %s\n", s.bytes, cur->str.bytes);
		break;
	case NODE_TAG_NUMBER:
		strprintf(buf, "%s  number: %lf\n", s.bytes, cur->num);
		break;
	case NODE_TAG_OBJECT:
		for (node_t *oe = cur->head; oe != NULL; oe = oe->next)
			node_dump_str_recur(oe, buf, depth + 1);
		break;
	case NODE_TAG_ARRAY:
		for (node_t *ae = cur->head; ae != NULL; ae = ae->next)
			node_dump_str_recur(ae, buf, depth + 1);
		break;
	case NODE_TAG_OBJECT_ELEM:
		strprintf(buf, "%s  key: %s\n", s.bytes, cur->name.bytes);
		node_dump_str_recur(cur->val, buf, depth + 1);
		break;
	case NODE_TAG_ARRAY_ELEM:
		strprintf(buf, "%s  index: %zu\n", s.bytes, cur->index);
		node_dump_str_recur(cur->val, buf, depth + 1);
		break;
	}
}

char *
node_dump_str(node_t *root)
{
	string_t ret;
	ret = string_new();

	strprintf(&ret, "dump node:\n");
	strprintf(&ret, "--------------------\n");

	node_dump_str_recur(root, &ret, 0);

	strprintf(&ret, "--------------------\n");

	return ret.bytes;
}

void
node_dump(node_t *root)
{
	printf("%s", node_dump_str(root));
}
