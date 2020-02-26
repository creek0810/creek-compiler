#include <stdlib.h>
#include "type.h"

/* primitive type */
Type INT_TYPE = (Type){.aligned = 4, .size = 4};
Type CHAR_TYPE = (Type){.aligned = 1, .size = 1};

/* function definition */
Type *point_to(Type *cur_type) {
    Type *new_type = calloc(1, sizeof(Type));
    new_type->size = 4;
    new_type->aligned = 4;
    new_type->point_to = cur_type;
    return new_type;
}