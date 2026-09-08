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

** File: parseJSON_support.c
**
** Author: John Howie, (c) Howie Consulting Inc., 2026. (john@howieconsultinginc.com)
**
** Description
**
**      This source code file contains the routines called by the parser
** created by (f)lex and (bison)yacc, to add arrays elements to the internal
** representation of the parsed JSON.
**
** Modifications
**
** 2026-07-06   John Howie      Original.
**
*/

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "parseJSON.h"
#include "parseJSON.lex.h"
#include "parseJSON_support.h"

/*
** This is an external function, from (f)lex and (bison)yacc
*/

extern int parseJSONparse(void);

/*
** This is an external variable, from (f)lex and (bison)yacc
*/

extern FILE *parseJSONin;

/*
** The followng is our internal error
*/

static JSONERROR jsonerror = JSON_OK;
char *lastJSONerrorstring = (char *) 0;

/*
** The following is the global debug flag
*/

bool parseJSONdebugflag = false;

/*
** The following is the stack we use to track nested JSON elements, and a stack
** counter and current stack size. The stack is incremented by a fixed size
** every time we grow it, and that is defined in the support header file
*/

static struct jsonvalue **JSONelementstack = (struct jsonvalue **)0;
static int numelementsonJSONstack = 0, JSONstacksize = 0;

/*
** The following is a global variable that is used to store the location of the
** most recently parsed JSON. It changes every time JSON is parsed. It should
** be copied by the user, and freed when no longer required
*/

static JSON currentparsedJSON = (JSON)0;

/*
** The following is a global variable used to track the state of arrays and
** objects when the user is manipulating elements and members
*/

static struct jsonarrayobjectstate *arrayobjectstate = (struct jsonarrayobjectstate *)0;

/* const JSON parse_JSON_from_buffer (const char *JSONdata, int JSONdatalen, bool makecopy)
**
** This function parses raw JSON in a buffer, turning it into the in-memory
** representation that we can work with. The first argument is the raw JSON
** data, the second is the length of the JSON data, and the third is whether or
** not to make a copy of the data before parsing it. The lexical analysis and
** parsing functiopn will corrupt the in-memory raw JSON, so it is always best
** to set makecopy true. If makecopy is false, no copy is made, and the buffer
** must be at least two characters longer that the raw JSON data (as specified
** bu the second argument, as two NULL characters are appended to it)
*/

const JSON parse_JSON_from_buffer(const char *JSONdata, int JSONdatalen, bool makecopy)
{
	char *rawJSONbuffer;

	// Check to see if we have to make a copy of the JSON, before we
	// process it

	if (makecopy)
	{
		// Call parseJSON_scan_bytes to set up the buffer we will scan
		// from. A copy is made of the buffer we pass to it, and that
		// is used (preserving the contents of the original buffer)

		if (parseJSON_scan_bytes(JSONdata, JSONdatalen) == (YY_BUFFER_STATE)0)
		{
			// An error occurred, and we cannot proceed

			jsonerror = JSON_GENERALERROR;
			setlastJSONerrorstring("Unable to make copy of JSON in memory to parse");
			return (JSON)0;
		}
	}
	else
	{
		// The caller does not want us to make a copy of the JSON. We
		// will work with the original. Add two NULLs to the end of the
		// buffer

		rawJSONbuffer = (char *)JSONdata;
		rawJSONbuffer[JSONdatalen] = (char)0;
		rawJSONbuffer[JSONdatalen + 1] = (char)0;

		// Ssetup the lexical analyzer to read from the in-memory copy
		// of JSON without copying it

		if (parseJSON_scan_buffer((char *)JSONdata, (JSONdatalen + 2)) == (YY_BUFFER_STATE)0)
		{
			// An error occurred, and we cannot proceed. Set up the
			// error and return

			jsonerror = JSON_GENERALERROR;
			setlastJSONerrorstring("Unable to use JSON in memory to parse");
			return (JSON)0;
		}
	}

	// Now, call the actual parser and just return what it gives us

	return (JSON)parseJSON();
}

/* const JSON parse_JSON_from_file (FILE *fp)
**
** This function parses raw JSON from a file, turning it into a series of
** structures that can be navigated and processed
*/

const JSON parse_JSON_from_file(const char *JSONfile)
{
	JSON parsedJSON;

	// Sanity check that we got a filename

	if (JSONfile == (char *)0)
	{
		// We did not get a filename, so we cannot proceed

		setlastJSONerrorstring("no filename specified to parseJSONfromfile ()\n");
		return (JSON)0;
	}

	// Attempt to open the file the user specified, and assign it to the
	// global variable parseJSONparse (yylex()) uses

	if ((parseJSONin = fopen(JSONfile, "r")) == NULL)
	{
		// An error occurred, and we could not open the file the user
		// specified. We cannot proceed

		setlastJSONerrorstring("unable to open file specified\n");
		return (JSON)0;
	}

	// Call the function that actually parses the JSON

	parsedJSON = parseJSON();

	// Close the configuation file, and return

	(void)fclose(parseJSONin);

	// Return the JSON

	return parsedJSON;
}

/* JSON parseJSON (void)
**
** This function is called to actually parse the JSON, and interpret the result
** before returning to the caller. It should not be called by the user, but
** there is actually no reason why it cannot. We just don't make it available
** in the header file that the user has access to
*/

JSON parseJSON(void)
{
	// Clear the internal parsed JSON pointer

	currentparsedJSON = (JSON)0;

	// Parse JSON, and check the result

	switch (parseJSONparse())
	{
	case 0:
		// We were able to parse the JSON just fine

		return get_parsed_JSON();
		break;

	case 1:
		// An error occurred, and we could not parse the JSON in the
		// file correctly

		jsonerror = JSON_BADJSON;
		free_parsed_JSON(currentparsedJSON);
		return (JSON)0;
		break;

	case 2:
	default:
		// An error occurred, and we could not parse the JSON due to
		// memory exhaustion

		jsonerror = JSON_GENERALERROR;
		free_parsed_JSON(currentparsedJSON);
		return (JSON)0;
		break;
	}
}

/* void free_parsed_JSON (JSON parsedJSON)
**
** This function is used to free up parsed JSON. The in-memory representation
** of the parsed JSON is passed as the sole argument
**
** TODO: Re-write to avoid recursion
*/

void free_parsed_JSON(JSON parsedJSON)
{
	struct jsonvalue	*currentvalue = (struct jsonvalue *) parsedJSON;
	struct jsonarrayelement	*currentelement, *nextelement;
	struct jsonobjectmember	*currentmember, *nextmember;

	// Sanity check that we were passed actual in-memory representation of
	// parsed JSON. If we were not, we just return

	if (parsedJSON == (JSON) 0)
	{
		// We were not passed any in-memory representation of JSON, so
		// just return

		return;
	}

	// Find out what the JSON value is, as that drives how we free it

	switch (currentvalue ->valuetype) {
	case JSON_VALUE_BOOLEAN:
	case JSON_VALUE_NULL:
		// Delete the value, as there are no pointers, members or
		// elements to delete with it

		free (currentvalue);
		break;

	case JSON_VALUE_NUMBER:
		// Numbers have a string that is used to represent the number,
		// so we have to free that string before we delete the value

		free (currentvalue ->v.number);
		free (currentvalue);
		break;

	case JSON_VALUE_STRING:
		// We have to free the string before we free the value

		free (currentvalue ->v.string);
		free (currentvalue);
		break;

	case JSON_VALUE_ARRAY:
		// We have to go through the array deleting all the values
		// in it. We could use the public functions to iterate through
		// the members, but we do not need to as we know the underlying
		// structure of the array

		currentelement = currentvalue ->v.array.jsonarrayelement;
		while (currentelement != (struct jsonarrayelement *) 0) {
			// Get the next element - we need to record it so we
			// can get to it after we have deleted this element

			nextelement = currentelement ->next;

			// Recurse into ourselves to delete the element itself

			free_parsed_JSON ((JSON) currentelement ->value);

			// Now, delete the current element

			free (currentelement);

			// Make the next element the current element

			currentelement = nextelement;
		}

		// Now we have deleted all the elements of the array, we can
		// free up the memory of the array itself

		free (currentvalue);
		break;

	case JSON_VALUE_OBJECT:
		// We have to go through the members of the array, deleting the
		// member name and the associated value. We could use the
		// public functions to iterate through the members, but we know
		// the internal structure

		currentmember = currentvalue ->v.object.objectmember;
		while (currentmember != (struct jsonobjectmember *) 0) {
			// Get the next member, as we need it after we delete
			// the current member

			nextmember = currentmember ->next;

			// An object member is made up of a string name and the
			// value of the member itself. We need to delete the
			// name first, then recurse into ourselves to delete
			// the value itself

			free (currentmember ->name);
			free_parsed_JSON (currentmember ->value);

			// Now, delete the current object

			free (currentmember);

			// Make the next object the current object

			currentmember = nextmember;
		}

		// Now that we have deleted the members of the object, we can
		// delete the object itself

		free (currentvalue);
		break;

	default:
		// An error occurred - we have some unknown JSON value type. We
		// can only set the error and return

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring ("Unknown JSON value type found when deleting JSON");
		return;
		break;
	}
}

/* const JSON get_parsed_JSON (void)
**
** This routine is used to return the most recently parsed JSON to the caller
*/

const JSON get_parsed_JSON(void)
{
	// Return the pointer to the most recently parsed JSON. This pointer
	// will change every time JSON is parsed

	return currentparsedJSON;
}

/* void set_parse_JSON_debug (bool debug)
**
** This function is used to set the debug flag for the parser. It will produce
** A LOT of output if set
*/

void set_parse_JSON_debug(bool debug)
{
	// Set the debug flag

	parseJSONdebugflag = debug;
}

/* JSONERROR get_last_JSON_error (void)
**
** This function is used to return the last JSON error encountered, if any
*/

JSONERROR get_last_JSON_error(void)
{
	// Return the internal error

	return jsonerror;
}

/* const char *get_last_JSON_error_string (void)
**
** This function is called to get the last error string written
*/

const char *get_last_JSON_error_string (void)
{
	return lastJSONerrorstring;
}

/* void parseJSONerror (char *errormessage)
**
** This function is called from the (bison)yacc-generated parser when it runs
** into a problem, usually when it encounters a syntax error. We print the
** error to stderr, and record it
*/

