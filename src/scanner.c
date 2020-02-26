#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "token.h"

/*
lexical elements:
1. keyword
2. identifier
3. constant
4. string-literal
5. punctuator
*/

Token *token_list = NULL;
Token *cur_token = NULL;

/* warning: op need to be sorted by letters */
/* check more letters op first */
typedef struct CharTypeMap CharTypeMap;
struct CharTypeMap {
    char *val;
    TokenType type;
};

int punc_cnt= 54;
const char *punc_table[54] = {
    // more than 3 letters
    "%:%:", "<<=", ">>=", "...",
    // 2 letters op
    "->", "++", "--", "<:",
    "<<", ">>", "<=", ">=",
    "==", "!=", "&&", "||",
    "*=", "/=", "%=", ":>",
    "+=", "-=", "<%", "%>",
    "&=", "^=", "|=", "##",
    "%:",
    // 1 letter op
    "[", "]", "(", ")",
    "{", "}", ".", "&",
    "*", "+", "-", "~",
    "/", "%", "<", ">",
    "^", "|", "?", ":",
    ";", "=", ",", "#",
    "!",
};

int keyword_cnt = 44;
const CharTypeMap keywords[44] = {
    {"alignof", TK_KW_UNDEFINED},
    {"auto", TK_KW_UNDEFINED},
    {"break", TK_KW_BREAK},
    {"case", TK_KW_UNDEFINED},
    {"char", TK_KW_CHAR},
    {"const", TK_KW_UNDEFINED},
    {"continue", TK_KW_CONTINUE},
    {"default", TK_KW_UNDEFINED},
    {"do", TK_KW_DO},
    {"double", TK_KW_DOUBLE},
    {"else", TK_KW_ELSE},
    {"enum", TK_KW_UNDEFINED},
    {"extern", TK_KW_UNDEFINED},
    {"float", TK_KW_FLOAT},
    {"for", TK_KW_FOR},
    {"goto", TK_KW_GOTO},
    {"if", TK_KW_IF},
    {"inline", TK_KW_UNDEFINED},
    {"int", TK_KW_INT},
    {"long", TK_KW_UNDEFINED},
    {"register", TK_KW_UNDEFINED},
    {"restrict", TK_KW_UNDEFINED},
    {"return", TK_KW_RETURN},
    {"short", TK_KW_SHORT},
    {"signed", TK_KW_SIGNED},
    {"sizeof", TK_KW_UNDEFINED},
    {"static", TK_KW_UNDEFINED},
    {"struct", TK_KW_UNDEFINED},
    {"switch", TK_KW_SWITCH},
    {"typedef", TK_KW_UNDEFINED},
    {"union", TK_KW_UNDEFINED},
    {"unsigned", TK_KW_UNSIGNED},
    {"void", TK_KW_VOID},
    {"volatile", TK_KW_UNDEFINED},
    {"while", TK_KW_WHILE},
    {"_Alignas", TK_KW_UNDEFINED},
    {"_Atomic", TK_KW_UNDEFINED},
    {"_Bool", TK_KW_UNDEFINED},
    {"_Complex", TK_KW_UNDEFINED},
    {"_Generic", TK_KW_UNDEFINED},
    {"_Imaginary", TK_KW_UNDEFINED},
    {"_Noreturn", TK_KW_UNDEFINED},
    {"_Static_assert", TK_KW_UNDEFINED},
    {"_Thread_local", TK_KW_UNDEFINED},
};



/* help function */

int startswith(char *str, char *tar) {
    int str_len = strlen(str);
    int tar_len = strlen(tar);
    if(str_len >= tar_len) {
        return strncmp(str, tar, tar_len) == 0;
    }
    return 0;
}

void add_token(char *str, int len, TokenType type) {
    Token *new_token = calloc(1, sizeof(Token));
    new_token->type = type;
    new_token->next = NULL;
    // deal with str
    new_token->str = calloc(1, sizeof(char) * (len + 1));
    strncpy(new_token->str, str, len);

    new_token->len = len;
    if(cur_token == NULL) {
        cur_token = new_token;
        token_list = new_token;
    } else {
        cur_token->next = new_token;
        cur_token = new_token;
    }
}

bool is_string(char *str) {
    return startswith(str, "u8\"") || startswith(str, "u\"") ||
           startswith(str, "U\"") || startswith(str, "L\"") || startswith(str, "\"");
}

bool is_char(char *str) {
    return startswith(str, "u\'") || startswith(str, "U\'") ||
           startswith(str, "L\'") || startswith(str, "\'");
}

int read_escape(char str[], int cur_loc) {
    /*
        TODO: support
        \x[0-9a-fA-F]*
        \[0-7]
           ^ 1-3
    */
    const char basic_escape[11] = {
        "\\\'", "\\\"", "\\?", "\\\\",
        "\\a", "\\b", "\\f", "\\u", "\\r",
        "\\t", "\\v"
    };
    for(int i=0; i<11; i++) {
        if(startswith(str, &basic_escape[i])) {
            return cur_loc + 2;
        }
    }
    return cur_loc;
}

