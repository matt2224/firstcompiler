 #include <stdlib.h>
 #include <string.h>

typedef enum {
    SEMICOLON,
    PRINT,
    IF,
    ELSE,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    TRUE_T,
    FALSE_T,
    DO_TIMES,
    NUM,
    END
} token;

typedef struct {
    token token;
    char *lexeme;
} symtable_record, *symtable_record_ptr;

symtable_record_ptr symtable[100];
int table_index = 0;

void symtable_insert(char *l, token t) {
    symtable_record_ptr rec = malloc(sizeof(symtable_record));
    rec->token = t;
    rec->lexeme = l;

    symtable[table_index] = rec;
    table_index++;
}

symtable_record_ptr symtable_get(char *l) {
    int i;
    for (i = 0; i < table_index; i++) {
        symtable_record_ptr rec = symtable[i];

        if (strcmp(rec->lexeme, l) == 0) {
            return rec;
        }
    }

    return NULL;
}