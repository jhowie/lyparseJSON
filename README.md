**lyparseJSON** - A lightweight JSON parser built using lex (flex) and yacc (bison).

This parser, when built, creates a library that can be used by programs to parse
JSON in files and from a buffer in memory. There are test programs that show you
how to use the library, and a header file which contains the public interface to
the library.

Parsed JSON is represented in memory as a series of structures. The parsed JSON
can be queried and values (arrays, objects, booleans, numbers, nulls and strings)
retrieved.

It is possible to create JSON using the API, too. Examples of how to do this can
be found in the test programs.

**LIMITATIONS**

The following limitations should be kept in mind:

  1) The library uses recursion in a few places, but is reasonably efficient; and
  2) The library is NOT thread safe (only one thread can be parsing JSON at a time).

These limitations will be addressed (eventually).

**LICENSE**

The code is published under the BSD-3 Clause License except for the code generated
by lex (flex) and yacc (bison). These are published under the GNU license. 
