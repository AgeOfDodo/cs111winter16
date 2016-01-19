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
> "$e"
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
sleep 1
cat $c | grep "Bad file descriptor" > /dev/null || { echo "FAIL: --rdonly: Error on writing to read_only file"; exit 1; }

./simpsh --command 0 1 2 echo "hi" 2>&1 | grep "initiation" > /dev/null || { echo "FAIL: --command: report uninitialized file descriptor"; exit 1;}

./simpsh --rdonly $a --wronly $b --wronly $c --command 0 1 cat - 2>&1 | grep "Error: Incorrect usage of --command. Requires integer argument." > /dev/null || { echo "FAIL: --command: report none digit file descriptor"; exit 1;}

./simpsh --rdonly $a --wronly $b --wronly $c --command 0 1 2 3 cat - 2>&1 > /dev/null
cat $c | grep "Error: Unknown command"  > /dev/null || { echo "FAIL: --command: report invalid number of arguments"; exit 1; }

./simpsh --command 0 1 2 echo "hi" 2>&1 | grep "Error: Invalid use of file descriptor" > /dev/null || { echo "FAIL: --command: report invalid use of file descriptor"; exit 1;}


# --verbose

./simpsh --verbose --rdonly $a --wronly $b --wronly $c --command 0 1 2 cat - > $d
echo '--rdonly /tmp/a ' > $e; echo '--wronly /tmp/b ' >> $e; echo '--wronly /tmp/c ' >> $e; echo '--command 0 1 2 cat - ' >> $e
diff -u $d $e > /dev/null || { echo "FAIL: --verbose: valid output when verbose is in the beginning"; exit 1;}

./simpsh --rdonly $a --wronly $b --verbose --wronly $c --command 0 1 2 cat - > $d
echo '--wronly /tmp/c ' > $e; echo '--command 0 1 2 cat - ' >> $e
diff -u $d $e > /dev/null || { echo "FAIL: --verbose: valid output when verbose is in the middle of arguments"; exit 1;}

####################################################

# test cases for lab1b

# pipe should work as expected
echo 'Ac: line 2' >> $a
echo 'Ab: line 3' >> $a
echo 'B : hi from file b' > b
> "$c"
> "$d"
> "$e"
# cat a | sort | cat b - | tr 'A-Z' 'a-z' > c

./simpsh --verbose  --rdonly $a   --pipe   --pipe   --creat \
--trunc --wronly $c   --creat --append --wronly $d --command \
0 2 6 sort   --command 1 4 6 cat $b - --command 3 5 6 tr 'A-Z' 'a-z' 2>&1 > /dev/null
cat $a | sort | cat $b - | tr 'A-Z' 'a-z' > $e
sleep 1
diff -u $e $c  || { echo "FAIL: error multi pipe operations."; exit 1;}

# --catch and --abort
./simpsh --catch 11 --abort 2>&1 | grep "11 caught" > /dev/null || { echo "FAIL: --catch: cannot catch signal"; exit 1;}

# file flag 
echo "original text" > "$c"
> "$d"
> "$e"
./simpsh --rdonly $b --append --wronly $c --wronly d --command 0 1 2 cat b 2>&1 > /dev/null 
cat $c | grep "original text" > /dev/null || { echo "FAIL: --append does not work"; exit 1;} 
cat $c | grep "hi from file b" > /dev/null || { echo "FAIL: --append does not work"; exit 1;}




exit 0

