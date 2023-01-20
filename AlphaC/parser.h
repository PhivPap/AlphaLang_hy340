/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IF = 258,
    ELSE = 259,
    WHILE = 260,
    FOR = 261,
    FUNCTION = 262,
    RETURN = 263,
    BREAK = 264,
    CONTINUE = 265,
    LOCAL = 266,
    TRUE = 267,
    FALSE = 268,
    NIL = 269,
    INTCONST = 270,
    REALCONST = 271,
    STRING = 272,
    ID = 273,
    CURLY_BR_O = 274,
    CURLY_BR_C = 275,
    COLON = 276,
    COMMA = 277,
    DBL_COLON = 278,
    SEMICOLON = 279,
    ASSIGN = 280,
    OR = 281,
    AND = 282,
    EQUAL = 283,
    UNEQUAL = 284,
    GREATER_THAN = 285,
    GREATER_EQUAL = 286,
    LESSER_THAN = 287,
    LESSER_EQUAL = 288,
    PLUS = 289,
    MINUS = 290,
    ASTERISK = 291,
    SLASH = 292,
    MODULO = 293,
    NOT = 294,
    PLUS_PLUS = 295,
    MINUS_MINUS = 296,
    UMINUS = 297,
    PERIOD = 298,
    DBL_PERIOD = 299,
    SQUARE_BR_O = 300,
    SQUARE_BR_C = 301,
    ROUND_BR_O = 302,
    ROUND_BR_C = 303
  };
#endif
/* Tokens.  */
#define IF 258
#define ELSE 259
#define WHILE 260
#define FOR 261
#define FUNCTION 262
#define RETURN 263
#define BREAK 264
#define CONTINUE 265
#define LOCAL 266
#define TRUE 267
#define FALSE 268
#define NIL 269
#define INTCONST 270
#define REALCONST 271
#define STRING 272
#define ID 273
#define CURLY_BR_O 274
#define CURLY_BR_C 275
#define COLON 276
#define COMMA 277
#define DBL_COLON 278
#define SEMICOLON 279
#define ASSIGN 280
#define OR 281
#define AND 282
#define EQUAL 283
#define UNEQUAL 284
#define GREATER_THAN 285
#define GREATER_EQUAL 286
#define LESSER_THAN 287
#define LESSER_EQUAL 288
#define PLUS 289
#define MINUS 290
#define ASTERISK 291
#define SLASH 292
#define MODULO 293
#define NOT 294
#define PLUS_PLUS 295
#define MINUS_MINUS 296
#define UMINUS 297
#define PERIOD 298
#define DBL_PERIOD 299
#define SQUARE_BR_O 300
#define SQUARE_BR_C 301
#define ROUND_BR_O 302
#define ROUND_BR_C 303

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 105 "parser.y"

  struct alpha_token_t* tok;
  struct Expr* expr;
  struct LogicList* llist;
  struct indexedelem_pair* idxpair;
  struct call_node* cnode;
  int intval;

#line 162 "parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
