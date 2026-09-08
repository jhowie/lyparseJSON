/* File: parseJSON_support.h
**
** Author: John Howie, (c) Howie Consulting Inc., 2026. (john@howieconsultinginc.com)
**
** Description
**
**      This header file contains the function prototypes and other definitions
** needed for the JSON parser implemented in (f)lex and (bison)yacc.
**
** Modifications
**
** 2026-07-06   John Howie      Original.
**
*/

# ifndef __PARSEJSON_SUPPORT_H__
# define __PARSEJSON_SUPPORT_H__

/*
** Define the internal representation of parsed JSON. We do it here, and not in
** the header file the caller has, as we do not want to expose the details of
** what parsed JSON looks like
*/

typedef char*   jsonstring;
typedef char*   jsonnumber;

struct jsonobjectmember {
        // An object member must always have a name and a value

        jsonstring              name;
        struct jsonvalue        *value;

        // There can be more than one member (name/value pair) in an object,
        // so need to manage pointers to the next and last members

        struct jsonobjectmember *next, *last;
};

struct jsonobject {
        // If the object is empty, the following is NULL

        struct jsonobjectmember *objectmember;
};

struct jsonarrayelement {
        // An array element is just a value

        struct jsonvalue        *value;

        // There can be more than one element in a JSON array, so we have to
        // manage pointers to the next and last elements

        struct jsonarrayelement *next, *last;
};

struct jsonarray {
        // If the array is empty, the following is NULL

        struct jsonarrayelement *jsonarrayelement;
};

struct jsonvalue {
        JSON_VALUETYPE        valuetype;

        // The value can be any number of things, so we use a union to store
        // the different options

        union {
                struct jsonarray        array;
                struct jsonobject       object;
                jsonstring              string;
                jsonnumber              number;
                bool                    boolean;
        }                               v;
};

/*
** The following is used to keep track of state when working with arrays and
** objects - allowing the user to navigate through elements and members
*/

struct jsonarrayobjectstate {
        // The array or object

        struct jsonvalue        *arrayorobject;

        // The current element or member

        union {
                struct jsonarrayelement *arrayelement;
                struct jsonobjectmember *objectmember;
        }                               c;

        // Linked list maintenance pointers (to other states)

        struct jsonarrayobjectstate     *last, *next;
};

/*
** The following are internal functions to this module that we do not want a
** user to see
*/

static JSON		parseJSON (void);
static bool             writeJSONtofp (const JSON parsedJSON, FILE *fp, bool membervalue, const int level);
static int              calculateJSONbuffersize (const JSON parsedJSON, bool membervalue, const int level);
static struct jsonvalue	*makeJSONelement (JSON_VALUETYPE valuetype);
static bool		connecttoparsedJSON (struct jsonvalue *value);
static bool             pushJSONvalue (struct jsonvalue *value);
static struct jsonvalue *peekJSONvalue (void);
static bool             popJSONvalue (void);
static bool             parseJSONarrayobjectbegin (JSON_VALUETYPE valuetype);
static bool             parseJSONarrayorobjectend (void);
static bool             addnewstate (struct jsonvalue *arrayorobject);
static JSON             getnextelementormember (struct jsonvalue *arrayorobject);
static bool             deletestate (struct jsonvalue *arrayorobject);
static char             *encodeJSONstring (const unsigned char *string);
static unsigned char    *decodeJSONstring (const char *string);
static void             setlastJSONerrorstring (char *string);

/*
** The following internal functions are called by the parser. While we could
** expose them to the user for their use, they are a little "ugly", so we do
** not and we have a separate set of public functions for creating JSON
*/

bool    parseJSONaddstring (char *string);
bool    parseJSONaddnumber (char *number);
bool    parseJSONaddboolean (bool trueorfalse);
bool    parseJSONaddnull (void);
bool    parseJSONaddmembername (char *string);
bool    parseJSONobjectbegin (void);
bool    parseJSONobjectend (void);
bool    parseJSONarraybegin (void);
bool    parseJSONarrayend (void);

/*
** The following is the amount by which we grow the internal stack when we
** need to grow it
*/

# define JSON_STACK_GROW_SIZE   16      // 16 elements

/*
** The following functions are called by lex and yacc generated code.
*/

void parseJSONerror (char *errormessage);

# endif // __PARSEJSON_SUPPORT_H__