void parseJSONerror(char *errormessage)
{
	jsonerror = JSON_BADJSON;
	setlastJSONerrorstring (errormessage);
}

/* int parseJSONwrap (void)
**
** This function replaces yywrap from the Lex/Flex library, as we overrode the
** generated function names in the Makefile. It simply returns the value 1, to
** stop processing at end of file input to the parser.
*/

int parseJSONwrap(void)
{
	return 1;
}

/* static void setlastJSONerrorstring (char *string)
**
** This function is used to set the last JSON error string, which can be
** retrieved by the user, if they want more detail about what went wrong
*/

static void setlastJSONerrorstring (char *string)
{
	lastJSONerrorstring = string;

	if (parseJSONdebugflag)
		fprintf(stderr, "%s\n", string);
}

/* bool write_JSON_to_file (const JSON parsedJSON, const char *JSONfile)
**
** This function is used to write JSON out to a file, from the in-memory
** representation of parsed JSON. The function returns true upon success and
** false otherwise. If the second argument, the name of the file to write out
** to is a NULL pointer, the function will write to stdout
*/

bool write_JSON_to_file(const JSON parsedJSON, const char *JSONfile)
{
	FILE *fp;
	bool retval;

	// Check that we got a pointer to JSON

	if (parsedJSON == (JSON)0)
	{
		// There is nothing to write out. Return true

		return true;
	}

	// If the user specified a file to write to, open that file for writing

	if (JSONfile != (char *)0)
	{
		// Open the file for writing

		if ((fp = fopen(JSONfile, "w")) == (FILE *)NULL)
		{
			// An error occurred, and we could not open the file to
			// write JSON to. Set the error and return false

			jsonerror = JSON_GENERALERROR;
			setlastJSONerrorstring("Unable to open file to write JSON to");
			return false;
		}
	}
	else
	{
		// Make fp the equivalent of stdout

		fp = stdout;
	}

	// Call the function that actually writes the JSON out

	retval = writeJSONtofp(parsedJSON, fp, false, 0);
	if (retval)
	{
		// Write out a trailing newline

		fprintf(fp, "\n");
	}

	// Close the file, and return the result

	fclose(fp);
	return retval;
}

/* static int calculateJSONbuffersize (const JSON parsedJSON, bool membervalue, const int level)
**
** This internal function is called to calculate the size of the buffer
** necessary to store written out JSON. We need this to allocate memory to
** store JSON
**
** TODO: Re-write to avoid recursion
*/

static int calculateJSONbuffersize(const JSON parsedJSON, bool membervalue, const int level)
{
	int buffersize = 0;
	const struct jsonvalue *currentvalue = (struct jsonvalue *)parsedJSON;
	struct jsonarrayelement *currentarrayelement;
	struct jsonobjectmember *currentobjectmember;
	int i;

	// Calculate the number of characters we need for indentation. Each one
	// is a tab character

	if (!membervalue)
		buffersize += level;

	// Work out what the current JSON we have is

	switch (currentvalue->valuetype)
	{
	case JSON_VALUE_BOOLEAN:
		// Figure out how many characters are required to write out the
		// boolean value (four for "true" and five for "false")

		buffersize += (currentvalue->v.boolean ? 4 : 5);
		break;

	case JSON_VALUE_NULL:
		// Add four for "null"

		buffersize += 4;
		break;

	case JSON_VALUE_NUMBER:
		// We simply add the length of the number (which is stored as
		// a string in memory)

		buffersize += strlen(currentvalue->v.number);
		break;

	case JSON_VALUE_STRING:
		// We simply add the length of the string plus two (for the
		// surrounding quotes)

		buffersize += (strlen(currentvalue->v.string) + 2);
		break;

	case JSON_VALUE_ARRAY:
		// Add two for the beginning of the array and a newline

		buffersize += 2;

		// Go through each member of the array

		currentarrayelement = currentvalue->v.array.jsonarrayelement;
		while (currentarrayelement != (struct jsonarrayelement *)0)
		{
			// Recurse into ourselves to print this array element

			buffersize += calculateJSONbuffersize(currentarrayelement->value, false, (level + 1));

			// Now, calculate the space necessary for an optional
			// separator and a newline (where there are more
			// elements in the array)

			if (currentarrayelement->next != (struct jsonarrayelement *)0)
				buffersize += 2;
			else
				buffersize++;

			// Get the next element in the array

			currentarrayelement = currentarrayelement->next;
		}

		// We got to the end of the array. Add to the buffer size the
		// indent (level) and the array terminator

		buffersize += (level + 1);
		break;

	case JSON_VALUE_OBJECT:
		// Add two for the beginning of the object and a newline

		buffersize += 2;

		// Go through each member of the object

		currentobjectmember = currentvalue->v.object.objectmember;
		while (currentobjectmember != (struct jsonobjectmember *)0)
		{
			// Add to the buffer size the indent (level plus 1) and
			// the member string name plus two for quotes plus
			// three for ' : '

			buffersize += ((level + 1) + 2 + strlen(currentobjectmember->name) + 3);

			// Recurse into ourselves to display value associated
			// with the member

			buffersize += calculateJSONbuffersize(currentobjectmember->value, true, (level + 1));

			// Now, calculate the space necessary for an optional
			// separator and a newline (where there are more
			// members in the object)

			if (currentobjectmember->next != (struct jsonobjectmember *)0)
				buffersize += 2;
			else
				buffersize++;

			// Get the next member in the object

			currentobjectmember = currentobjectmember->next;
		}

		// We got to the end of the object. Add to the buffer size the
		// indent (level) and the object terminator

		buffersize += (level + 1);
		break;

	default:
		// We got an unknown value, set an error message and return
		// false

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring("Got unknown value type when calculating JSON buffer size");
		return 0;
		break;
	}

	// Just return the buffer size

	return buffersize;
}

/* const char *write_JSON_to_buffer (const JSON parsedJSON, int *numbyteswritten)
**
** This function is used to write JSON to a buffer, in memory. The buffer is
** created for the caller, and a pointer to it returned. If the JSON cannot
** be written to the buffer, or the buffer cannot be created (and grown as
** necessary), a NULL pointer is returned
*/

const char *write_JSON_to_buffer(const JSON parsedJSON, int *numbyteswritten)
{
	FILE *fp;
	int buffersize;
	char *JSONbuffer;

	// Calculate how much memory we need to store the JSON we will write
	// into the buffer. We add one for a trailing NULL character

	buffersize = calculateJSONbuffersize(parsedJSON, false, 0) + 1;

	// Now, allocate memory for the buffer

	JSONbuffer = malloc(buffersize);
	if (JSONbuffer == (char *)0)
	{
		// An error occurred, and we could not allocate memory for the
		// JSON buffer

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring("Unable to allocate memory for JSON buffer");
		return (char *)0;
	}

	// Now, open a file pointer to the buffer we just created

	if ((fp = fmemopen(JSONbuffer, buffersize, "w")) == (FILE *)0)
	{
		// An error occurred, and we could not open a file stream to
		// the buffer

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring("Unable to open file stream to JSON buffer");
		free(JSONbuffer);
		return (char *)0;
	}

	// Now, write JSON to the file pointer (that writes to the memory
	// buffer we created)

	if (!writeJSONtofp(parsedJSON, fp, false, 0))
	{
		// An error occurred, and we could not write the JSON to the
		// buffer, via the file stream

		jsonerror = JSON_INTERNALERROR;
		setlastJSONerrorstring("Unable to write JSON to file stream to buffer");
		free(JSONbuffer);
		return (char *)0;
	}

	// Close the file stream to our memory buffer

	fclose(fp);

	// Append a NULL character (just in case)

	JSONbuffer[buffersize] = (char)0;

	// Update the pointer to the integer where we store the number of bytes
	// written (if not a NULL pointer), and return the buffer to the user

	if (numbyteswritten != (int *)0)
		*numbyteswritten = buffersize;
	return JSONbuffer;
}

/* static bool writeJSONtofp (const JSON parsedJSON, FILE *fp, bool membervalue, const int level)
**
** This internal function is used to write the internal parsed JSON to the file
** pointer, which points to a file or a memory buffer. The level is used for
** indenting. The membervalue boolean, if true, causes the identation to be
** ignored
**
** TODO: Re-write to avoid recursion
*/

static bool writeJSONtofp(const JSON parsedJSON, FILE *fp, bool membervalue, const int level)
{
	const struct jsonvalue *currentvalue = (struct jsonvalue *)parsedJSON;
	struct jsonarrayelement *currentarrayelement;
	struct jsonobjectmember *currentobjectmember;
	int i;

	// Create the indentation if not a member value

	for (i = 0; i < level && (!membervalue); i++)
		fprintf(fp, "\t");

	// Work out what the current JSON we have is

	switch (currentvalue->valuetype) {
	case JSON_VALUE_BOOLEAN:
		// Write out the boolean value

		fprintf(fp, "%s", currentvalue->v.boolean ? "true" : "false");
		break;

	case JSON_VALUE_NULL:
		// Just print out the null value

		fprintf(fp, "null");
		break;

	case JSON_VALUE_NUMBER:
		// Write out the number (which is held as a string internally)

		fprintf(fp, "%s", currentvalue->v.number);
		break;

	case JSON_VALUE_STRING:
		// Print out the string

		fprintf(fp, "\"%s\"", currentvalue->v.string);
		break;

	case JSON_VALUE_ARRAY:
		// Write out the array begin character

		fprintf(fp, "[\n");

		// Go through each member of the array

		currentarrayelement = currentvalue->v.array.jsonarrayelement;
		while (currentarrayelement != (struct jsonarrayelement *)0)
		{
			// Recurse into ourselves to print this array element

			if (!writeJSONtofp(currentarrayelement->value, fp, false, (level + 1)))
			{
				// An error occurred - return false

				return false;
			}

			// Now, print the separator, and a newline if there is
			// another element, otherwise just print a newline

			if (currentarrayelement->next != (struct jsonarrayelement *)0)
				fprintf(fp, ",\n");
			else
				fprintf(fp, "\n");

			// Get the next element in the array

			currentarrayelement = currentarrayelement->next;
		}

		// We got to the end of the array. We need to write out the
		// array terminating character

		for (i = 0; i < level; i++)
			fprintf(fp, "\t");
		fprintf(fp, "]");
		break;

	case JSON_VALUE_OBJECT:
		// Write out the object begin character

		fprintf(fp, "{\n");

		// Go through each member of the object

		currentobjectmember = currentvalue->v.object.objectmember;
		while (currentobjectmember != (struct jsonobjectmember *)0)
		{
			// Display the member name

			for (i = 0; i < (level + 1); i++)
				fprintf(fp, "\t");
			fprintf(fp, "\"%s\" : ", currentobjectmember->name);

			// Recurse into ourselves to display value associated
			// with the member

			if (!writeJSONtofp(currentobjectmember->value, fp, true, (level + 1)))
			{
				// An error occurred - return false

				return false;
			}

			// Now, print the separator, and a newline if there is
			// another element, otherwise just print a newline

			if (currentobjectmember->next != (struct jsonobjectmember *)0)
				fprintf(fp, ",\n");
			else
				fprintf(fp, "\n");

			// Get the next member in the object

			currentobjectmember = currentobjectmember->next;
		}

		// We reached the end of the members in the object, so write
		// the object terminator

		for (i = 0; i < level; i++)
			fprintf(fp, "\t");
		fprintf(fp, "}");
		break;

	default:
		// We got an unknown value, set an error message and return
		// false

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring("Got unknown value type when writing out JSON");
		return false;
		break;
	}

	// Just return true

	return true;
}

