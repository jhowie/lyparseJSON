/*
** Copyright: (c) John Howie, Howie Consulting Inc., 2026. (john@howieconsultinginc.com)
*/

%{
# define YYDEBUG        1

# include <stdio.h>
# include <stdbool.h>
# include "parseJSON.h"
# include "parseJSON_support.h"

int parseJSONlex (void);
%}

%union {
	char *str;
}

%token  LYJSON_OBJECT_BEGIN
%token	LYJSON_OBJECT_END
%token  LYJSON_ARRAY_BEGIN
%token  LYJSON_ARRAY_END

%token  LYJSON_SEPARATOR
%token  LYJSON_NVSEPARATOR

%token  LYJSON_TRUE
%token  LYJSON_FALSE
%token  LYJSON_NULL

%token  <str>   LYJSON_STRING
%token  <str>   LYJSON_NUMBER

%%

/*
** The following is taken from http://www.json.org
*/

json            : /* No json */
                | element
                ;

elements        : element
                | elements LYJSON_SEPARATOR element
                ;

element         : value
                ;

value           : object
                | array 
                | LYJSON_STRING { if (! parseJSONaddstring ($1)) YYABORT; }                               
                | LYJSON_NUMBER { if (! parseJSONaddnumber ($1)) YYABORT; }
                | LYJSON_TRUE   { if (! parseJSONaddboolean (true)) YYABORT; }
                | LYJSON_FALSE  { if (! parseJSONaddboolean (false)) YYABORT; }
                | LYJSON_NULL   { if (! parseJSONaddnull ()) YYABORT; }
                ;

object          : LYJSON_OBJECT_BEGIN { if (! parseJSONobjectbegin ()) YYABORT; } LYJSON_OBJECT_END { if (! parseJSONobjectend ()) YYABORT; }
                | LYJSON_OBJECT_BEGIN { if (! parseJSONobjectbegin ()) YYABORT; } members LYJSON_OBJECT_END { if (! parseJSONobjectend ()) YYABORT; }
                ;

members         : member
                | members LYJSON_SEPARATOR member
                ;

member          : LYJSON_STRING { if (! parseJSONaddmembername ($1)) YYABORT; } LYJSON_NVSEPARATOR element      
                ;

array           : LYJSON_ARRAY_BEGIN { if (! parseJSONarraybegin ()) YYABORT; } LYJSON_ARRAY_END { if (! parseJSONarrayend ()) YYABORT; }
                | LYJSON_ARRAY_BEGIN { if (! parseJSONarraybegin ()) YYABORT; } elements LYJSON_ARRAY_END { if (! parseJSONarrayend ()) YYABORT; }
                ;

%%

