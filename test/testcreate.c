/* File: testcreate.c
**
** Author: John Howie (c) Howie Consulting Inc., 2026. (john@howieconsultinginc.com)
**
** Description
**
**      This test program creates an array of JSON objects, representing a
** list of customers.
**
** Modifications
**
** 2026-09-07 John Howie        Original.
**
*/

# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <string.h>
# include "parseJSON.h"

int main (void);

int main (void)
{
        JSON            customerarray, customerobject, phoneobject, number, string;
        int             i;
        const char      *JSONout;
        int             numbyteswritten;

        struct {
                int     number;
                char    *name;
                char    *addressline1;
                char    *addressline2;
                char    *addresstown;
                char    *addressstateorprovince;
                char    *addressziporpostal;
                char    *level;
                struct {
                        char    *home;
                        char    *mobile;
                } phones;
                float   rewardsbalance;
        } customer [] = {
                {
                        1,
                        "John Howie",
                        "32 W 1st St.",
                        "",
                        "Smallville",
                        "KS",
                        "67733",
                        "Gold",
                        {
                                "316-555-1234",
                                "316-555-7890"
                        },
                        123.45
                },
                {
                        2,
                        "Amy Smith",
                        "5023 Big Street",
                        "Apt. 214",
                        "Metropolis",
                        "IL",
                        "60601",
                        "Platinum",
                        {
                                "312-555-4567",
                                "312-555-8989"
                        },
                        1099.21
                },
                {
                        3,
                        "Poppy DeeJay",
                        "54 Main St.",
                        "",
                        "Seattle",
                        "WA",
                        "98101",
                        "Silver",
                        {
                                "206-555-1919",
                                ""
                        },
                        1040.32
                },
                {
                        4,
                        "Lorenzo Nino",
                        "128 Spokane Avenue",
                        "Unit 32",
                        "Wenatchee",
                        "WA",
                        "98801",
                        "Bronze",
                        {
                                "",
                                "509-555-6482"
                        },
                        546.92
                }
        };

        // Create an array we will use to store the objects in

        if ((customerarray = create_JSON_array ()) == (JSON) 0) {
                // An error occurred, and we were unable to create the array we
                // will store the object in

                fprintf (stderr, "Error: unable to create array for objects (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                return 1;
        }

        // Go through the list of customers, building customer objects and
        // add them to the array

        for (i = 0; i < 4; i ++) {
                // Create the first customer object we will addd to the array

                if ((customerobject = create_JSON_object ()) == (JSON) 0) {
                        // An error occurred, and we could not create the customer
                        // object

                        fprintf (stderr, "Error: unable to create customer object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Add members to the customer object, starting with the
                // customer number

                if ((number = create_JSON_number_long (customer [i].number)) == (JSON) 0) {
                        // An error occurred, and we could not create a number
                        // to add as a member to the customer object

                        fprintf (stderr, "Error: unable to create number value (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                if (add_member_to_JSON_object (customerobject, (unsigned char *) "cust_id", true, number) == (JSON) 0) {
                        // An error occurred, and we could not add the customer
                        // number to the object

                        fprintf (stderr, "Error: unable to add customer number to customer object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Now, create and add the customer name

                if ((string = create_JSON_string ((unsigned char *) customer [i].name, true)) == (JSON) 0) {
                        // An error occurred, and we could create the customer
                        // name

                        fprintf (stderr, "Error: unable to create string value (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);                        
                        return 1;
                }

                if (add_member_to_JSON_object (customerobject, (unsigned char *) "cust_name", true, string) == (JSON) 0) {
                        // An error occurred, and we could not add the customer
                        // name to the object

                        fprintf (stderr, "Error: unable to add customer name to customer object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Now, create and add address line 1

                if ((string = create_JSON_string ((unsigned char *) customer [i].addressline1, true)) == (JSON) 0) {
                        // An error occurred, and we could create the customer
                        // address line 1

                        fprintf (stderr, "Error: unable to create string value (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);                        
                        return 1;
                }

                if (add_member_to_JSON_object (customerobject, (unsigned char *) "cust_addr_1", true, string) == (JSON) 0) {
                        // An error occurred, and we could not add the customer
                        // address line 1 to the object

                        fprintf (stderr, "Error: unable to add customer address1 to customer object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Now, create and add address line 2

                if ((string = create_JSON_string ((unsigned char *) customer [i].addressline2, true)) == (JSON) 0) {
                        // An error occurred, and we could create the customer
                        // address line 2

                        fprintf (stderr, "Error: unable to create string value (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);                        
                        return 1;
                }

                if (add_member_to_JSON_object (customerobject, (unsigned char *) "cust_addr_2", true, string) == (JSON) 0) {
                        // An error occurred, and we could not add the customer
                        // address line 2 to the object

                        fprintf (stderr, "Error: unable to add customer address2 to customer object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Now, create and add address town

                if ((string = create_JSON_string ((unsigned char *) customer [i].addresstown, true)) == (JSON) 0) {
                        // An error occurred, and we could create the customer
                        // town

                        fprintf (stderr, "Error: unable to create string value (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);                        
                        return 1;
                }

                if (add_member_to_JSON_object (customerobject, (unsigned char *) "cust_addr_town", true, string) == (JSON) 0) {
                        // An error occurred, and we could not add the customer
                        // town to the object

                        fprintf (stderr, "Error: unable to add customer town to customer object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Now, create and add address state or province

                if ((string = create_JSON_string ((unsigned char *) customer [i].addressstateorprovince, true)) == (JSON) 0) {
                        // An error occurred, and we could create the customer
                        // state or province

                        fprintf (stderr, "Error: unable to create string value (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);                        
                        return 1;
                }

                if (add_member_to_JSON_object (customerobject, (unsigned char *) "cust_addr_st_or_pr", true, string) == (JSON) 0) {
                        // An error occurred, and we could not add the customer
                        // town to the object

                        fprintf (stderr, "Error: unable to add customer state/province to customer object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Now, create and add address ZIP or postal code

                if ((string = create_JSON_string ((unsigned char *) customer [i].addressziporpostal, true)) == (JSON) 0) {
                        // An error occurred, and we could create the customer
                        // ZIP or postal code

                        fprintf (stderr, "Error: unable to create string value (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);                        
                        return 1;
                }

                if (add_member_to_JSON_object (customerobject, (unsigned char *) "cust_addr_ZIPPostal", true, string) == (JSON) 0) {
                        // An error occurred, and we could not add the customer
                        // ZIP or postal code to the object

                        fprintf (stderr, "Error: unable to add customer ZIP/Postal to customer object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Next, create and add customer level

                if ((string = create_JSON_string ((unsigned char *) customer [i].level, true)) == (JSON) 0) {
                        // An error occurred, and we could create the customer
                        // level

                        fprintf (stderr, "Error: unable to create string value (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);                        
                        return 1;
                }

                if (add_member_to_JSON_object (customerobject, (unsigned char *) "cust_level", true, string) == (JSON) 0) {
                        // An error occurred, and we could not add the customer
                        // level to the object

                        fprintf (stderr, "Error: unable to add customer level to customer object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Create a new object for the phone numbers

                if ((phoneobject = create_JSON_object ()) == (JSON) 0) {
                        // We were unable to create the new object to hold the
                        // customer phone numbers

                        fprintf (stderr, "Error: unable to create phone object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Get the home phone number - we only record it if it is not
                // empty

                if (strlen (customer [i].phones.home) > 0) {
                        // The phone number is not empty, so we need to create
                        // a string value for it

                        if ((string = create_JSON_string ((unsigned char *) customer [i].phones.home, true)) == (JSON) 0) {
                                // An error occurred, and we could create the
                                // customer home phone number

                                fprintf (stderr, "Error: unable to create string value (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                                free_parsed_JSON (phoneobject);
                                free_parsed_JSON (customerobject);
                                free_parsed_JSON (customerarray);                        
                                return 1;
                        }

                        if (add_member_to_JSON_object (phoneobject, (unsigned char *) "home", true, string) == (JSON) 0) {
                                // An error occurred, and we could not add the
                                // customer home phone number to the object

                                fprintf (stderr, "Error: unable to add customer home phone to phone object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                                free_parsed_JSON (phoneobject);
                                free_parsed_JSON (customerobject);
                                free_parsed_JSON (customerarray);
                                return 1;
                        }
                }

                // Get the mobile phone number - we only record it if it is not
                // empty

                if (strlen (customer [i].phones.mobile) > 0) {
                        // The phone number is not empty, so we need to create
                        // a string value for it

                        if ((string = create_JSON_string ((unsigned char *) customer [i].phones.mobile, true)) == (JSON) 0) {
                                // An error occurred, and we could create the
                                // customer mobile phone numner

                                fprintf (stderr, "Error: unable to create string value (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                                free_parsed_JSON (phoneobject);
                                free_parsed_JSON (customerobject);
                                free_parsed_JSON (customerarray);                        
                                return 1;
                        }

                        if (add_member_to_JSON_object (phoneobject, (unsigned char *) "mobile", true, string) == (JSON) 0) {
                                // An error occurred, and we could not add the
                                // customer mobile phone number to the object

                                fprintf (stderr, "Error: unable to add customer mobile phone to phone object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                                free_parsed_JSON (phoneobject);
                                free_parsed_JSON (customerobject);
                                free_parsed_JSON (customerarray);
                                return 1;
                        }
                }

                // Add the phone object to the customer object

                if (add_member_to_JSON_object (customerobject, (unsigned char *) "cust_phones", true, phoneobject) == (JSON) 0) {
                        // An error occurred, and we could not add the phone
                        // object to the customer object

                        fprintf (stderr, "Error: unable to add phone object to customer object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (phoneobject);
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Lastly, we need to create a number value for the rewards
                // balance and add it to the customer object

                if ((number = create_JSON_number_double ((double) customer [i].rewardsbalance)) == (JSON) 0) {
                        // An error occurred, and we could not create a number
                        // value

                        fprintf (stderr, "Error: unable to create number value (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                if (add_member_to_JSON_object (customerobject, (unsigned char *) "cust_rewards", true, number) == (JSON) 0) {
                        // An error occurred, and we could not add the customer
                        // rewards balance to the object

                        fprintf (stderr, "Error: unable to add customer rewards balance to customer object (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }

                // Lastly, add the customer object to the array

                if (add_element_to_JSON_array (customerarray, customerobject) == (JSON) 0) {
                        // An error occurred, and we could not add the customer
                        // object to the array

                        fprintf (stderr, "Error: unable to add customer object to array (%d - %s)\n", get_last_JSON_error (), get_last_JSON_error_string ());
                        free_parsed_JSON (customerobject);
                        free_parsed_JSON (customerarray);
                        return 1;
                }
        }

        // Now, get the JSON back out from the parsed JSON we created

        JSONout = write_JSON_to_buffer (customerarray, &numbyteswritten);
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