/* struct jsonvalue *makeJSONelement (JSON_VALUETYPE valuetype)
**
** This internal function is called to make a new JSON element (value),
** initialize it, and set its internal value type. It returns a pointer to the
** newly created element
*/

struct jsonvalue *makeJSONelement(JSON_VALUETYPE valuetype)
{
	struct jsonvalue *newelement;

	// Create the new element

	newelement = (struct jsonvalue *)malloc(sizeof(struct jsonvalue));
	if (newelement == (struct jsonvalue *)0)
	{
		// An error occurred, and we could not allocate the memory to
		// hold the new JSON element. All we can do is return a NULL
		// pointer, and let upstream code caller(s) handle the error

		jsonerror = JSON_GENERALERROR;
		return (struct jsonvalue *)0;
	}

	// Initialize the memory of the element, record its type, and return it

	memset(newelement, 0, sizeof(struct jsonvalue));
	newelement->valuetype = valuetype;
	return newelement;
}

/* static bool connectelementtoparsedJSON (struct jsonvalue *value)
**
** This function is called to connect the element passed to it to existing
** parsed JSON. It looks to see what is being parsed (array elements, objects,
** etc.) to add the value to the correct point
*/

static bool connecttoparsedJSON(struct jsonvalue *value)
{
	struct jsonvalue *currentvalue;
	struct jsonarrayelement *newarrayelement, *currentarrayelement;
	struct jsonobjectmember *currentobjectmember;

	// Get the current value from the stack

	if ((currentvalue = peekJSONvalue()) == (struct jsonvalue *)0)
	{
		// There is nothing on the stack. If this is the very first
		// value, we update the pointer to the current parsed JSON. If
		// that is already set, though, we have an error as it means we
		// have bad JSON

		if (currentparsedJSON != (JSON)0)
		{
			// There is already parsed JSON. This is an error

			jsonerror = JSON_INTERNALERROR;
			setlastJSONerrorstring("No object or array on stack to connect value to, and there is parsed JSON");
			return false;
		}

		// Make the current parsed JSON point to this initial value

		currentparsedJSON = (JSON)value;
		return true;
	}

	// Check to see what we have. It should be an array or an object.

	switch (currentvalue->valuetype)
	{
	case JSON_VALUE_ARRAY:
		// We need to add the value to the end of the array. We need to
		// create a JSON array element

		if ((newarrayelement = (struct jsonarrayelement *)malloc(sizeof(struct jsonarrayelement))) == (struct jsonarrayelement *)0)
		{
			// We could not allocate memory for the new array
			// element. We return an error

			jsonerror = JSON_GENERALERROR;
			setlastJSONerrorstring("Unable to allocate memory for new JSON array element");
			return false;
		}

		// Initailize the new array element

		memset(newarrayelement, 0, sizeof(struct jsonarrayelement));
		newarrayelement->value = value;

		// Check to see if the array is empty

		currentarrayelement = currentvalue->v.array.jsonarrayelement;
		if (currentarrayelement == (struct jsonarrayelement *)0)
		{
			// This is the first element of the array, so just
			// update the array first member

			currentvalue->v.array.jsonarrayelement = newarrayelement;
		}
		else
		{
			// We need to find the end of the array, so we can add
			// the new element to it

			while (currentarrayelement->next != (struct jsonarrayelement *)0)
				currentarrayelement = currentarrayelement->next;

			// Add the new array element to the end

			newarrayelement->last = currentarrayelement;
			currentarrayelement->next = newarrayelement;
		}
		break;

	case JSON_VALUE_OBJECT:
		// We need to add the value to the member most recently added
		// to the object. Check that the object has at least one member

		currentobjectmember = currentvalue->v.object.objectmember;
		if (currentobjectmember == (struct jsonobjectmember *)0)
		{
			// We have an internal error - there should be at least
			// one object member

			jsonerror = JSON_INTERNALERROR;
			setlastJSONerrorstring("Unable to add member value to object, as object has no members");
			return false;
		}

		// Now, we find the last member in the object

		while (currentobjectmember->next != (struct jsonobjectmember *)0)
			currentobjectmember = currentobjectmember->next;

		// Sanity check that the member value is NULL, otherwise we have
		// an internal error

		if (currentobjectmember->value != (struct jsonvalue *)0)
		{
			// There is already a value assigned to this member

			jsonerror = JSON_INTERNALERROR;
			setlastJSONerrorstring("Unable to add member value to object, as member already has a value");
			return false;
		}

		// Add the value to the member

		currentobjectmember->value = value;
		break;

	default:
		// We should never, ever get here. If we did, it means that
		// we have a corrupted stack

		jsonerror = JSON_INTERNALERROR;
		setlastJSONerrorstring("connecttoparsedJSON: Found value on stack that is not an array or object");
		free_parsed_JSON(currentparsedJSON);
		return false;
	}

	// Just return true

	return true;
}

/* static bool pushJSONvalue (struct jsonvalue *value)
**
** This function is used to push an element (an array or an object) onto a
** stack. This is necessary as arrays and objects can be nested. The stack just
** contains pointers, and is used to track where we are as we parse JSON. The
** stack can grow as required, but never shrinks. The function returns true if
** the push was successful, and false otherwise.
*/

static bool pushJSONvalue(struct jsonvalue *value)
{
	struct jsonvalue **newstack;

	// Check that we got the right type of value to add to the stack (we
	// only add arrays and objects)

	if ((value->valuetype != JSON_VALUE_ARRAY) && (value->valuetype != JSON_VALUE_OBJECT))
	{
		// We got the wrong type of value, and we cannot push it onto
		// the array. Just set an error and then return false

		jsonerror = JSON_INCORRECTVALUETYPE;
		return false;
	}

	// Check if we need to grow the stack

	if (numelementsonJSONstack == JSONstacksize)
	{
		// We need to grow the stack

		JSONstacksize += JSON_STACK_GROW_SIZE;
		if ((newstack = realloc(JSONelementstack, (sizeof(struct jsonvalue *) * JSONstacksize))) == (struct jsonvalue **)0)
		{
			// An error occurred, and we could not allocate memory
			// for the newly expanded stack. Record the error and
			// return false

			jsonerror = JSON_GENERALERROR;
			return false;
		}

		// Copy the newstack to the stack variable we use

		JSONelementstack = newstack;
	}

	// Push the value onto the stack

	JSONelementstack[numelementsonJSONstack++] = value;

	// Return true

	return true;
}

/* static struct jsonvalue *peekJSONvalue (void)
**
** This function is called to peek the value at the top of the stack. It
** returns a pointer to it (which can be used to manipulate it), but does not
** remove it from the top of the stack. If there is nothing on the stack it
** returns a NULL pointer
*/

static struct jsonvalue *peekJSONvalue(void)
{
	// Check that there is anything on the stack, first. If there is
	// nothing,  return NULL, otherwise return the value at the top of the
	// stack

	if (numelementsonJSONstack == 0)
	{
		// Return NULL

		return (struct jsonvalue *)0;
	}

	// Return a pointer to the value at the top of the stack

	return JSONelementstack[(numelementsonJSONstack - 1)];
}

/* static bool popJSONvalue (void)
**
** This function is called to remove a value from the top of the stack. If it
** is successful if returns true, otherwise it returns false. The only way that
** an error can occur is if there is nothing on the stack (or if it has not yet
** been created)
*/

static bool popJSONvalue(void)
{
	// Check that the stack has been initialized

	// Check that the stack has been initialized and that there is an
	// element to pop off the stack

	if ((JSONstacksize == 0) || (numelementsonJSONstack == 0))
	{
		// There is nothing on the stack to pop off

		jsonerror = JSON_INTERNALERROR;
		return false;
	}

	// Pop the value off the stack. We do this simply by decrementing the
	// number of items on the stack

	numelementsonJSONstack--;

	// Return true

	return true;
}

/* static bool addnewstate (struct jsonvalue *arrayorobject)
**
** This function is used to add new state for an array or objecy, allowing a
** user to navigate the elements or members using getfirst... () and
** getnext...() functions
*/

static bool addnewstate(struct jsonvalue *arrayorobject)
{
	struct jsonarrayobjectstate *newstate;

	// Initialize the new state structure

	if ((newstate = malloc(sizeof(struct jsonarrayobjectstate))) == (struct jsonarrayobjectstate *)0)
	{
		// An error occurred, and we could not allocate memory for the
		// new state structure. Set an error and return

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring("Unable to allocate memory for new state structure");
		return false;
	}

	// Initialize the new state structure

	memset(newstate, 0, sizeof(struct jsonarrayobjectstate));
	newstate->arrayorobject = arrayorobject;

	// Make the current array element or object member be the first one as
	// that is what is returned to the user, as this is only ever invoked
	// when the user calls getfirst...()

	if (arrayorobject->valuetype == JSON_VALUE_ARRAY)
		newstate->c.arrayelement = arrayorobject->v.array.jsonarrayelement;
	else
		newstate->c.objectmember = arrayorobject->v.object.objectmember;

	// Now, simply insert the state at the head of the linked list of
	// states we maintain

	newstate->next = arrayobjectstate;
	if (arrayobjectstate != (struct jsonarrayobjectstate *)0)
		arrayobjectstate->last = newstate;
	arrayobjectstate = newstate;

	// Now, return true

	return true;
}