int read_int_suffix(char str[], int cur_loc) {
    if(str[cur_loc] == 'u' || str[cur_loc] == 'U') {
        cur_loc ++;
        // warning: must check ll LL first
        if(startswith(str + cur_loc, "ll") || startswith(str + cur_loc, "LL")) {
            cur_loc += 2;
        } else if(str[cur_loc] == 'l' || str[cur_loc] == 'L') {
            cur_loc ++;
        }
    } else if(startswith(str + cur_loc, "ll") || startswith(str + cur_loc, "LL")) {
        cur_loc += 2;
        if(str[cur_loc] == 'u' || str[cur_loc] == 'U')
            cur_loc ++;
    } else if(str[cur_loc] == 'l' || str[cur_loc] == 'L') {
        cur_loc ++;
        if(str[cur_loc] == 'u' || str[cur_loc] == 'U')
            cur_loc ++;
    }
    return cur_loc;
}

bool is_hex(char ch) {
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') ||
            (ch >='A' && ch <= 'F');
}

/* token type function */
int punc(char *str, int base_loc, int str_len) {
    int rest_len = str_len - base_loc;
    for(int i=0; i<punc_cnt; i++) {
        int punc_len = strlen(punc_table[i]);
        if(rest_len >= punc_len &&
            strncmp(str + base_loc, punc_table[i], punc_len) == 0) {
            add_token(str + base_loc, punc_len, TK_PUNC);
            return base_loc + punc_len;
        }
    }
}

int ident(char *str, int base_loc, int str_len) {
    // we have checked first char is alpha or '_'
    int loc = base_loc + 1;


    // identifer and keyword can only be the following
    // alpha digit '_'
    while(loc < str_len &&
          (isalpha(str[loc]) || isdigit(str[loc]) || str[loc] == '_')) {
        loc++;
    }
    // check whether is keyword or identifer
    for(int i=0; i<keyword_cnt; i++) {
        int keyword_len = strlen(keywords[i].val);
        // is keyword
        if(keyword_len == (loc - base_loc) &&
           strncmp(str + base_loc, keywords[i].val, keyword_len) == 0) {
            add_token(str + base_loc, keyword_len, keywords[i].type);
            return loc;
        }
    }
    // is identifer
    add_token(str + base_loc, loc - base_loc, TK_IDENT);
    return loc;
}

int constant(char *str, int base_loc, int str_len) {
    // TODO: support float
    int loc = base_loc;
    if(startswith(str + base_loc, "0x") || startswith(str + base_loc, "0X")) {
        loc += 2;
        // hex int
        while(is_hex(str[loc])) loc ++;
    } else if(str[loc] == '0') {
        // oct int
        while(str[loc] >= '0' && str[loc] <= '7') loc ++;
    } else {
        // dec int
        while(isdigit(str[loc])) loc ++;
    }
    // read int suffix
    loc = read_int_suffix(str, loc);
    add_token(str + base_loc, loc - base_loc, TK_INT);
    return loc;
}

int char_literal(char *str, int base_loc, int str_len) {
    // TODO: handle bad token of escape
    int loc = base_loc;
    bool has_found_start = false;
    while(loc < str_len) {
        // deal with escape
        if(str[loc] == '\\') {
            loc = read_escape(str, loc);
            continue;
        }
        // break condition
        if(has_found_start && str[loc] == '\'') {
            loc ++;
            break;
        }
        // init start by finding first '
        if(!has_found_start && str[loc] == '\'') {
            has_found_start = true;
        }
        loc++;
    }
    add_token(str + base_loc, loc - base_loc, TK_CHAR);
    return loc;
}

int string_literal(char *str, int base_loc, int str_len) {
    int loc = base_loc;
    bool has_found_start = false;
    while(loc < str_len) {
        // deal with escape
        if(str[loc] == '\\') {
            loc = read_escape(str, loc);
            continue;
        }
        // break condition
        if(has_found_start && str[loc] == '\"') {
            loc ++;
            break;
        }
        // init start by finding first '
        if(!has_found_start && str[loc] == '\"') {
            has_found_start = true;
        }
        loc++;
    }
    add_token(str + base_loc, loc - base_loc, TK_STRING);
    return loc;
}

Token *scan(FILE *fp) {
    // the max len of a line should not exceed 5000!
    char str[5000] = {0};
    while(fgets(str, 5000, fp) != NULL) {
        int cur_loc = 0;
        int str_len = strlen(str);
        while(cur_loc < str_len) {
            // indent
            if(isspace(str[cur_loc])) {
                cur_loc ++;
                continue;
            }

            // inline comment
            if(startswith(str + cur_loc, "//")) {
                break;
            }

            // char literal
            if(is_char(str + cur_loc)) {
                cur_loc = char_literal(str, cur_loc, str_len);
                continue;
            }

            // constant
            if(isdigit(str[cur_loc])) {
                cur_loc = constant(str, cur_loc, str_len);
                continue;
            }

            /* warning: need to check before ident
                or u8" will be tokenized as a ident.
            */
            // string literal
            if(is_string(str + cur_loc)) {
                cur_loc = string_literal(str, cur_loc, str_len);
                continue;
            }

            // identifier or keyword
            // TODO: support universal-character-name
            if(isalpha(str[cur_loc]) || str[cur_loc] == '_') {
                cur_loc = ident(str, cur_loc, str_len);
                continue;
            }

            // punctuator
            cur_loc = punc(str, cur_loc, str_len);
        }
    }
    add_token("", 0, TK_EOF);
    return token_list;
}
