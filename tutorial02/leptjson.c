#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <math.h>
#include <errno.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
     char* end;
    /* \TODO validate number */
    const char* ptr = c->json;
    if (*ptr == '+' || *ptr == '.')
        return LEPT_PARSE_INVALID_VALUE;
    if (*ptr == '-')ptr++;
    for (; *ptr != '\0' && *ptr != ' ' && *ptr != 'e'&& *ptr != 'E'; ptr++) {
        if (!ISDIGIT(*ptr) && *ptr != 'E' && *ptr != 'e' && *ptr != '.' && *ptr != '+' && *ptr != '-')
            return LEPT_PARSE_INVALID_VALUE;
        if (*(ptr + 1) == '\0' && *ptr == '.')
            return LEPT_PARSE_INVALID_VALUE;
        if (*ptr == '0' && *(ptr + 1) != 'E' && *(ptr + 1) != 'e' && *(ptr + 1) != '\0' && *(ptr + 1) != '.')return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    for (; *ptr != '\0' && *ptr != ' '; ptr++) {
        if (!ISDIGIT(*ptr) && *ptr != 'E' && *ptr != 'e' && *ptr != '.' && *ptr != '+' && *ptr != '-')
            return LEPT_PARSE_INVALID_VALUE;
    }
    v->n = strtod(c->json, &end);
    if (!isfinite(v->n))return LEPT_PARSE_NUMBER_TOO_BIG;
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_literal(lept_context* c, lept_value* v) {
    if (*c->json == 'f') 
    {
        EXPECT(c, 'f');
        if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
            return LEPT_PARSE_INVALID_VALUE;
        c->json += 4;
        v->type = LEPT_FALSE;
    }
    else if (*c->json == 't') {
        EXPECT(c, 't');
        if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
            return LEPT_PARSE_INVALID_VALUE;
        c->json += 3;
        v->type = LEPT_TRUE;
    }
    else
    {
        EXPECT(c, 'n');
        if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
            return LEPT_PARSE_INVALID_VALUE;
        c->json += 3;
        v->type = LEPT_NULL;
    }
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  
        case 'f':  
        case 'n':  
            return lept_parse_literal(c, v);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
