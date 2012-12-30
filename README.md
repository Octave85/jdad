# Lango

Lango is a compilation of tools for programming language functions.

## Environment
### symtab
symtab.c contains functions that run a symbol table with chaining.
It uses Paul Hsieh's SuperFastHash hash function over 1024 buckets by default.
Within each chain, each individual name entry consists of a list
of 'layers'. These can be accessed as a list or a stack, allowing for either multiple types of the same name, or a stack of instances of a single symbol in different scopes.