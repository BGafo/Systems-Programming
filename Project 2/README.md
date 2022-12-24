Implementing the ls and ls -l commands for Linux OS

ls with no command-line options
The ls command prints all files/directories in the current working directory.
Running it in your hw2 directory, you might see this output:
find.c
ls.c
Makefile
tree.c
The files/directories should be sorted in case-insensitive lexicographic order:
.<0 <1 <···<9 <a <b <···<z
ls with the -lcommand-line option
If the user invokes ls -l (lowercase “l” for “long”), you should print a “long format” with
extra information about each file.
-rw-rw-r-- bob users 1562 Sep 29 12:00 find.c
-rw-rw-r-- bob users 1024 Sep 29 12:05 ls.c
-rw-rw-r-- bob users 176 Sep 28 11:27 Makefile
-rw-rw-r-- bob users 2044 Sep 27 18:23 tree.c
The first column contains a 10-character permissions string.
•the first character is ‘-’ for files, ‘d’ for directories
•the next three are read, write, and execute permissions for the user
•the next three are read, write, and execute permissions for the group
•the next three are read, write, and execute permissions for others
Permissions should be denoted by a ‘r’, ‘w’, or ‘x’ if present, or ‘-’ otherwise.
The second and third columns show the file owner’s user name and group name (or user
ID / group ID if a name can’t be found).
The next column shows the file size in bytes.
Next is the file’s modification time (mtime) formatted as in the example above.
Finally the filename is listed.
You may find strftime, getpwuid, and getgrgid useful.
