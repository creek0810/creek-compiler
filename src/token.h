#ifndef _TOKEN_H
#define _TOKEN_H
/*
this file define:
1. token structure
2. token type
*/

typedef struct Token Token;
typedef enum TokenType TokenType;

enum TokenType{
    // kw token
    TK_KW_BREAK,
    TK_KW_CHAR,
    TK_KW_CONTINUE,
    TK_KW_ELSE,
    TK_KW_FOR,
    TK_KW_IF,
    TK_KW_INT,
    TK_KW_RETURN,
    TK_KW_WHILE,
    // TODO: following token haven't been supported
    TK_KW_SIGNED,
    TK_KW_UNSIGNED,
    TK_KW_DOUBLE,
    TK_KW_FLOAT,
    TK_KW_SHORT,
    TK_KW_VOID,

    TK_KW_GOTO,
    TK_KW_SWITCH,
    TK_KW_DO,
    TK_KW_CASE,

    TK_KW_UNDEFINED, // to specified the token that is not supported
    // another
    TK_PUNC,
    TK_IDENT,
    TK_INT,
    TK_FLOAT,
    TK_STRING,
    TK_EOF,
};

struct Token {
    TokenType type;
    Token *next;
    char *str;
    int len;
};

#endif