#!/usr/bin/env

gcc -c src/*.c -DBUILDING_JDAD_DLL
gcc -shared -o jdad.dll *.o
rm *.o