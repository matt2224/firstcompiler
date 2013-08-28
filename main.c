#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "symbol.c"

#define BUFFER_SIZE 25
#define EOF_MARKER '`'

token lookahead;
int token_value;
char token_value_str[20] = "";

char buffer[BUFFER_SIZE] = "";
int lexeme_beginning = 0;
int forward = 0;
int end_reached = 0;

int label_count = 0;

syntax_error() {
    printf("Syntax error\n");
    exit(1);
}

lex_error() {
    printf("Lex error\n");
    exit(1);
}

void new_label(char *c) {
    sprintf(c, "L%d", label_count);
    label_count++;
}

token lex() {
    int state = 0;  

    // remove last token from buffer.
    int n = BUFFER_SIZE - forward;
    strncpy(buffer, buffer+forward, n);
    buffer[n] = '\0';     
    lexeme_beginning = 0;
    forward = 0;

    int buffer_end = strlen(buffer);

    if (!end_reached) {
        // fill buffer
        while (buffer_end < BUFFER_SIZE - 1) {
            char c = getchar();
            if (!feof(stdin)) {
                buffer[buffer_end++] = c;
            } else {
                end_reached = 1;
                // EOF marker.
                buffer[buffer_end++] = EOF_MARKER;
                break;
            }
        }      
        buffer[buffer_end] = '\0';    
    }

    while (1) {
        char c = buffer[forward++];
        switch(state) {
            case 0: if (c == ' ' || c == '\t' || c == '\n') {
                state = 0;
                lexeme_beginning++;
            } else {
                switch(c) {
                    case ';': return SEMICOLON;
                    case ')': return CLOSE_BRACKET;
                    case '(': return OPEN_BRACKET; 
                    case '=': return EQUALS;
                    case EOF_MARKER: return EOF;
                    default: if (isalpha(c)) { state = 1; }
                             else if (isdigit(c)) { state = 2; }
                             else { lex_error(); }
                             break;
                }
            } break;
            case 1: if (isdigit(c) || isalpha(c)) {
                state = 1;
            } else {
                // retract forward pointer.
                forward--;
                int n = forward - lexeme_beginning;

                strncpy(token_value_str, buffer+lexeme_beginning, n);
                token_value_str[n] = '\0';

                symtable_record_ptr rec = symtable_get(token_value_str);
                if (rec) {
                    return rec->token;
                } else {
                    return ID;
                }
            } break;
            case 2: if (isdigit(c)) {
                state = 2;
            } else {
                // retract forward pointer.
                forward--;
                int n = forward - lexeme_beginning;

                strncpy(token_value_str, buffer+lexeme_beginning, n);
                token_value_str[n] = '\0';

                token_value = atoi(token_value_str);
                return NUM;
            } break;
            default: lex_error();
        }
    }
}

void emit(char *s) {
    printf("%s", s);
}

void match(token expected) {

    if (lookahead != expected) {
        syntax_error();
    } else if (lookahead != EOF) {
        // fetch next token
        lookahead = lex();
    }
}


void literal() {
    if (lookahead == TRUE_T) {
        match(TRUE_T);
        emit("1");
    } else if (lookahead == FALSE_T) {
        match(FALSE_T);
        emit("0");
    } else if (lookahead == ID) {
        symtable_record_ptr rec = symtable_get(token_value_str);
        match(ID);
        char s[33];
        sprintf(s, "%d", rec->value);
        emit(s);
    } else {
        char s[33];
        sprintf(s, "%d", token_value);
        match(NUM);
        emit(s);
    }
}

void stmt_list();

void stmt(int depth) {
    if (lookahead == PRINT) {
        match(PRINT);         
        emit("li $v0,1 \n");
        emit("li $t0,");
        literal();
        emit("\n");
        emit("move $a0,$t0 \n");
        emit("syscall \n");

        // print newline
        emit("li $v0,4 \n");
        emit("la $a0,nl \n");
        emit("syscall \n");

    } else if (lookahead == IF) {
        char after_label[10];
        new_label(after_label);
        char else_label[10]; 
        new_label(else_label);

        match(IF); emit("li $t0,");
        literal(); emit("\n beq $t0,0,"); emit(else_label); emit("\n");
        match(OPEN_BRACKET);
        stmt_list(depth+1);
        match(CLOSE_BRACKET); emit("b "); emit(after_label); emit("\n");
        match(ELSE); emit(else_label); emit(":");
        match(OPEN_BRACKET);
        stmt_list(depth+1);
        match(CLOSE_BRACKET); emit(after_label); emit(":");
    } else if (lookahead == DO_TIMES) {
        
        // the register used is based on the depth,
        // so that structures can be nested without conflict.
        char reg_id[5];
        sprintf(reg_id, "$t%d", depth+1);


        char start_label[10];
        new_label(start_label);
        char end_label[10]; 
        new_label(end_label);

        match(DO_TIMES);
        emit("li "); emit(reg_id); emit(",0 \n");
        emit(start_label); emit(":");
        emit("bge "); emit(reg_id); emit(",");
        literal();
        emit(","); emit(end_label); emit("\n");
        match(OPEN_BRACKET);
        stmt_list(depth+1);
        match(CLOSE_BRACKET);
        emit("addi "); emit(reg_id); emit(","); emit(reg_id); emit(",1 \n");
        emit("b "); emit(start_label); emit("\n");
        emit(end_label); emit(":");
    } else if (lookahead == ID) {
        char lexeme[22] = ""; 
        strcpy(lexeme, token_value_str);

        match(ID);
        match(EQUALS);

        int value = token_value;
        match(NUM);

        symtable_insert(lexeme, ID, value);
    } else if (lookahead == FUNC) {
        match(FUNC);

        char func_label[20] = "";   
        strcpy(func_label, token_value_str);

        match(ID);
        emit(func_label); emit(":");

        match(OPEN_BRACKET);
        stmt_list(depth+1);
        match(CLOSE_BRACKET);

        emit("b "); emit("exit \n");
    } else if (lookahead == CALL) {
        match(CALL);

        char func_label[20] = "";   
        strcpy(func_label, token_value_str);

        match(ID);
        emit("b "); emit(func_label); emit("\n");
    }
}

void stmt_list(int depth) {
    if (lookahead == PRINT || lookahead == IF || 
        lookahead == DO_TIMES || lookahead == ID ||
        lookahead == FUNC || lookahead == CALL) {
        stmt(depth); match(SEMICOLON); stmt_list(depth);
    }
}


void parse() {
    emit(".data \n nl: .asciiz \"\\n\" \n");
    emit(".text \n main:");
    emit("b start \n");

    stmt_list(0);
    match(EOF);

    emit("exit: li $v0, 10 \n");
    emit("syscall");
}


int main() {
    // put keywords into symbol table.
    symtable_insert("print", PRINT, 0);
    symtable_insert("if", IF, 0);
    symtable_insert("else", ELSE, 0);
    symtable_insert("dotimes", DO_TIMES, 0);
    symtable_insert("true", TRUE_T, 0);
    symtable_insert("false", FALSE_T, 0);
    symtable_insert("func", FUNC, 0);
    symtable_insert("call", CALL, 0);

    lookahead = lex();

    parse();

    return 0;
}