/* static JSON getnextelementormember (struct jsonvalue *arrayorobject)
**
** This function is used to get the next element or member in an array or an
** object, using the state we maintain
*/

static JSON getnextelementormember(struct jsonvalue *arrayorobject)
{
	struct jsonarrayobjectstate *currentstate = arrayobjectstate;

	// Go through the list of state structures to look for the array or
	// object that the user passed to us

	while (currentstate != (struct jsonarrayobjectstate *)0)
	{
		// Check to see if we have matched the array or object passed
		// to us, to the current one

		if (arrayorobject == currentstate->arrayorobject)
		{
			// We found a match for the array or object. We update
			// the state to point to the next one and we return it

			if (arrayorobject->valuetype == JSON_VALUE_ARRAY)
			{
				// Update state to point to the next array
				// element, and return it

				currentstate->c.arrayelement = currentstate->c.arrayelement->next;
				return currentstate->c.arrayelement;
			}
			else
			{
				// Update state to point to the next object
				// member, and return it

				currentstate->c.objectmember = currentstate->c.objectmember->next;
				return currentstate->c.objectmember;
			}
		}

		// Get the next state

		currentstate = currentstate->next;
	}

	// Hmmm, we went through the list of state structures, but found no
	// match. This is an internal error. Set the error and return a NULL

	jsonerror = JSON_INTERNALERROR;
	setlastJSONerrorstring("Unable to find matching array or object in state");
	return (JSON)0;
}

/* static bool deletestate (struct jsonvalue *arrayorobject)
**
** This function is used to delete state for an array or object, when it is no
** longer needed. We delete state when we have reached the end of an array or
** the last member of an object. State is also deleted when parsed JSON is
** deleted
*/

static bool deletestate(struct jsonvalue *arrayorobject)
{
	struct jsonarrayobjectstate *currentstate = arrayobjectstate;

	// Go through the list of states, looking for the one passed to us

	while (currentstate != (struct jsonarrayobjectstate *)0)
	{
		// Check if we have a matching array or object

		if (arrayorobject == currentstate->arrayorobject)
		{
			// We have a matching array or object. We remove it
			// from the linked list my maniuplating the last and
			// next pointers of state structures in front of it and
			// after it in the list (or the pointer to the list if
			// it is the first state in the list)

			if (currentstate->last != (struct jsonarrayobjectstate *)0)
				currentstate->last->next = currentstate->next;
			else
				arrayobjectstate = currentstate->next;

			if (currentstate->next != (struct jsonarrayobjectstate *)0)
				currentstate->next->last = currentstate->last;

			// Free up the state structure

			free(currentstate);

			// Return true

			return true;
		}

		// Get the next state structure

		currentstate = currentstate->next;
	}

	// Hmmm, we should not have gotten here. We did not find the state in
	// the list. This is an internal error. Set it and return false

	jsonerror = JSON_INTERNALERROR;
	setlastJSONerrorstring("Unable to delete state as state not found");
	return false;
}

/* bool parseJSONaddstring (char *string)
**
** This function is called to add a string read from JSON. The string is a
** value, not a member name in an object
*/

bool parseJSONaddstring(char *string)
{
	struct jsonvalue *newstringvalue;

	// Make the new JSON element, a value for a string

	if ((newstringvalue = makeJSONelement(JSON_VALUE_STRING)) == (struct jsonvalue *)0)
	{
		// An error occurred, and we could not create a new value

		setlastJSONerrorstring("Unable to create new string value");
		return false;
	}

	// Set the string

	newstringvalue->v.string = string;

	// Connect the value to the rest of the parsed JSON

	if (!connecttoparsedJSON(newstringvalue))
	{
		// An error occurred. Free the parsed JSON and return false

		free_parsed_JSON(currentparsedJSON);
		return false;
	}

	// Return true

	return true;
}

/* bool parseJSONaddnumber (char *number)
**
** This function is called to add a number value read from JSON. The number is
** passed as a string, and needs to be converted to an int, float, or double as
** appropriate
*/

bool parseJSONaddnumber(char *number)
{
	struct jsonvalue *newnumbervalue;

	// Make the new JSON element, a value for a number

	if ((newnumbervalue = makeJSONelement(JSON_VALUE_NUMBER)) == (struct jsonvalue *)0)
	{
		// An error occurred, and we could not create a new value

		setlastJSONerrorstring("Unable to create new number value");
		return false;
	}

	// We store a number as a string internally. It is converted into an
	// actual number when the user retrieves it

	newnumbervalue->v.number = number;

	// Connect the value to the rest of the parsed JSON

	if (!connecttoparsedJSON(newnumbervalue))
	{
		// An error occurred. Free the parsed JSON and return false

		free_parsed_JSON(currentparsedJSON);
		return false;
	}

	// Return true

	return true;
}

/* void parseJSONaddboolean (bool trueorfalse)
**
** This function is called to add a boolean value read from JSON. The actual
** value (true or false) is passed as an argument
*/

bool parseJSONaddboolean(bool trueorfalse)
{
	struct jsonvalue *newbooleanvalue;

	// Make the new JSON element, a value for a boolean

	if ((newbooleanvalue = makeJSONelement(JSON_VALUE_BOOLEAN)) == (struct jsonvalue *)0)
	{
		// An error occurred, and we could not create a new value

		setlastJSONerrorstring("Unable to create new boolean value");
		return false;
	}

	// Set the boolean value

	newbooleanvalue->v.boolean = trueorfalse;

	// Connect the value to the rest of the parsed JSON

	if (!connecttoparsedJSON(newbooleanvalue))
	{
		// An error occurred. Free the parsed JSON and return false

		free_parsed_JSON(currentparsedJSON);
		return false;
	}

	// Return true

	return true;
}

/* bool parseJSONaddnull (void)
**
** This function is called to add a null value read from JSON
*/

bool parseJSONaddnull(void)
{
	struct jsonvalue *newnullvalue;

	// Make the new JSON element, a value for a null

	if ((newnullvalue = makeJSONelement(JSON_VALUE_NULL)) == (struct jsonvalue *)0)
	{
		// An error occurred, and we could not create a new value

		setlastJSONerrorstring("Unable to create new null value");
		return false;
	}

	// Connect the value to the rest of the parsed JSON

	if (!connecttoparsedJSON(newnullvalue))
	{
		// An error occurred. Free the parsed JSON and return false

		free_parsed_JSON(currentparsedJSON);
		return false;
	}

	// Return true

	return true;
}

/* bool parseJSONaddmembername (char *string)
**
** This function is called to add a member name to the name/value pair in an
** object.
*/

bool parseJSONaddmembername(char *string)
{
	struct jsonvalue *currentobject;
	struct jsonobjectmember *currentmember, *newmember;

	// Get the current object. It should be on top of the stack. We sanity
	// check that we found a value on the top of the stack

	if ((currentobject = peekJSONvalue()) == (struct jsonvalue *)0)
	{
		// An error occurred, and we got nothing off the stack

		jsonerror = JSON_INTERNALERROR;
		setlastJSONerrorstring("Unable to get object to add member name to");
		free_parsed_JSON(currentparsedJSON);
		return false;
	}

	// Check we got an object back

	if (currentobject->valuetype != JSON_VALUE_OBJECT)
	{
		// We got a value back, but it is not an object

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring("Value on stack was not an object");
		free_parsed_JSON(currentparsedJSON);
		return false;
	}

	// Create a new object member

	if ((newmember = malloc(sizeof(struct jsonobjectmember))) == (struct jsonobjectmember *)0)
	{
		// We could not allocate memory for the new JSON object member

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring("Unable to allocate memory for new member");
		free_parsed_JSON(currentparsedJSON);
		return false;
	}

	// Initialize the new member

	memset(newmember, 0, sizeof(struct jsonobjectmember));
	newmember->name = string;

	// Now, add the member. If this is the first member, it will be easy to
	// add to the object

	currentmember = currentobject->v.object.objectmember;
	if (currentmember == (struct jsonobjectmember *)0)
	{
		// This is the first member, so update the object directly

		currentobject->v.object.objectmember = newmember;
	}
	else
	{
		// We need to add the member to the end of the list of members
		// in the object. We need to find the last member in the list
		// of members

		while (currentmember->next != (struct jsonobjectmember *)0)
			currentmember = currentmember->next;

		// Append the member to the list of members

		newmember->last = currentmember;
		currentmember->next = newmember;
	}

	// We are done. Just return true

	return true;
}

/* bool parseJSONobjectbegin ()
**
** This function is called at the beginning of an object found in JSON when it
** is being parsed
*/

bool parseJSONobjectbegin()
{
	bool retval;

	// Create a new object element, using the common function for arrays
	// and objects

	if ((retval = parseJSONarrayobjectbegin(JSON_VALUE_OBJECT)) == false)
	{
		// An error occurred

		setlastJSONerrorstring("Unable to create new object");
	}

	// Return

	return retval;
}

/* bool parseJSONOobjectend (void)
**
** This function is called when the end of an object is found in JSON when
** parsing it
*/

bool parseJSONobjectend(void)
{
	bool retval;

	// We have finishged parsing an object

	if ((retval = parseJSONarrayorobjectend()) == false)
	{
		// An error occurred

		setlastJSONerrorstring("Unable to process end of object");
	}

	// Return

	return retval;
}

/* bool parseJSONarraybegin (void)
**
** This function is called when the beginning of an array is found in JSON
** being parsed
*/

bool parseJSONarraybegin(void)
{
	bool retval;

	// Create a new array element, using the common function for arrays
	// and objects

	if ((retval = parseJSONarrayobjectbegin(JSON_VALUE_ARRAY)) == false)
	{
		// An error occurred

		setlastJSONerrorstring("Unable to create new array");
	}

	// Return

	return retval;
}

