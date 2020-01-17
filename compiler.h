#include <stdio.h>
#include <stdlib.h>

typedef struct Token Token;
typedef enum TokenType TokenType;

enum TokenType{
    TK_PUNC,
    TK_IDENT,
    TK_KEYWORD,
    TK_CONSTANT,
    TK_LITERAL,
};

struct Token {
    TokenType type;
    Token *next;
    char *str;
    int len;
};

void tokenize(FILE*);