/* File: teststrings.c
**
** Author: John Howie (c) Howie Consulting Inc., 2026. (john@howieconsultinginc.com)
**
** Description
**
**      This test program reads JSON from a buffer in memory, and displays what
** it finds and build the in-memory representation.
**
** Modifications
**
** 2026-09-07   John Howie      Original.
**
*/

# include <stdio.h>
# include <stdbool.h>
# include <string.h>
# include <stdlib.h>
# include "parseJSON.h"


int main (int argc, char *argv []);

int main (int argc, char *argv [])
{
        char            JSONin [] = "[\"abcdefghihklmnopqrstuvwxyz\",\"This is a string with a \\\"quote\\\" in it\",\"This string has a reverse solidus -> \\\\ <- here\",\"This string has a forward solidus -> \\/ <- here\",\"This string has a bb\\backspace (only one 'b')\",\"This string has a formfeed -> \\f <- here\",\"This string has a linefeed -> \\n <- here\",\"This string has a carriage return -> \\r <- here\",\"This string has a horizontal tab -> \\t <- here\",\"This string has Unicode -> \\uABCD <- here\"]";
        struct {
                char    *description;
                char    mbcs [4];
        }               descandchar [] = {
                { "This is a pound: ", { 0xC2, 0xA3, 0x00, 0x00 } },
                { "This is a copyright: ", { 0xC2, 0xA9, 0x00, 0x00 } },
                { "This is a latinae: ", { 0xE1, 0xB4, 0x81, 0x00 } }
        };
        int             i;  
        bool            convertunicode;
        JSON            parsedJSON, element, array;
        unsigned char   *stringout, stringin [128 +1];
        const char      *JSONout;
        int             numbyteswritten;

        // Check if we got an argument

        switch (argc) {
        case 1:
                // We got no arguments, so assume we need to convert Unicode

                convertunicode = true;
                break;

        case 2:
                // We got an argument, check if it is true or false

                if (! strcasecmp (argv [1], "true")) {
                        // We need to convert Unicode characters

                        convertunicode = true;
                }
                else if (! strcasecmp (argv [1], "false")) {
                        // We are not to convert Unicode characters

                        convertunicode = false;
                }
                else {
                        // We got an error...

                        fprintf (stderr, "Usage: %s [true | false]\n", argv [0]);
                        return 1;
                }
                break;

        default:
                // We got an error...

                fprintf (stderr, "Usage: %s [true | false]\n", argv [0]);
                return 1;
                break;
        }

        // Set the JSON debug to false

        set_parse_JSON_debug (false);

        // Call the function to parse the buffer containing JSON

        parsedJSON = parse_JSON_from_buffer (JSONin, strlen (JSONin), true);

        // Check that we had no errors

        if (get_last_JSON_error () != JSON_OK) {
                // We encountered an error. All we can do is display an error
                // and return false

                fprintf (stderr, "An error (%d) occurred parsing the JSON\n", get_last_JSON_error ());
                return 1;
        }

        // Get the first element from the array

        if ((element = get_first_element_in_JSON_array (parsedJSON)) == (JSON) 0) {
                // We could not get the first element from the JSON array

                fprintf (stderr, "Error: unable to get first element from JSON array (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                free_parsed_JSON (parsedJSON);
                return 1;
        }

        // Go into a loop, printing out strings in the array

        do {
                if ((stringout = get_JSON_string (element)) == (unsigned char *) 0) {
                        // An error occurred, and we could not get the string

                        fprintf (stderr, "Unable to get string from element (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (parsedJSON);
                        return 1;
                }

                // Print the wchar_t string

                printf ("%s\n", stringout);
        }
        while ((element = get_next_element_in_JSON_array (parsedJSON)) != (JSON) 0);

        // We are done, free up parsed JSON

        free_parsed_JSON (parsedJSON);

        // Create a JSON array

        if ((array = create_JSON_array ()) == (JSON) 0) {
                // An error occurred, and we could not create a JSON array

                fprintf (stderr, "Unable to create JSON array (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                return 1;
        }

        for (i = 0; i < 3; i ++) {
                // Build a string we want to convert into JSON, and add it to the array

                strcpy ((char *) stringin, descandchar [i].description);
                strcat ((char *) stringin, descandchar [i].mbcs);

                printf ("Adding to the array - %s\n", stringin);

                if ((element = create_JSON_string (stringin, convertunicode)) == (JSON) 0) {
                        // An error occurred, and we could not create the string

                        fprintf (stderr, "Unable to create JSON string to add to array (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (array);
                        return 1;
                }

                if (add_element_to_JSON_array (array, element) == (JSON) 0) {
                        // An error occurred, and we could not add the string to the
                        // array

                        fprintf (stderr, "Unable to add string element to array (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (array);
                        free_parsed_JSON (element);
                        return 1;
                }
        }

        // Now, get the JSON back out from the JSON we created

        JSONout = write_JSON_to_buffer (array, &numbyteswritten);
        if (JSONout == (char *) 0) {
                // An error occurred

                fprintf (stderr, "An error (%d) occurred writing JSON to a memory buffer\n", get_last_JSON_error ());
                return 1;
        }

        // Display what we got back, and add a trailing newline

        printf ("Buffer size: %d\nJSON:\n%s\n", numbyteswritten, JSONout);

        // Free up the JSON buffer

        free ((void *) JSONout);

        // Just return
        
        return 0;
}