/* bool parseJSONarrayend (void)
**
** This function is called when the end of an array is found in JSON being
** parsed
*/

bool parseJSONarrayend(void)
{
	bool retval;

	// We have finished parsing the array

	if ((retval = parseJSONarrayorobjectend()) == false)
	{
		// An error occurred

		setlastJSONerrorstring("Unable to process end of array");
	}

	// Return

	return retval;
}

/* static bool parsearrayobjectbegin (JSON_VALUETYPE valuetype)
**
** This function is called from parsearraybegin () or parseobjectbegin (), as
** the logic is the same for both
*/

static bool parseJSONarrayobjectbegin(JSON_VALUETYPE valuetype)
{
	struct jsonvalue *newobject;

	// Create a new element of the value type passed to us

	newobject = makeJSONelement(valuetype);
	if (newobject == (struct jsonvalue *)0)
	{
		// An error occurred, and we cannot proceed

		jsonerror = JSON_GENERALERROR;
		free_parsed_JSON(currentparsedJSON);
		return false;
	}

	// Connect the object to existing JSON (if any)

	if (!connecttoparsedJSON(newobject))
	{
		// An error occurred. Free the parsed JSON and return false

		free_parsed_JSON(currentparsedJSON);
		return false;
	}

	// Now, push the object onto the stack, as we need to keep track of it
	// in case it is a nested object (or an array or object is an element
	// or member of this array or object)

	if (!pushJSONvalue(newobject))
	{
		// An error occurred. We cannot proceed. All we can do is set
		// the internal error and try to clean up

		jsonerror = JSON_INTERNALERROR;
		free_parsed_JSON(currentparsedJSON);
		return false;
	}

	// Return true

	return true;
}

/* static bool parseJSONarrayorobjectend (void)
**
** This function is a common function called from parseJSONarrayend () or
** parseJSONobjectend ()
*/

static bool parseJSONarrayorobjectend(void)
{
	bool retval;

	// Pop the element off the stack

	if ((retval = popJSONvalue()) == false)
	{
		// We encountered an error

		jsonerror = JSON_INTERNALERROR;
		free_parsed_JSON(currentparsedJSON);
	}

	// Return

	return retval;
}

/*
** The following functions are for library users to query the parsed JSON, and
** get elements, members, values, etc.
*/

/* JSON_VALUETYPE get_JSON_value_type (const JSON parsedJSON)
**
** This function is used by the user to find out what the value type is of the
** JSON blob. Note that arrays and objects can have nested values (elements and
** members), and the user has to use the API we provide to get them. We return
** 0 is there is an error
*/

JSON_VALUETYPE get_JSON_value_type(const JSON parsedJSON)
{
	struct jsonvalue *currentvalue = (struct jsonvalue *) parsedJSON;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *)0)
	{
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON to determine what the value type is");
		return (JSON_VALUETYPE)0;
	}

	// Now, check that we got a valid value type

	if ((currentvalue->valuetype < JSON_VALUE_STRING) || (currentvalue->valuetype > JSON_VALUE_NULL))
	{
		// We do not have a valid JSON value type. The most likely
		// cause is that the user passed us a pointer so something that
		// is not parsed JSON, or is JSON that was corrupted

		jsonerror = JSON_BADJSON;
		setlastJSONerrorstring("Unknown and invalid JSON value type");
		return (JSON_VALUETYPE)0;
	}

	// Return the JSON value type

	return currentvalue->valuetype;
}

/* bool is_JSON_array_empty (const JSON parsedJSONarray)
**
** This function is used by the user to check if an array is empty
*/

bool is_JSON_array_empty(const JSON parsedJSONarray)
{
	struct jsonvalue *currentvalue = (struct jsonvalue *) parsedJSONarray;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *)0)
	{
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to get first element in array");
		return 0;
	}

	// Now, check that we got a valid value type (an array)

	if (currentvalue->valuetype != JSON_VALUE_ARRAY)
	{
		// We were not passed an array! Set the error and return

		jsonerror = JSON_NOTANARRAY;
		setlastJSONerrorstring("Passed JSON is not an array");
		return (JSON)0;
	}

	// Now, return true if the array is empty, and false otherwise

	return ((currentvalue->v.array.jsonarrayelement == (struct jsonarrayelement *)0) ? true : false);
}

/* const JSON get_first_element_in_JSON_array (const JSON parsedJSONarray)
**
** Get the first element in the array that was passed to us. We return a NULL
** is there are no elements in the array, or if the user passed us a NULL
** value for parsedJSON or the parsedJSON is not an array. The user needs to
** check the JSON error to see if the array is empty or if there is some other
** issue
*/

const JSON get_first_element_in_JSON_array(const JSON parsedJSONarray)
{
	struct jsonvalue *currentvalue = (struct jsonvalue *) parsedJSONarray;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *)0)
	{
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to get first element in array");
		return (JSON)0;
	}

	// Now, check that we got a valid value type (an array)

	if (currentvalue->valuetype != JSON_VALUE_ARRAY)
	{
		// We were not passed an array! Set the error and return

		jsonerror = JSON_NOTANARRAY;
		setlastJSONerrorstring("Passed JSON is not an array");
		return (JSON)0;
	}

	// Make sure that the array is not empty

	if (currentvalue->v.array.jsonarrayelement == (struct jsonarrayelement *)0)
	{
		// The array is empty. This is not an error. Just return a NULL

		return (JSON)0;
	}

	// We need to record the JSON array element so that we can track where
	// we are in the array. As arrays can be nested (in arrays or in
	// objects in arrays) we need to maintain state for each

	if (!addnewstate(currentvalue))
	{
		// An error occurred! We could not create a state for the array

		return (JSON)0;
	}

	// Return the first element in the array

	return (JSON)currentvalue->v.array.jsonarrayelement->value;
}

/* const JSON get_next_element_in_JSON_array (const JSON parsedJSON)
**
** This function is used to get the next element in the JSON array passed to
** it as an argument. If getfirst...() is not called beforehand, the result is
** unpredicatble
*/

const JSON get_next_element_in_JSON_array(const JSON parsedJSONarray)
{
	struct jsonvalue *currentvalue = (struct jsonvalue *) parsedJSONarray;
	struct jsonarrayelement *nextelement;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *)0)
	{
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to get next element in array");
		return (char *)0;
	}

	// Now, check that we got a valid value type (an array)

	if (currentvalue->valuetype != JSON_VALUE_ARRAY)
	{
		// We were not passed an array! Set the error and return

		jsonerror = JSON_NOTANARRAY;
		setlastJSONerrorstring("Passed JSON is not an array");
		return (JSON)0;
	}

	// Make sure that the array is not empty

	if (currentvalue->v.array.jsonarrayelement == (struct jsonarrayelement *)0)
	{
		// The array is empty. This is not an error. Just return a NULL

		return (JSON)0;
	}

	// Okay, now we need to get the next element in the array. We call the
	// common function we use to get elements and members

	nextelement = (struct jsonarrayelement *)getnextelementormember(currentvalue);

	// If the element is NULL we need to delete the state as there are no
	// more elements

	if (nextelement == (struct jsonarrayelement *)0)
	{
		// An internal error occurred or there are no more array
		// elements to return. Delete the state and return NULL

		deletestate(currentvalue);
		return (JSON)0;
	}

	// Return the value

	return (JSON)nextelement->value;
}

/* const char *get_first_membername_in_JSON_object (const JSON parsedJSONobject)
**
** This function is called to get the first member name in an object. Member
** names are in the order they were parsed (first to last)
*/

const unsigned char *get_first_membername_in_JSON_object(const JSON parsedJSONobject)
{
	struct jsonvalue *currentvalue = (struct jsonvalue *) parsedJSONobject;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *)0)
	{
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to get first element in array");
		return (unsigned char *) 0;
	}

	// Now, check that we got a valid value type (an object)

	if (currentvalue->valuetype != JSON_VALUE_OBJECT)
	{
		// We were not passed an object! Set the error and return

		jsonerror = JSON_NOTANOBJECT;
		setlastJSONerrorstring("Passed JSON is not an object");
		return (unsigned char *) 0;
	}

	// Make sure that the object has members

	if (currentvalue->v.object.objectmember == (struct jsonobjectmember *)0)
	{
		// The object has no members. This is not an error. Return NULL

		return (unsigned char *) 0;
	}

	// We need to record the JSON object member so that we can track where
	// we are in the object. As objects can be nested we need to maintain
	// state for each

	if (!addnewstate(currentvalue))
	{
		// An error occurred! We could not create a state for the
		// object

		return (unsigned char *) 0;
	}

	// Return the first member name in the object

	return decodeJSONstring (currentvalue->v.object.objectmember->name);
}

/* const unisgned char *get_nextmembername_in_JSON_object (const JSON parsedJSONobject)
**
** This function is called to get the next member name in the object. It uses
** the internal state we maintain to know where we are in the member names in
** this object. If getfirst...() has not been called previously, the results
** are unpredictable
*/

const unsigned char *get_next_membername_in_JSON_object(const JSON parsedJSONobject)
{
	struct jsonvalue	*currentvalue = (struct jsonvalue *) parsedJSONobject;
	struct jsonobjectmember *nextmember;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *)0)
	{
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to get next element in array");
		return (unsigned char *)0;
	}

	// Now, check that we got a valid value type (an object)

	if (currentvalue->valuetype != JSON_VALUE_OBJECT)
	{
		// We were not passed an array! Set the error and return

		jsonerror = JSON_NOTANOBJECT;
		setlastJSONerrorstring("Passed JSON is not an object");
		return (unsigned char *)0;
	}

	// Make sure that the object is not empty

	if (currentvalue->v.object.objectmember == (struct jsonobjectmember *)0)
	{
		// The object is empty. This is not an error. Just return a NULL

		return (unsigned char *) 0;
	}

	// Okay, now we need to get the next member in the object. We call the
	// common function we use to get elements and members

	nextmember = (struct jsonobjectmember *)getnextelementormember(currentvalue);

	// If the element is NULL we need to delete the state as there are no
	// more elements

	if (nextmember == (struct jsonobjectmember *)0)
	{
		// An internal error occurred or there are no more object
		// members to return. Delete the state and return NULL

		deletestate(currentvalue);
		return (unsigned char *) 0;
	}

	// Return the next member name

	return decodeJSONstring (nextmember->name);
}

