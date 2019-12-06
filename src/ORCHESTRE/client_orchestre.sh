#!/bin/bash

name=$(basename ${0%.*})
gcc -Wall -Wextra -pedantic -std=c99 -o $name myassert.c $name.c 
./$name l
#rm $name.o
#rm $name

