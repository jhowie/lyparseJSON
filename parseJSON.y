/*

BSD 3-Clause License

Copyright (c) 2026, John Howie

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