/* const JSON get_membervalue_in_JSON_object (const JSON parsedJSON, const unsigned char *membername)
**
** This function is called to get the member value in a JSON object identified
** by its name. This assumes that every member in an object has a unique name,
** as only the first value with a matching name is found
*/

const JSON get_membervalue_in_JSON_object(const JSON parsedJSON, const unsigned char *membername)
{
	char			*encodedmembername;
	struct jsonvalue	*currentvalue = (struct jsonvalue *) parsedJSON;
	struct jsonobjectmember *currentmember;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *)0)
	{
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to get member value");
		return (JSON) 0;
	}

	// Now, check that we got a valid value type (an object)

	if (currentvalue->valuetype != JSON_VALUE_OBJECT)
	{
		// We were not passed an array! Set the error and return

		jsonerror = JSON_NOTANOBJECT;
		setlastJSONerrorstring("Passed JSON is not an object");
		return (JSON) 0;
	}

	// We need to encode the member name

	if ((encodedmembername = encodeJSONstring (membername)) == (char *) 0) {
		// An error occurred, and we could not encode the member name

		return (JSON) 0;
	}

	// Go through the members in the object, looking for a matching name

	currentmember = currentvalue->v.object.objectmember;
	while (currentmember != (struct jsonobjectmember *)0)
	{
		// Check if the member name is the same as the name requested

		if (!strcmp(currentmember->name, encodedmembername))
		{
			// We found a matching name, just return the value

			free (encodedmembername);
			return (JSON)currentmember->value;
		}

		// Get the next member in the object

		currentmember = currentmember->next;
	}

	// We did not find a matching member name. This is not an error, just
	// return NULL JSON

	free (encodedmembername);
	return (JSON)0;
}

/* unsigned char *get_JSON_string (const JSON parsedJSONstring)
**
** Get the JSON string from the string value. Note that we need to convert it
** to process any escape characters, etc.
*/

unsigned char *get_JSON_string(const JSON parsedJSONstring)
{
	struct jsonvalue *currentvalue = (struct jsonvalue *)parsedJSONstring;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *) 0) {
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to get string value");
		return (unsigned char *) 0;
	}

	// Now, check that we got a valid value type (string)

	if (currentvalue->valuetype != JSON_VALUE_STRING) {
		// We were not passed a string! Set the error and return

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring("Passed JSON is not a string");
		return (unsigned char *) 0;
	}

	// Return the decoded string

	return decodeJSONstring (currentvalue ->v.string);
}

/* JSON_VALUENUMBERTYPE get_JSON_number_type (const JSON parsedJSON)
**
** This function is used to determine what the number we have is. It analyzes
** the string representation of the number to figure out if it is an integer
** or a double
*/

JSON_VALUENUMBERTYPE get_JSON_number_type(const JSON parsedJSON)
{
	struct jsonvalue *currentvalue;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *) 0) {
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to determine number type");
		return (JSON_VALUENUMBERTYPE) 0;
	}

	// Now, check that we got a valid value type (number)

	if (currentvalue->valuetype != JSON_VALUE_NUMBER) {
		// We were not passed a number! Set the error and return

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring("Passed JSON is not a number");
		return (JSON_VALUENUMBERTYPE) 0;
	}

	// Try to figure out what we have

	if ((index(currentvalue->v.number, (int)'.') != 0) || (index(currentvalue->v.number, (int)'e') != 0) || (index(currentvalue->v.number, (int)'E') != 0))
		return JSON_VALUE_NUMBER_DOUBLE;
	else
		return JSON_VALUE_NUMBER_INTEGER;
}

/* long get_JSON_number_long (const JSON parsedJSON)
**
** Return the JSON number as a long. If the number is actually a double, the
** results are unpredicatble.
*/

long get_JSON_number_long(const JSON parsedJSON)
{
	struct jsonvalue *currentvalue;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *) 0) {
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to return a long number");
		return 0;
	}

	// Now, check that we got a valid value type (number)

	if (currentvalue->valuetype != JSON_VALUE_NUMBER) {
		// We were not passed a number! Set the error and return

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring("Passed JSON is not a number");
		return 0;
	}

	// Return the number as a long

	return strtol (currentvalue->v.number, (char **)0, 10);
}

/* double get_JSON_number_double (const JSON parsedJSON)
**
** The user wants us to return the number as a double. This should work even if
** the number is, in fact, an integer
*/

double get_JSON_number_double(const JSON parsedJSON)
{
	struct jsonvalue *currentvalue;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *) 0) {
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to return a double number");
		return 0;
	}

	// Now, check that we got a valid value type (number)

	if (currentvalue->valuetype != JSON_VALUE_NUMBER) {
		// We were not passed a number! Set the error and return

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring("Passed JSON is not a number");
		return 0;
	}

	// Return the number as a double

	return strtod (currentvalue->v.number, (char **)0);
}

/* char *get_JSON_number_string (const JSON parsedJSON)
**
** The user wants us to return the number as a string. This is easy as the
** internal representation of a JSON number is a string.
*/

char *get_JSON_number_string(const JSON parsedJSON)
{
	struct jsonvalue *currentvalue;
	char *outstring;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *) 0) {
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to return a string number");
		return (char *) 0;
	}

	// Now, check that we got a valid value type (number)

	if (currentvalue->valuetype != JSON_VALUE_NUMBER) {
		// We were not passed a number! Set the error and return

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring("Passed JSON is not a number");
		return (char *) 0;
	}

	// Allocate memory for the string we return to the user

	if ((outstring = (char *) malloc (strlen(currentvalue->v.number) + 1)) == (char *) 0) {
		// We encountered an error allocating memory for the string. We
		// set the error, and return a NULL pointer

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring("Unable to allocate memory for number string");
		return (char *) 0;
	}

	// Copy over the memory and return it

	return strcpy(outstring, currentvalue->v.number);
}

/* bool get_JSON_boolean (const JSON parsedJSON)
**
** This function is used to return the value of a boolean. If there is an error
** the return is false, so the user needs to check the internal error state to
** make sure one did not occur
*/

bool get_JSON_boolean(const JSON parsedJSON)
{
	struct jsonvalue *currentvalue;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *) 0) {
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to return a boolean value");
		return false;
	}

	// Now, check that we got a valid value type (number)

	if (currentvalue->valuetype != JSON_VALUE_BOOLEAN) {
		// We were not passed a number! Set the error and return

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring("Passed JSON is not a boolean value");
		return false;
	}

	// Return the boolean value

	return currentvalue->v.boolean;
}

/* JSON create_JSON_array (void)
**
** This function is called by the user to create a new JSON array. It returns
** a point to the JSON array created, which is NULL if an error occurred
*/

JSON create_JSON_array (void)
{
	struct jsonvalue *newarray;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Create a new array element

	if ((newarray = makeJSONelement (JSON_VALUE_ARRAY)) == (struct jsonvalue *) 0) {
		// An error occurred, and we could not create an array element,
		// so record the error and return a NULL value

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to create JSON array");
	}

	return (JSON) newarray;
}

/* JSON create_JSON_object (void)
**
** This function is called to create a new JSON object. The created object is
** empty (no members)
*/

JSON create_JSON_object (void)
{
	struct jsonvalue *newobject;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Create a new object element

	if ((newobject = makeJSONelement (JSON_VALUE_OBJECT)) == (struct jsonvalue *) 0) {
		// An error occurred, and we could not create an object
		// element, so record the error and return a NULL value

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to create JSON object");
	}

	return (JSON) newobject;
}

/* static char *encodeJSONstring (const unsigned char *string)
**
** This function is used to encode JSON strings, used in the JSON string value
** and in member names in objects
*/

