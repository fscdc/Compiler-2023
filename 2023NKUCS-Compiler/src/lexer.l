%option noyywrap
%option nounput
%option noinput
%top{
    #include <stdarg.h>
    #include "common.h"
    #include "parser.h"
    using namespace std;
    std::vector<Type*> vec;
    std::vector<SymbolEntry*> vec1;
}
%{
    extern dump_type_t dump_type;

    inline void dump_tokens(const char* format, ...){
        va_list args;
        va_start(args, format);
        if (dump_type == TOKENS)
            vfprintf(yyout, format, args);
        va_end(args);
    }

%}

/* definitions section */
DEC ([1-9][0-9]*|0)
OCT (0[0-7]+)
HEX (0[xX][0-9A-Fa-f]+)

DEC_FP ((([0-9]*[.][0-9]*([eE][+-]?[0-9]+)?)|([0-9]+[eE][+-]?[0-9]+))[fLlL]?)
HEX_FP (0[xX](([0-9A-Fa-f]*[.][0-9A-Fa-f]*([pP][+-]?[0-9]+)?)|([0-9A-Fa-f]+[pP][+-]?[0-9]+))[fLlL]?)

ID [[:alpha:]_][[:alpha:][:digit:]_]*
EOL (\r\n|\n|\r)
WHITE [\t ]


%x BLOCKCOMMENT
COMLINE \/\/[^\n]*
commentbegin "/*"
commentelement .|\n
commentend "*/"


%%
    /* rules section */

"int" {
    dump_tokens("INT\t%s\n", yytext);
    return INT;
}

"float" {
    dump_tokens("FLOAT\t%s\n", yytext);
    return FLOAT;
}

"void" {
    dump_tokens("VOID\t%s\n", yytext);
    return VOID;
}

"const" {
    dump_tokens("CONST\t%s\n", yytext);
    return CONST;
}

"if" {
    dump_tokens("IF\t%s\n", yytext);
    return IF;
}

"else" {
    dump_tokens("ELSE\t%s\n", yytext);
    return ELSE;
}

"while" {
    dump_tokens("WHILE\t%s\n", yytext);
    return WHILE;
}

"break" {
    dump_tokens("BREAK\t%s\n", yytext);
    return BREAK;
}

"continue" {
    dump_tokens("CONTINUE\t%s\n", yytext);
    return CONTINUE;
}

"return" {
    dump_tokens("RETURN\t%s\n", yytext);
    return RETURN;
}

"=" {
    dump_tokens("ASSIGN\t%s\n", yytext);
    return ASSIGN;
}

"<" {
    dump_tokens("LESS\t%s\n", yytext);
    return LESS;
}
">" {
    dump_tokens("GREATER\t%s\n", yytext);
    return GREATER;
}
"<=" {
    dump_tokens("LESSEQL\t%s\n", yytext);
    return LESSEQL;
}
">=" {
    dump_tokens("GREATEREQL\t%s\n", yytext);
    return GREATEREQL;
}
"==" {
    dump_tokens("EQL\t%s\n", yytext);
    return EQL;
}
"!=" {
    dump_tokens("NOTEQL\t%s\n", yytext);
    return NOTEQL;
}

"+" {
    dump_tokens("ADD\t%s\n", yytext);
    return ADD;
}

"-" {
    dump_tokens("SUB\t%s\n", yytext);
    return SUB;
}

"*" {
    dump_tokens("MUL\t%s\n", yytext);
    return MUL;
}

"/" {
    dump_tokens("DIV\t%s\n", yytext);
    return DIV;
}

"%" {
    dump_tokens("MOD\t%s\n", yytext);
    return MOD;
}

"&&" {
    dump_tokens("AND\t%s\n", yytext);
    return AND;
}
"||" {
    dump_tokens("OR\t%s\n", yytext);
    return OR;
}
"!" {   
    dump_tokens("NOT\t%s\n", yytext);
    return NOT;
}

";" {
    dump_tokens("SEMICOLON\t%s\n", yytext);
    return SEMICOLON;
}

"(" {
    dump_tokens("LPAREN\t%s\n", yytext);
    return LPAREN;
}

")" {
    dump_tokens("RPAREN\t%s\n", yytext);
    return RPAREN;
}

"{" {
    dump_tokens("LBRACE\t%s\n", yytext);
    return LBRACE;
}

"}" {
    dump_tokens("RBRACE\t%s\n", yytext);
    return RBRACE;
}
"," {
    dump_tokens("COMMA\t%s\n", yytext);
    return COMMA;
}

"[" {
    dump_tokens("LBRACKET\t%s\n", yytext);
    return LBRACKET;
}

"]" {
    dump_tokens("RBRACKET\t%s\n", yytext);
    return RBRACKET;
}


{DEC} {
    int decimal = atoi(yytext);
    dump_tokens("DEC\t%s\t%d\n", yytext, decimal);
    yylval.itype = decimal;
    return INTEGER;
}

{OCT} {
    int octal = strtol(yytext, NULL, 8);
    dump_tokens("OCT\t%s\t%d\n", yytext, octal);
    yylval.itype = octal;
    return INTEGER;
}   

{HEX} {
    int hex = strtol(yytext, NULL, 16);
    dump_tokens("HEX\t%s\t%d\n", yytext, hex);
    yylval.itype = hex;
    return INTEGER;
}

{DEC_FP} {
    float decfp=stof(yytext,nullptr);
    dump_tokens("DEC_FP\t%s\t%d\n", yytext, decfp);
    yylval.ftype = atof(yytext);
    return FLOATNUMBER;
}

{HEX_FP} {
    float hexfp=stof(yytext,nullptr);
    dump_tokens("HEX_FP\t%s\t%d\n", yytext, hexfp);
    yylval.ftype = atof(yytext);
    return FLOATNUMBER;
}

{ID} {
    char *lexeme;
    dump_tokens("ID\t%s\n", yytext);
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    return ID;
}


{EOL} {
    yylineno++;
}
{WHITE}

{COMLINE} {}

{commentbegin} {
    BEGIN BLOCKCOMMENT;   
}
<BLOCKCOMMENT>{commentelement} {}
<BLOCKCOMMENT>{commentend} {
    BEGIN INITIAL;
}





    /*  Your code here (lab3). */
%%
/* user code section */
