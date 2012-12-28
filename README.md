# Lango

Lango is a compilation of tools for programming language functions.

## Environmen
### symtab
symtab.c contains functions that run a symbol table with chaining.
It uses Paul Hsieh's SuperFasterHash hash function. 
Additionally, each link of the chains corresponds to a stack of symbols with that name. This allows for possible lexical scoping.
symtab also implements a global heap memory structure. 
Each symbol points to a value in the heap, which can be a string, integer, long, or another type of data (with a void pointer).