static char *encodeJSONstring (const unsigned char *string)
{
	int	stringlength, i = 0, o = 0, mb1, mb2, mb3, mb4;
	long	utf8charout;
	bool	inputend = false, wellformedutf8;
	char	*encodedstring;

	// Allocate memory for the new string. We make it exactly the same size
	// as the string passed to us (we add one for the NULL character)

	stringlength = strlen ((char *) string) +1;
	if ((encodedstring = malloc (stringlength)) == (char *) 0) {
		// An error occurred, and we could not allocate memory for the
		// string. Just clean up and return

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to allocate memory for JSON string");
		return (char *) 0;
	}

	// Go into a loop, reading the input string and copying it over to the
	// output string. We have to encode it for JSON, so we inspect what we
	// read in

	do {
		// Check if the character just read in needs to be encoded

		switch (string [i]) {
		case '"':
		case '/':
		case '\\':
			// We need to extend the string

			if ((encodedstring = realloc (encodedstring, ++ stringlength)) == (char *) 0) {
				// An error occurred, and we could not add more
				// memory for the string. All we can do is
				// return an error

				free (encodedstring);
				jsonerror = JSON_GENERALERROR;
				setlastJSONerrorstring ("Unable to add memory for JSON string");
				return (char *) 0;
			}

			// Add a quote character to the output string, and then
			// copy over the input character
			
			encodedstring [o ++] = '\\';
			encodedstring [o ++] = string [i ++];
			break;

		case '\b':
			// We need to extend the string

			if ((encodedstring = realloc (encodedstring, ++ stringlength)) == (char *) 0) {
				// An error occurred, and we could not add more
				// memory for the string. All we can do is
				// return an error

				free (encodedstring);
				jsonerror = JSON_GENERALERROR;
				setlastJSONerrorstring ("Unable to add memory for JSON string");
				return (char *) 0;
			}

			// Add a quote character to the output string, and then
			// the character representation of the backspace
			
			encodedstring [o ++] = '\\';
			encodedstring [o ++] = 'b';
			i ++;
			break;


		case '\f':
			// We need to extend the string

			if ((encodedstring = realloc (encodedstring, ++ stringlength)) == (char *) 0) {
				// An error occurred, and we could not add more
				// memory for the string. All we can do is
				// return an error

				free (encodedstring);
				jsonerror = JSON_GENERALERROR;
				setlastJSONerrorstring ("Unable to add memory for JSON string");
				return (char *) 0;
			}

			// Add a quote character to the output string, and then
			// the character representation of the formfeed
			
			encodedstring [o ++] = '\\';
			encodedstring [o ++] = 'f';
			i ++;
			break;

		case '\n':
			// We need to extend the string

			if ((encodedstring = realloc (encodedstring, ++ stringlength)) == (char *) 0) {
				// An error occurred, and we could not add more
				// memory for the string. All we can do is
				// return an error

				free (encodedstring);
				jsonerror = JSON_GENERALERROR;
				setlastJSONerrorstring ("Unable to add memory for JSON string");
				return (char *) 0;
			}

			// Add a quote character to the output string, and then
			// the character representation of the newline
			
			encodedstring [o ++] = '\\';
			encodedstring [o ++] = 'n';
			i ++;
			break;

		case '\r':
			// We need to extend the string

			if ((encodedstring = realloc (encodedstring, ++ stringlength)) == (char *) 0) {
				// An error occurred, and we could not add more
				// memory for the string. All we can do is
				// return an error

				free (encodedstring);
				jsonerror = JSON_GENERALERROR;
				setlastJSONerrorstring ("Unable to add memory for JSON string");
				return (char *) 0;
			}

			// Add a quote character to the output string, and then
			// the character representation of the carriage return
			
			encodedstring [o ++] = '\\';
			encodedstring [o ++] = 'r';
			i ++;
			break;

		case '\t':
			// We need to extend the string

			if ((encodedstring = realloc (encodedstring, ++ stringlength)) == (char *) 0) {
				// An error occurred, and we could not add more
				// memory for the string. All we can do is
				// return an error

				free (encodedstring);
				jsonerror = JSON_GENERALERROR;
				setlastJSONerrorstring ("Unable to add memory for JSON string");
				return (char *) 0;
			}

			// Add a quote character to the output string, and then
			// the character representation of the horiaontal tab
			
			encodedstring [o ++] = '\\';
			encodedstring [o ++] = 't';
			i ++;
			break;

		case 0:
			// The character is the NULL character (end of string),
			// so set inputend to true and fall through (so it is
			// copied over)

			inputend = true;

		default:
			// Check if we need to encode this character with the
			// '\u' escape

			if (string [i] > 0x7F) {
				// We treat this character and the one
				// following it as 'Basic Multilingual Plane'
				// (U+0000 - U+FFFF). We follow the rules found
				// in ISO/IEC 10646:2020. We need to extend the
				// encoded string. We can easily calculate the
				// extra space we need by looking at the top
				// three bits

				if ((string [i] & 0xE0) == 0xE0) {
					// This is a three octet Basic
					// Multilingual Plane character. We
					// check that the octets are well-
					// formed

					wellformedutf8 = true;
					switch (string [i]) {
					case 0xE0:
						if ((string [i +1] < 0xA0) || (string [i +1] > 0xBF))
							wellformedutf8 = false;
						break;

					case 0xED:
						if ((string [i +1] < 0x80) || (string [i +1] > 0x9F))
							wellformedutf8 = false;
						break;

					default:
						if ((string [i +1] < 0x80) || (string [i +1] > 0xBF))
							wellformedutf8 = false;
						break;
					}

					if ((string [i +2] < 0x80) || (string [i +2] > 0xBF))
						wellformedutf8 = false;

					if (! wellformedutf8) {
						// We have a badly formed UTF-8

						jsonerror = JSON_INVALIDSTRING;
						setlastJSONerrorstring ("Three-octet UTF-8 ill-formed");
						free (encodedstring);
						return (char *) 0;
					}

					// Extend the string length by three
					// characters

					stringlength += 3;

					// Build the UTF-8 character in memory

					utf8charout =
						((string [i] & ~0xE0) << 12) +
						((string [i +1] & ~0x80) << 6) +
						(string [i +2] & ~0x80);
					i += 3;
				}
				else if ((string [i] & 0xC0) == 0xC0) {
					// This is a two octet Basic
					// Multilingual Plane character. We
					// check that the octets are well-
					// formed

					if (((string [i] < 0xC2) || (string [i] > 0xDF)) ||
					    ((string [i +1] < 0x80) || (string [i +1] > 0xBF))) {
						// We have a badly formed UTF-8

						jsonerror = JSON_INVALIDSTRING;
						setlastJSONerrorstring ("Two-octet UTF-8 ill-formed");
						free (encodedstring);
						return (char *) 0;
					}

					// Extend the string length by four
					// characters

					stringlength += 4;

					// Build the UTF-8 character in memory

					utf8charout =
						((string [i] & ~0xC0) << 6) +
						(string [i +1] & ~0x80);
					i += 2;
				}
				else {
					// We cannot encode UTF-8 characters
					// above 0xFFFF

					free (encodedstring);
					jsonerror = JSON_INVALIDSTRING;
					setlastJSONerrorstring ("UTF-8 character above 0xFFFF found");
					return (char *) 0;
				}

				// Extend the encoded string

				if ((encodedstring = realloc (encodedstring, stringlength)) == (char *) 0) {
					// An error occurred, and we could not
					// extend the string

					free (encodedstring);
					jsonerror = JSON_GENERALERROR;
					setlastJSONerrorstring ("Unable to add memory for JSON string");
					return (char *) 0;
				}

				// Now, write out the encoding sequence,
				// starting with '\u'

				encodedstring [o ++] = '\\';
				encodedstring [o ++] = 'u';

				// Now, write out the hexadecimal characters

				mb1 = ((utf8charout & 0xF000) >> 12);
				mb2 = ((utf8charout & 0x0F00) >> 8);
				mb3 = ((utf8charout & 0x00F0) >> 4);
				mb4 = (utf8charout & 0x000F);

				encodedstring [o ++] = ((mb1 > 9) ? (mb1 + 'A' - 10) : (mb1 + '0'));
				encodedstring [o ++] = ((mb2 > 9) ? (mb2 + 'A' - 10) : (mb2 + '0'));
				encodedstring [o ++] = ((mb3 > 9) ? (mb3 + 'A' - 10) : (mb3 + '0'));
				encodedstring [o ++] = ((mb4 > 9) ? (mb4 + 'A' - 10) : (mb4 + '0'));
			}
			else {
				// Copy over the input character to the output string

				encodedstring [o ++] = string [i ++];
			}
			break;
		}
	} while (! inputend);

	// Return the string

	return encodedstring;
}

/* static unsigned char *decodeJSONstring (const char *string)
**
** This function is called to decode an encoded JSON string, used when getting
** a string value or a member name
*/

static unsigned char *decodeJSONstring (const char *string)
{
	unsigned char	*decodedstring = (unsigned char *) 0;
	unsigned long	utf8charin;
	int		i = 0, o = 0;

	// Allocate memory for the string we will return to the user

	if ((decodedstring = (unsigned char *) malloc (strlen (string) + 1)) == (unsigned char *) 0) {
		// We have an error - we could not allocate memory for the
		// string we return to the user. Set an error and return a NULL
		// pointer

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring("Unable to allocate memory for string");
		return (unsigned char *) 0;
	}

	// Go through the string looking for escape characters while we copy
	// over the JSON string to the string we return to the user. When we
	// find an escape character we process it

	while (string[i] != (char)0) {
		// Check if this is an escape character

		if (string[i] == '\\') {
			// We have an escape character. Process the next
			// character to see what we need to copy to the
			// output string

			switch (string [++i]) {
			case '"':
				// Copy over a double quotation

				decodedstring [o++] = '"';
				break;

			case '\\':
				// Copy over a reverse solidus

				decodedstring [o++] = '\\';
				break;

			case '/':
				// Copy over a solidus

				decodedstring [o++] = '/';
				break;

			case 'b':
				// Copy over a backspace

				decodedstring [o++] = 0x08;
				break;

			case 'f':
				// Copy over a formfeed

				decodedstring [o++] = 0x0c;
				break;

			case 'n':
				// Copy over a newline

				decodedstring [o++] = 0x0a;
				break;

			case 'r':
				// Copy over a carriage return

				decodedstring [o++] = 0x0d;
				break;

			case 't':
				// Copy over a horizontal tab

				decodedstring [o++] = 0x09;
				break;

			case 'u':
				// The next four characters are hexadecimal
				// digits we need to turn into Unicode. Read in
				// the next four bytes and build a code point

				utf8charin =
				    (((string[(i +1)] > '9') ? ((string[(i +1)] & ~0x20) - 'A' + 10) : (string[(i +1)] - '0')) << 12) +
				    (((string[(i +2)] > '9') ? ((string[(i +2)] & ~0x20) - 'A' + 10) : (string[(i +2)] - '0')) << 8) +
				    (((string[(i +3)] > '9') ? ((string[(i +3)] & ~0x20) - 'A' + 10) : (string[(i +3)] - '0')) << 4) +
				    ((string[(i +4)] > '9') ? ((string[(i +4)] & ~0x20) - 'A' + 10) : (string[(i +4)] - '0'));

				// Now, write out the characters based on the
				// code point

				if (utf8charin < 0x007F) {
					// This is a single character out

					decodedstring [o ++] = (unsigned char) utf8charin;
				}
				else if (utf8charin < 0x07FF) {
					// This is two characters out

					decodedstring [o ++] = (0xC0 | ((utf8charin & ~0x003F) >> 6));
					decodedstring [o ++] = (0x80 | (utf8charin & 0x003F));
				}
				else if ((utf8charin < 0xD7FF) || ((utf8charin > 0xE000) && (utf8charin < 0xFFFF))) {
					// This is three characters out

					decodedstring [o ++] = (0xE0 | ((utf8charin & ~0x0FFF) >> 12));
					decodedstring [o ++] = (0x80 | ((utf8charin & ~0xF03F) >> 6));
					decodedstring [o ++] = (0x80 | (utf8charin & 0x003F));
				}
				else {
					// We got an illegal Basic Multiplane
					// Character 

					jsonerror = JSON_BADJSON;
					setlastJSONerrorstring ("The \\u code sequence is invalid (0xD800 - 0xDFFF)");
					free (decodedstring);
					return (unsigned char *) 0;
					break;
				}

				// Update the input string counter

				i += 4;
				break;

			default:
				// We have bad JSON. We should never get here
				// as our lexical analyzer should have caught
				// it

				jsonerror = JSON_INVALIDSTRING;
				setlastJSONerrorstring("The JSON string contains an invalid escape sequence");
				free(decodedstring);
				return (unsigned char *)0;
				break;
			}

			// Increment the input string counter

			i++;
		}
		else
		{
			// Copy over the character

			decodedstring[o++] = string[i++];
		}
	}

	// Add a trailing NULL to the output string, and return it

	decodedstring[o] = (char)0;
	return decodedstring;
}

