/*
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
