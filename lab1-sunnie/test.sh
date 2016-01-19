#!/bin/sh
# test.sh
# CS111 Winter 2016
# Lab1 Cristen Anderson, Sunnie So

# exit with 0 upon success.
# exit with 1 when at least one test fails.

# tmp files
# exit if fails to create files
a=/tmp/a || exit 1
b=/tmp/b || exit 1
c=/tmp/c || exit 1
d=/tmp/d || exit 1
e=/tmp/e || exit 1
> "$a"
> "$b"
> "$c"
> "$d"
# a=$(mktemp /tmp/a.XXXXXXXXXX) || exit 1
# b=$(mktemp /tmp/b.XXXXXXXXXX) || exit 1
# c=$(mktemp /tmp/c.XXXXXXXXXX) || exit 1
# d=$(mktemp /tmp/d.XXXXXXXXXX) || exit 1


# for lab 1a
echo "hello from file a" > $a

# --rdonly
./simpsh --rdonly noFile 2>&1 | grep "Error: open returned unsuccessfully" > /dev/null ||{ echo "FAIL: --rdonly: report invalid filename."; exit 1; }

# --wronly
./simpsh --wronly noFile 2>&1 | grep "Error: open returned unsuccessfully" > /dev/null || { echo "FAIL: --wronly: report invalid filename"; exit 1; }

# --command

./simpsh --rdonly $a --wronly $b --wronly $c --command 0 1 2 cat -
> "$c"
> "$d"
cat $a > $c
cat $b > $d
diff -u $c $d > /dev/null || { echo "FAIL: --command: execute simple command 'cat' "; exit 1;}


> "$c"
./simpsh --rdonly $a --rdonly $b --wronly $c --command 0 1 2 cat -

./simpsh --command 0 1 2 echo "hi" 2>&1 | grep "initiation" > /dev/null || { echo "FAIL: --command: report uninitialized file descriptor"; exit 1;}

# To prevent race condition, let the next command run first...
cat $c | grep "Bad file descriptor" > /dev/null || { echo "FAIL: --rdonly: Error on writing to read_only file"; exit 1; }

./simpsh --rdonly $a --wronly $b --wronly $c --command 0 1 cat - 2>&1 | grep "Error: Incorrect usage of --command. Requires integer argument." > /dev/null || { echo "FAIL: --command: report none digit file descriptor"; exit 1;}

./simpsh --rdonly $a --wronly $b --wronly $c --command 0 1 2 3 cat - 2>&1 > /dev/null
cat $c |grep "Error: Unknown command"  > /dev/null || { echo "FAIL: --command: report invalid number of arguments"; exit 1; }

./simpsh --command 0 1 2 echo "hi" 2>&1 | grep "Error: Invalid use of file descriptor" > /dev/null || { echo "FAIL: --command: report invalid use of file descriptor"; exit 1;}


# --verbose

./simpsh --verbose --rdonly $a --wronly $b --wronly $c --command 0 1 2 cat - > $d
echo '--rdonly /tmp/a ' > $e; echo '--wronly /tmp/b ' >> $e; echo '--wronly /tmp/c ' >> $e; echo '--command 0 1 2 cat - ' >> $e
diff -u $d $e > /dev/null || { echo "FAIL: --verbose: valid output when verbose is in the beginning"; exit 1;}

./simpsh --rdonly $a --wronly $b --verbose --wronly $c --command 0 1 2 cat - > $d
echo '--wronly /tmp/c ' > $e; echo '--command 0 1 2 cat - ' >> $e
diff -u $d $e > /dev/null || { echo "FAIL: --verbose: valid output when verbose is in the middle of arguments"; exit 1;}


# delete temp files
rm "$a"
rm "$b"
rm "$c"

exit 0