/* JSON create_JSON_string (const char *string)
**
** Create a new JSON string from the string passed to the function. Control
** characters such as form feed, newline, etc. are escaped, as are others as
** required by the JSON standard. We do not convert multibyte characters
*/

JSON create_JSON_string (const unsigned char *string)
{
	struct jsonvalue	*newstring;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Create a new string element and assign the encoded string to it

	if ((newstring = makeJSONelement (JSON_VALUE_STRING)) == (struct jsonvalue *) 0) {
		// An error occurred, and we could not create a string element,
		// so record the error and return a NULL value

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to create JSON string");
		return (JSON) 0;
	}
	if ((newstring ->v.string = encodeJSONstring (string)) == (char *) 0) {
		// An error occurred - just return NULL JSON

		return (JSON) 0;
	}

	// Return the JSON string. We are are finished

	return (JSON) newstring;
}

/* JSON create_JSON_number (const char *number)
**
** We need to create a new JSON number. JSON numbers are stored as strings, so
** we just allocate memory for the string representation of the number and copy
** it over
*/

JSON create_JSON_number (const char *number)
{
	struct jsonvalue	*newnumber;
	int			stringlength;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Create a new number element

	if ((newnumber = makeJSONelement (JSON_VALUE_NUMBER)) == (struct jsonvalue *) 0) {
		// An error occurred, and we could not create a number element,
		// so record the error and return a NULL value

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to create JSON number");
		return (JSON) 0;
	}

	// Allocate memory for the number. Numbers are stored as strings
	// internally

	stringlength = strlen (number) +1;
	if ((newnumber ->v.number = malloc (stringlength)) == (char *) 0) {
		// An error occurred, and we could not allocate memory for the
		// string. Just clean up and return

		free (newnumber);
		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to allocate memory for JSON number");
		return (JSON) 0;
	}

	// Copy the number over

	strcpy (newnumber ->v.number, number);

	// Return the number

	return (JSON) newnumber;
}

/* JSON create_JSON_number_long (long number)
**
** We need to create a new JSON number. Numbers are stored as strings, so we
** need to convert the integer passed to us into a string and store that
*/

JSON create_JSON_number_long (long number)
{
	struct jsonvalue	*newnumber;
	int			stringlength;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Create a new number element

	if ((newnumber = makeJSONelement (JSON_VALUE_NUMBER)) == (struct jsonvalue *) 0) {
		// An error occurred, and we could not create a number element,
		// so record the error and return a NULL value

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to create JSON number");
		return (JSON) 0;
	}

	// Calculate the size of the string necessary to store the number. We
	// add two, instead of one, to account for the NULL character at the
	// end of the string

	stringlength = (int) floor (log10 ((double) number)) +2;

	// Allocate memory for the number

	if ((newnumber ->v.number = malloc (stringlength)) == (char *) 0) {
		// An error occurred, and we could not allocate memory for the
		// string. Just clean up and return

		free (newnumber);
		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to allocate memory for JSON long number");
		return (JSON) 0;
	}

	// Print number into the string

	if (sprintf (newnumber ->v.number, "%ld", number) < 0) {
		// An error occurred, and we could not format the number

		free (newnumber);
		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to format JSON long number");
		return (JSON) 0;
	}

	// Return the number

	return (JSON) newnumber;
}

/* JSON create_JSON_number_double (double number)
**
**
** We need to create a new JSON number. Numbers are stored as strings, so we
** need to convert the double passed to us into a string and store that
*/

JSON create_JSON_number_double (double number)
{
	struct jsonvalue	*newnumber;
	int			stringlength;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Create a new number element

	if ((newnumber = makeJSONelement (JSON_VALUE_NUMBER)) == (struct jsonvalue *) 0) {
		// An error occurred, and we could not create a number element,
		// so record the error and return a NULL value

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to create JSON number");
		return (JSON) 0;
	}

	// Calculate the size of the string necessary to store the number. We
	// add two, instead of one, to account for the NULL character at the
	// end of the string

	stringlength = (int) floor (log10 (number)) +2;

	// Allocate memory for the number

	if ((newnumber ->v.number = malloc (stringlength)) == (char *) 0) {
		// An error occurred, and we could not allocate memory for the
		// string. Just clean up and return

		free (newnumber);
		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to allocate memory for JSON double number");
		return (JSON) 0;
	}

	// Print number into the string

	if (sprintf (newnumber ->v.number, "%le", number) < 0) {
		// An error occurred, and we could not format the number

		free (newnumber);
		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to format JSON double number");
		return (JSON) 0;
	}

	// Return the number

	return (JSON) newnumber;
}

/* JSON create_JSON_boolean (bool trueorfalse)
**
** We need to create a JSON boolean element. We set the value to the argument
** passed to us
*/

JSON create_JSON_boolean (bool trueorfalse)
{
	struct jsonvalue *newboolean;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Create a new boolean element

	if ((newboolean = makeJSONelement (JSON_VALUE_BOOLEAN)) == (struct jsonvalue *) 0) {
		// An error occurred, and we could not create a boolean
		// element, so record the error and return a NULL value

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to create JSON boolean");
		return (JSON) 0;
	}

	// Set the value, and return it

	newboolean ->v.boolean = trueorfalse;
	return (JSON) newboolean;
}

/* JSON create_JSON_null (void)
**
** We need to create a new JSON null element
*/

JSON create_JSON_null (void)
{
	struct jsonvalue *newnull;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Create a new null element

	if ((newnull = makeJSONelement (JSON_VALUE_NULL)) == (struct jsonvalue *) 0) {
		// An error occurred, and we could not create an null element,
		// so record the error and return a NULL value

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to create JSON null");
	}

	// Return the element (or a NULL)

	return (JSON) newnull;
}

/* JSON add_element_to_JSON_array (JSON array, JSON value)
**
** We need to add a value to the JSON array. We always append new elements
** to the end of the array
*/

JSON add_element_to_JSON_array (JSON array, JSON value)
{
	struct jsonvalue	*currentvalue = (struct jsonvalue *) array;
	struct jsonarrayelement *arrayelement, *newarrayelement;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *) 0) {
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to add element to array");
		return (char *) 0;
	}

	// Now, check that we got a valid value type (array)

	if (currentvalue->valuetype != JSON_VALUE_ARRAY) {
		// We were not passed a number! Set the error and return

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring("Passed JSON is not an array");
		return (char *) 0;
	}

	// Create a new array element and assign the value to it

	if ((newarrayelement = (struct jsonarrayelement *) malloc (sizeof (struct jsonarrayelement))) == (struct jsonarrayelement *) 0) {
		// An error occurred, and we could not allocate memory for the
		// new array element

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to create new array element");
		return (JSON) 0;
	}
	memset (newarrayelement, 0, sizeof (struct jsonarrayelement));
	newarrayelement ->value = value;

	// Get the first element in the array (if any)

	arrayelement = currentvalue ->v.array.jsonarrayelement;

	// Check to see if the array is empty

	if (arrayelement == (struct jsonarrayelement *) 0) {
		// The array is empty so just make the new element the first
		// in the array, and then return the array back

		currentvalue ->v.array.jsonarrayelement = newarrayelement;
		return array;
	}

	// Go through the array elements, to find the last one so we can append
	// the new array element

	while (arrayelement ->next != (struct jsonarrayelement *) 0)
		arrayelement = arrayelement ->next;

	// Append the new array element to the array

	newarrayelement ->last = arrayelement;
	arrayelement ->next = newarrayelement;

	// Return the array

	return array;
}

/* JSON add_member_to_JSON_object (JSON object, const char *name, JSON value)
**
** We need to add a member to an object. The member is identified by the name
** and there is no check that the name is not already present in the object. We
** always add members after any existing members in the object
*/

JSON add_member_to_JSON_object (JSON object, const unsigned char *name, JSON value)
{
	struct jsonvalue	*currentvalue = (struct jsonvalue *) object;
	struct jsonobjectmember	*objectmember, *newobjectmember;

	// Make sure the internal error variable is clear

	jsonerror = 0;

	// Sanity check that we got something other than a NULL pointer

	if (currentvalue == (struct jsonvalue *) 0) {
		// Whoops, the user passed us a NULL value

		jsonerror = JSON_NOJSON;
		setlastJSONerrorstring("No JSON, so unable to add member to object");
		return (char *) 0;
	}

	// Now, check that we got a valid value type (object)

	if (currentvalue->valuetype != JSON_VALUE_OBJECT) {
		// We were not passed a number! Set the error and return

		jsonerror = JSON_INCORRECTVALUETYPE;
		setlastJSONerrorstring("Passed JSON is not an object");
		return (char *) 0;
	}

	// Allocate memory for the new object member, and add the name and
	// value to it

	if ((newobjectmember = (struct jsonobjectmember *) malloc (sizeof (struct jsonobjectmember))) == (struct jsonobjectmember *) 0) {
		// An error occurred, and we could not allocate memory for the
		// new object member

		jsonerror = JSON_GENERALERROR;
		setlastJSONerrorstring ("Unable to create new object member");
		return (JSON) 0;
	}
	memset (newobjectmember, 0, sizeof (struct jsonobjectmember));
	if ((newobjectmember ->name = encodeJSONstring (name)) == (char *) 0) {
		// An error occurred, so clean up and return

		free (newobjectmember);
		return (JSON) 0;
	}
	newobjectmember ->value = value;

	// Get the first member in the object

	objectmember = currentvalue ->v.object.objectmember;

	// Check to see if there are any members in the object

	if (objectmember == (struct jsonobjectmember *) 0) {
		// The object is empty, so add the member as the first and
		// return the object back

		currentvalue ->v.object.objectmember = newobjectmember;
		return object;
	}

	// Go through the members in the object, until we find the last one

	while (objectmember ->next != (struct jsonobjectmember *) 0)
		objectmember = objectmember ->next;

	// Append the member to the list of members

	newobjectmember ->last = objectmember;
	objectmember ->next = newobjectmember;

	// Return the object back 

	return object;
}
