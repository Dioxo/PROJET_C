#!/bin/bash

name=$(basename ${0%.*})
gcc -Wall -Wextra -pedantic -std=c99 -g -o $name -I ../ORCHESTRE ../ORCHESTRE/client_orchestre.c ../ORCHESTRE/myassert.c $name.c 
gcc -Wall -Wextra -pedantic -std=c99 -g -o client_test -I ../ORCHESTRE ../ORCHESTRE/client_orchestre.c ../ORCHESTRE/myassert.c client_test.c  
#rm $name.o
#rm $name
rm pipeClientToOrchestra
rm pipeOrchestraToClient
