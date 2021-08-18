#!/bin/bash
clear

if [[ $1 != "" ]]; then
	echo "compiling for debugging"
	sudo gcc relmos.c -Wmaybe-uninitialized -Wuninitialized -Wall -pedantic -Werror -g3 -fsanitize=address -lasan -o relmos
else
	echo "compiling for testing"
	sudo gcc relmos.c -Wmaybe-uninitialized -Wuninitialized -Wall -pedantic -Werror -g3 -o relmos
fi