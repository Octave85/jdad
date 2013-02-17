gcc -fPIC -c -O2 src/*.c -DBUILDING_JDAD_DLL
gcc -shared -o libjdad.so.1 *.o
rm *.o
