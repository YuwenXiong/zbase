/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_CREATE = 258,
     T_DROP = 259,
     T_TABLE = 260,
     T_INDEX = 261,
     T_SELECT = 262,
     T_FROM = 263,
     T_WHERE = 264,
     T_AND = 265,
     T_INSERT = 266,
     T_INTO = 267,
     T_VALUES = 268,
     T_DELETE = 269,
     T_LT = 270,
     T_LE = 271,
     T_GT = 272,
     T_GE = 273,
     T_EQ = 274,
     T_NE = 275,
     T_INT = 276,
     T_FLOAT = 277,
     T_CHAR = 278,
     T_SINGLE_QUOTE = 279,
     T_DOUBLE_QUOTE = 280,
     T_COMMA = 281,
     T_LEFT_BRACKET = 282,
     T_RIGHT_BRACKET = 283,
     T_STAR = 284,
     T_SEMICOLON = 285,
     T_UNIQUE = 286,
     T_PRIMARY = 287,
     T_KEY = 288,
     T_ON = 289,
     T_EOF = 290,
     T_EXIT = 291,
     T_ABORT = 292,
     P_INT = 293,
     NUM = 294,
     P_FLOAT = 295,
     OP = 296,
     IDENTIFIER = 297,
     ANYTOKEN = 298
   };
#endif
/* Tokens.  */
#define T_CREATE 258
#define T_DROP 259
#define T_TABLE 260
#define T_INDEX 261
#define T_SELECT 262
#define T_FROM 263
#define T_WHERE 264
#define T_AND 265
#define T_INSERT 266
#define T_INTO 267
#define T_VALUES 268
#define T_DELETE 269
#define T_LT 270
#define T_LE 271
#define T_GT 272
#define T_GE 273
#define T_EQ 274
#define T_NE 275
#define T_INT 276
#define T_FLOAT 277
#define T_CHAR 278
#define T_SINGLE_QUOTE 279
#define T_DOUBLE_QUOTE 280
#define T_COMMA 281
#define T_LEFT_BRACKET 282
#define T_RIGHT_BRACKET 283
#define T_STAR 284
#define T_SEMICOLON 285
#define T_UNIQUE 286
#define T_PRIMARY 287
#define T_KEY 288
#define T_ON 289
#define T_EOF 290
#define T_EXIT 291
#define T_ABORT 292
#define P_INT 293
#define NUM 294
#define P_FLOAT 295
#define OP 296
#define IDENTIFIER 297
#define ANYTOKEN 298




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

