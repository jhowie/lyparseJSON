/* File: testfile.c
**
** Author: John Howie (c) Howie Consulting Inc., 2026. (john@howieconsultinginc.com)
**
** Description
**
**      This test program reads JSON from a file, and displays what it finds
** and build the in-memory representation.
**
** Modifications
**
** 2026-07-06   John Howie      Original.
**
*/

# include <stdio.h>
# include <stdbool.h>
# include "parseJSON.h"

int main (int argc, char *argv []);

int main (int argc, char *argv [])
{
        JSON    parsedJSON;

        // Check that we got an argument - the name of the file to process

        if (argc != 2) {
                // We did not get the name of a file to process (or we got
                // extraneous arguments)

                fprintf (stderr, "Usage: %s <JSON file>\n", argv [0]);
                return 1;
        }

        // Set the JSON debug to true

        set_parse_JSON_debug (true);

        // Call the function to parse the JSON file

        parsedJSON = parse_JSON_from_file (argv [1]);

        // Check that we had no errors

        if (get_last_JSON_error () != JSON_OK) {
                // We encountered an error. All we can do is display an error
                // and return false

                fprintf (stderr, "An error (%d) occurred parsing the JSON\n", get_last_JSON_error ());
                return 1;
        }

        // Now, dump the JSON we parsed to the standard output

        if (! write_JSON_to_file (parsedJSON, (char *) 0)) {
                // We had an error writing out the parsed JSON. Display the
                // error and return
 
                fprintf (stderr, "An error (%d) occurred writing the JSON\n", get_last_JSON_error ());
                return 1;
        }

        return 0;
}
