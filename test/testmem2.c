/* File: testmem2.c
**
** Author: John Howie (c) Howie Consulting Inc., 2026. (john@howieconsultinginc.com)
**
** Description
**
**      This test program reads JSON from a buffer in memory, and displays what
** it finds and build the in-memory representation. The difference between this
** and testmem is that no copy of the JSON is made in memory before parsing it
**
** Modifications
**
** 2026-07-20   John Howie      Original.
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
        char            JSONin [] = { '{', '"', 's', 't', 'r', 'i', 'n', 'g', '"', ':', '"', 's', 't', 'r', 'i', 'n', 'g', ' ', 'v', 'a', 'l', 'u', 'e', '"', ',', '"', 'n', 'u', 'm', 'b', 'e', 'r', ' ', '-', ' ', 'i', 'n', 't', 'e', 'g', 'e', 'r', '"', ':',  '1', '2', '3', '4', '5', ',', '"', 'n', 'u', 'm', 'b', 'e', 'r', ' ', '-', ' ', 'f', 'l', 'o', 'a', 't', '"', ':', '1', '2', '3', '.', '4', '5', ',', '"', 'n', 'u', 'm', 'b', 'e', 'r', ' ', '-', ' ', 'd', 'o', 'u', 'b', 'l', 'e', '"', ':', '1', '2', '.', '3', '4', 'e', '5', ',', '"', 'n', 'u', 'm', 'b', 'e', 'r', ' ', '-', ' ', 'i', 'n', 't', 'e', 'g', 'e', 'r', ' ', 'n', 'e', 'g', 'a', 't', 'i', 'v', 'e', '"', ':', '-', '1', '2', '3', '4', '5', ',', '"', 'n', 'u', 'm', 'b', 'e', 'r', ' ', '-', ' ', 'f', 'l', 'o', 'a', 't', ' ', 'n', 'e', 'g', 'a', 't', 'i', 'v', 'e', '"', ':', '-', '1', '2', '3', '.', '4', '5', ',', '"', 'n', 'u', 'm', 'b', 'e', 'r', ' ', '-', ' ', 'd', 'o', 'u', 'b', 'l', 'e', ' ', 'n', 'e', 'g', 'a', 't', 'i', 'v', 'e', '"', ':', '-', '1', '2', '.', '3', '4', 'e', '5', ',', '"', 'b', 'o', 'o', 'l', 'e', 'a', 'n', ' ', 't', 'r', 'u', 'e', '"', ':', 't', 'r', 'u', 'e', ',', '"', 'b', 'o', 'o', 'l', 'e', 'a', 'n', ' ', 'f', 'a', 'l', 's', 'e', '"', ':', 'f', 'a', 'l', 's', 'e', ',', '"', 'n', 'u', 'l', 'l', ' ', 'v', 'a', 'l', 'u', 'e', '"', ':', 'n', 'u', 'l', 'l', ',', '"', 'o', 'b', 'j', 'e', 'c', 't', '"', ':', '{', '"', 's', 'u', 'b', 'o', 'b', 'j', 'e', 'c', 't', '"', ':', '"', 's', 'u', 'b', 'o', 'b', 'j', 'e', 'c', 't', '"', '}', ',', '"', 'a', 'r', 'r', 'a', 'y', '"', ':', '[', '1', ',', '"', 't', 'w', 'o', '"', ',', '3', ',', '"', 'f', 'o', 'u', 'r', '"', ',', '5', ']', '}', 0, 0, 0 };
        JSON            parsedJSON;
        const char      *JSONout;
        int             numbyteswritten;

        // Set the JSON debug to true

        set_parse_JSON_debug (true);

        // Call the function to parse the buffer containing JSON

        parsedJSON = parse_JSON_from_buffer (JSONin, strlen (JSONin), false);

        // Check that we had no errors

        if (get_last_JSON_error () != JSON_OK) {
                // We encountered an error. All we can do is display an error
                // and return false

                fprintf (stderr, "An error (%d) occurred parsing the JSON\n", get_last_JSON_error ());
                return 1;
        }

        // Now, get the JSON back out from the parsed JSON

        JSONout = write_JSON_to_buffer (parsedJSON, &numbyteswritten);
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
