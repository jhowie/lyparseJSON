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

** File: parseJSON.h
**
** Description
**
**      This file contains enumerations, function prototypes, definitions, etc.
** for the Lightweight JSON Parser and supporting functions
*/

# ifndef __PARSEJSON_H__
# define __PARSEJSON_H__

/*
** The following are the types of JSON values that we handle
*/

typedef enum jsonvaluetypes {
        JSON_VALUE_STRING = 1,
        JSON_VALUE_NUMBER,
        JSON_VALUE_OBJECT,
        JSON_VALUE_ARRAY,
        JSON_VALUE_BOOLEAN,
        JSON_VALUE_NULL
} JSON_VALUETYPE;

/*
** The following is a special subtype, not in JSON, that tells us what type of
** number we have in a value
*/

typedef enum jsonvaluenumbertypes {
        JSON_VALUE_NUMBER_INTEGER = 1,
        JSON_VALUE_NUMBER_DOUBLE
} JSON_VALUENUMBERTYPE;

/*
** The datatype we use to track errors
*/

typedef int     JSONERROR;

/*
** We abstract from the user what parsed JSON looks like
*/

typedef void    *JSON;

/*
** The following are functions the user can call to parse JSON, get the parsed
** JSON, and free up parsed JSON
*/

const JSON      parse_JSON_from_buffer (const char *JSONdata, int JSONdatalen, bool makecopy);
const JSON      parse_JSON_from_file (const char *JSONfile);
const JSON      get_parsed_JSON (void);
void            free_parsed_JSON (JSON parsedJSON);

/*
** The following is used to create JSON from an in-memory representation of
** parsed JSON
*/

bool            write_JSON_to_file (const JSON parsedJSON, const char *JSONfile);
const char      *write_JSON_to_buffer (const JSON parsedJSON, int *numbyteswritten);

/*
** The following is used to determine what element type we have
*/

JSON_VALUETYPE  get_JSON_value_type (const JSON parsedJSON);

/*
** The following is used to iterate through elements in an array
*/

bool            is_JSON_array_empty (const JSON parsedJSONarray);
const JSON      get_first_element_in_JSON_array (const JSON parsedJSONarray);
const JSON      get_next_element_in_JSON_array (const JSON parsedJSONarray);

/*
** The following is used to iterate over Name-Value (NV) pairs in an object
*/

const unsigned char     *get_first_membername_in_JSON_object (const JSON parsedJSONobject);
const unsigned char     *get_next_membername_in_JSON_object (const JSON parsedJSONobject);
const JSON              get_membervalue_in_JSON_object (const JSON parsedJSONobject, const unsigned char *membername, bool convertunicode);

/*
** The following return a value in its native format. Note that there is no
** function to get a "null" value, as it is determined from the type alone
*/

unsigned char           *get_JSON_string (const JSON parsedJSONstring);
JSON_VALUENUMBERTYPE    get_JSON_number_type (const JSON parsedJSONnumber);
long                    get_JSON_number_long (const JSON parsedJSONnumber);
double                  get_JSON_number_double (const JSON parsedJSONnumber);
char                    *get_JSON_number_string (const JSON parsedJSONnumber);
bool                    get_JSON_boolean (const JSON parsedJSONboolean);

/*
** The following functions are called to build JSON in memory by the user
*/

JSON    create_JSON_array (void);
JSON    create_JSON_object (void);
JSON    create_JSON_string (const unsigned char *string, bool convertunicode);
JSON    create_JSON_number (const char *number);
JSON    create_JSON_number_long (long number);
JSON    create_JSON_number_double (double number);
JSON    create_JSON_boolean (bool trueorfalse);
JSON    create_JSON_null (void);
JSON    add_element_to_JSON_array (JSON array, JSON value);
JSON    add_member_to_JSON_object (JSON object, const unsigned char *name, bool convertunicode, JSON value);

/*
** The following are general error functions
*/

JSONERROR       get_last_JSON_error (void);
const char      *get_last_JSON_error_string (void);

/*
** The following function is used to control debugging
*/

void set_parse_JSON_debug (bool debug);

/*
** The following are error types
*/

# define JSON_OK                        0 // No error
# define JSON_NOJSON                    1 // No JSON to work with (soft error)
# define JSON_BADJSON                   2 // Syntax error in JSON being parsed
# define JSON_NOTANARRAY                3 // Value is not an array
# define JSON_NOTANOBJECT               4 // Value if not an object
# define JSON_NOTANOBJECTMEMBER         5 // Object member not found
# define JSON_INCORRECTVALUETYPE        6 // Expecting a different type
# define JSON_INVALIDSTRING             7 // The string was not valid
# define JSON_INTERNALERROR             8 // Probable memory corruption
# define JSON_GENERALERROR              9 // Error in C LIbrary (e.g. malloc)

# endif // __PARSEJSON_H__
