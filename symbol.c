 #include <stdlib.h>
 #include <string.h>

typedef enum {
    ID,
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
    EQUALS,
    FUNC,
    CALL,
    END
} token;

typedef struct {
    token token;
    char lexeme[20];
    int value;
} symtable_record, *symtable_record_ptr;

// table can hold max 100 entries.
symtable_record_ptr symtable[100];
int table_index = 0;

void symtable_insert(char *l, token t, int v) {
    // create a new record and put it into the table.
    symtable_record_ptr rec = malloc(sizeof(symtable_record));
    rec->token = t;
    rec->value = v;
    strcpy(rec->lexeme, l); 

    symtable[table_index] = rec;
    table_index++;
}

symtable_record_ptr symtable_get(char *l) {
    // fetch a record from the table by lexeme.
    int i;
    for (i = 0; i < table_index; i++) {
        symtable_record_ptr rec = symtable[i];

        if (strcmp(rec->lexeme, l) == 0) {
            return rec;
        }
    }

    return NULL;
}

void symtable_print() {
    printf("%s", "\n ---------- SYMBOL TABLE ----------- \n");

    int i;
    for (i = 0; i < table_index; i++) {
        symtable_record_ptr rec = symtable[i];

        printf("\t %s | %d \n", rec->lexeme, rec->value);
    }
}