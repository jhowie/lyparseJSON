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

%option outfile="parseJSON.lex.c"

%{
# include <stdio.h>
# include <stdbool.h>
# include "parseJSON.h"
# include "parseJSON_support.h"
# include "parseJSON.tab.h"

extern bool parseJSONdebugflag;

# define Token(_X)      if (parseJSONdebugflag) printf ("lex: returning %s\n", yytext); return _X

char *mknumberstr (char *s);
char *mkstringstr (char *s);

%}

ws      [ \t\r\n]
number  [-]?(([0])|([1-9][0-9]*))([.][0-9]+)?(([Ee][+-]?[0-9+]))?
string  "\""([^\\\"]|\\.)*"\""

%%

"{"                             { Token (LYJSON_OBJECT_BEGIN); }
"}"                             { Token (LYJSON_OBJECT_END); }
"["                             { Token (LYJSON_ARRAY_BEGIN); }
"]"                             { Token (LYJSON_ARRAY_END); }
","                             { Token (LYJSON_SEPARATOR); }
":"                             { Token (LYJSON_NVSEPARATOR); }
"true"                          { Token (LYJSON_TRUE); }
"false"                         { Token (LYJSON_FALSE); }
"null"                          { Token (LYJSON_NULL); }

{number}                        { parseJSONlval.str = mknumberstr (yytext); Token (LYJSON_NUMBER); }
{string}                        { parseJSONlval.str = mkstringstr (yytext); Token (LYJSON_STRING); }


{ws}                    { }
.                       { Token (yytext [0]); }

%%
/* char *mknumberstr (char *s)
**
** This function is called to make a string containing a number that is passed
** to the parser
*/

char *mknumberstr (char *s)
{
	char *str;

	str = (char *) malloc (strlen (s) +1);
	if (str == (char *) 0) {
		parseJSONerror ("malloc failed in mknumberstr\n");
	}

	strcpy (str, s);

	return str;
}

/* char *mkstringstr (char *s)
**
** This function is called to make a string containing a string that is passed
** to the parser. In doing so we remove the surrounding quotes (") from the
** input stream (e.g. "12345" > 12345)
*/

char *mkstringstr (char *s)
{
	int	len;
	char	*str;

	len = strlen (s);
	str = (char *) malloc (len -2 +1);	// Remove quotes and add NULL
	if (str == (char *) 0) {
		parseJSONerror ("malloc failed in mstringstr\n");
	}

	// Copy over from first character after leading quote up to last 
	// character before trailing quote, and add trailing NULL character

	strncpy (str, (char *) (s + 1), (len - 2));
	*(str + (len -2)) = (char) 0;

	return str;
}
