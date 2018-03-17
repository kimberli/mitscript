/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_BC_BC_PARSER_H_INCLUDED
# define YY_BC_BC_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef BCDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define BCDEBUG 1
#  else
#   define BCDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define BCDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined BCDEBUG */
#if BCDEBUG
extern int bcdebug;
#endif
/* "%code requires" blocks.  */
#line 1 "bc-parser.yy" /* yacc.c:1909  */


#include <iostream>
#include <string>
#define YY_DECL int bclex (BCSTYPE* yylval, BCLTYPE * yylloc, yyscan_t yyscanner)
#ifndef FLEX_SCANNER 
#include "bc-lexer.h"
#endif 

using namespace std;

//The macro below is used by bison for error reporting
//it comes from stacck overflow 
//http://stackoverflow.com/questions/656703/how-does-flex-support-bison-location-exactly
#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 0; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }


#include "instructions.h"
#include "types.h"

#include <cassert>

int32_t safe_cast(int64_t value);
uint32_t safe_unsigned_cast(int64_t value);


#line 90 "bc-parser.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef BCTOKENTYPE
# define BCTOKENTYPE
  enum bctokentype
  {
    T_none = 258,
    T_false = 259,
    T_true = 260,
    T_int = 261,
    T_string = 262,
    T_ident = 263,
    T_function = 264,
    T_functions = 265,
    T_constants = 266,
    T_parameter_count = 267,
    T_local_vars = 268,
    T_local_ref_vars = 269,
    T_free_vars = 270,
    T_names = 271,
    T_instructions = 272,
    T_load_const = 273,
    T_load_func = 274,
    T_load_local = 275,
    T_store_local = 276,
    T_load_global = 277,
    T_store_global = 278,
    T_push_ref = 279,
    T_load_ref = 280,
    T_store_ref = 281,
    T_alloc_record = 282,
    T_field_load = 283,
    T_field_store = 284,
    T_index_load = 285,
    T_index_store = 286,
    T_alloc_closure = 287,
    T_call = 288,
    T_return = 289,
    T_add = 290,
    T_sub = 291,
    T_mul = 292,
    T_div = 293,
    T_neg = 294,
    T_gt = 295,
    T_geq = 296,
    T_eq = 297,
    T_and = 298,
    T_or = 299,
    T_not = 300,
    T_goto = 301,
    T_if = 302,
    T_dup = 303,
    T_swap = 304,
    T_pop = 305
  };
#endif

/* Value type.  */
#if ! defined BCSTYPE && ! defined BCSTYPE_IS_DECLARED

union BCSTYPE
{
#line 52 "bc-parser.yy" /* yacc.c:1909  */

	int32_t intconst;
	std::string* strconst;

	Function*   func;
	vector<shared_ptr<Function>>*   funclist;

    vector<string>* identlist;
	
	Instruction* inst;
	vector<Instruction>* instlist;
	

    Constant* constant;
    vector<shared_ptr<Constant>>* constantlist;

#line 170 "bc-parser.h" /* yacc.c:1909  */
};

typedef union BCSTYPE BCSTYPE;
# define BCSTYPE_IS_TRIVIAL 1
# define BCSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined BCLTYPE && ! defined BCLTYPE_IS_DECLARED
typedef struct BCLTYPE BCLTYPE;
struct BCLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define BCLTYPE_IS_DECLARED 1
# define BCLTYPE_IS_TRIVIAL 1
#endif



int bcparse (yyscan_t yyscanner, Function*& out);
/* "%code provides" blocks.  */
#line 46 "bc-parser.yy" /* yacc.c:1909  */

YY_DECL;
int yyerror(BCLTYPE * yylloc, yyscan_t yyscanner, Function*& out, const char* message);

#line 201 "bc-parser.h" /* yacc.c:1909  */

#endif /* !YY_BC_BC_PARSER_H_INCLUDED  */
