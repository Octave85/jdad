@echo off

g++ -c src\*.c -DBUILDING_JDAD_DLL
g++ -shared -o jdad.dll *.o
del *.o