@echo off

gcc -c -O2 src\*.c -DBUILDING_JDAD_DLL -std=c99
gcc -shared -o jdad.dll *.o
del *.o