grep.c
Write a program that takes a string as a command-line argument and then reads
from stdin until the user closes stdin (with ctrl-d, written ^D below). It should print
every line that contains the given string. It should be case-sensitive by default, but
case-insensitive if the command line option -i is given before the string. Strings
may have arbitrary length.
./grep ello
Hello there # input
Hello there # output
Hi there # input, ignored since it doesn't contain "ello"
The sun is yellow # input
The sun is yellow # output
YELLOW # input, ignored since "ello" is case-sensitive
^D # close stdin
./grep -i ello
Hello there # input
Hello there # output
YELLOW # input
YELLOW # output
^D # close stdin
