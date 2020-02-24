#ifndef _TYPE_H
#define _TYPE_H

typedef struct Type Type;
struct Type {
    int size;
    int aligned;
    Type *point_to;
};

Type INT_TYPE;
Type CHAR_TYPE;

#endif