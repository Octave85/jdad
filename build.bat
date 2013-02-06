@echo off

gcc -c src\*.c -DBUILDING_JDAD_DLL -std=c99
gcc -shared -o jdad.dll *.o
del *.o