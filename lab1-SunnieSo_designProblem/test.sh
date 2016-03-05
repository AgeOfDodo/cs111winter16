#!/bin/sh
# test.sh
# CS111 Winter 2016
# Lab1 Cristen Anderson, Sunnie So

# exit with 0 upon success.
# exit with 1 when at least one test fails.

# tmp files
# exit if fails to create files
a=/tmp/cs111_sunnie_a || exit 1
b=/tmp/cs111_sunnie_b || exit 1
c=/tmp/cs111_sunnie_c || exit 1
d=/tmp/cs111_sunnie_d || exit 1
e=/tmp/cs111_sunnie_e || exit 1
> "$a"
> "$b"
> "$c"
> "$d"
> "$e"
# a=$(mktemp /tmp/a.XXXXXXXXXX) || exit 1
# b=$(mktemp /tmp/b.XXXXXXXXXX) || exit 1
# c=$(mktemp /tmp/c.XXXXXXXXXX) || exit 1
# d=$(mktemp /tmp/d.XXXXXXXXXX) || exit 1

# for design problem
echo "hello from file a" > $a

./simpsh --rdonly $a   --pipe --rdonly $b --wronly $c --wronly $d --command 0 2 5 cat - --command 1 4 5 cat - $b --wait 1 | grep "cat - /tmp/cs111_sunnie_b" > /dev/null || { echo "FAIL: design problem"; exit 1; }
./simpsh --rdonly $a   --pipe --rdonly $b --wronly $c --wronly $d --command 0 2 5 cat - --command 1 4 5 cat - $b --wait 0 | grep "cat -" > /dev/null || { echo "FAIL: design problem"; exit 1; }
./simpsh --rdonly $a   --pipe --rdonly $b --wronly $c --wronly $d --command 0 2 5 sort --command 1 4 5 cat - $b --wait all | grep "cat - /tmp/cs111_sunnie_b" > /dev/null || { echo "FAIL: design problem"; exit 1; }
./simpsh --rdonly $a   --pipe --rdonly $b --wronly $c --wronly $d --command 0 2 5 sort --command 1 4 5 cat - $b --wait all | grep "sort" > /dev/null || { echo "FAIL: design problem"; exit 1; }
./simpsh --rdonly $a   --pipe --rdonly $b --wronly $c --wronly $d --command 0 2 5 sort --command 1 4 5 cat - $b --wait 2 2>&1 | grep "Error: Design Problem: invalid command descripter" > /dev/null || { echo "FAIL: design problem error catching"; exit 1; }


# # --rdonly
# ./simpsh --rdonly noFile 2>&1 | grep "Error: open returned unsuccessfully" > /dev/null ||{ echo "FAIL: --rdonly: report invalid filename."; exit 1; }

# # --wronly
# ./simpsh --wronly noFile 2>&1 | grep "Error: open returned unsuccessfully" > /dev/null || { echo "FAIL: --wronly: report invalid filename"; exit 1; }

# # --command

# ./simpsh --rdonly $a --wronly $b --wronly $c --command 0 1 2 cat -
# > "$c"
# > "$d"
# cat $a > $c
# cat $b > $d
# diff -u $c $d > /dev/null || { echo "FAIL: --command: execute simple command 'cat' "; exit 1;}


# > "$c"
# ./simpsh --rdonly $a --rdonly $b --wronly $c --command 0 1 2 cat -
# sleep 1
# cat $c | grep "Bad file descriptor" > /dev/null || { echo "FAIL: --rdonly: Error on writing to read_only file"; exit 1; }

# ./simpsh --command 0 1 2 echo "hi" 2>&1 | grep "initiation" > /dev/null || { echo "FAIL: --command: report uninitialized file descriptor"; exit 1;}

# ./simpsh --rdonly $a --wronly $b --wronly $c --command 0 1 cat - 2>&1 | grep "Error: Incorrect usage of --command. Requires integer argument." > /dev/null || { echo "FAIL: --command: report none digit file descriptor"; exit 1;}

# ./simpsh --rdonly $a --wronly $b --wronly $c --command 0 1 2 3 cat - 2>&1 > /dev/null
# # sleep 1
# cat $c | grep "Error: Unknown command"  > /dev/null || { echo "FAIL: --command: report invalid number of arguments"; exit 1; }

# ./simpsh --command 0 1 2 echo "hi" 2>&1 | grep "Error: Invalid use of file descriptor" > /dev/null || { echo "FAIL: --command: report invalid use of file descriptor"; exit 1;}


# # --verbose

# ./simpsh --verbose --rdonly $a --wronly $b --wronly $c --command 0 1 2 cat - > $d
# echo '--rdonly /tmp/cs111_sunnie_a ' > $e; echo '--wronly /tmp/cs111_sunnie_b ' >> $e; echo '--wronly /tmp/cs111_sunnie_c ' >> $e; echo '--command 0 1 2 cat - ' >> $e
# diff -u $d $e > /dev/null || { echo "FAIL: --verbose: valid output when verbose is in the beginning"; exit 1;}

# ./simpsh --rdonly $a --wronly $b --verbose --wronly $c --command 0 1 2 cat - > $d
# echo '--wronly /tmp/cs111_sunnie_c ' > $e; echo '--command 0 1 2 cat - ' >> $e
# diff -u $d $e > /dev/null || { echo "FAIL: --verbose: valid output when verbose is in the middle of arguments"; exit 1;}

# ####################################################

# # test cases for lab1b

# # --pipe 
# echo 'Ac: line 2' >> $a
# echo 'Ab: line 3' >> $a
# echo 'B : hi from file b' > b
# > "$c"
# > "$d"
# > "$e"
# ./simpsh --verbose  --rdonly $a   --pipe   --pipe   --creat \
# --trunc --wronly $c   --creat --append --wronly $d --command \
# 0 2 6 sort   --command 1 4 6 cat $b - --command 3 5 6 tr 'A-Z' 'a-z' 2>&1 > /dev/null
# cat $a | sort | cat $b - | tr 'A-Z' 'a-z' > $e
# sleep 1
# diff -u $e $c  || { echo "FAIL: error multi pipe operations."; exit 1;}

# # --catch and --abort
# ./simpsh --catch 11 --abort 2>&1 | grep "11 caught" > /dev/null || { echo "FAIL: --catch: cannot catch signal"; exit 1;}

# # file flag 
# echo "original text" > "$c"
# > "$d"
# > "$e"
# ./simpsh --rdonly $b --append --wronly $c --wronly $d --command 0 1 2 cat b 2>&1 > /dev/null 
# cat $c | grep "original text" > /dev/null || { echo "FAIL: --append does not work"; exit 1;} 
# cat $c | grep "hi from file b" > /dev/null || { echo "FAIL: --append does not work"; exit 1;}

# # wait
# > "$c"
# > "$d"
# > "$e"
# ./simpsh --verbose  --rdonly $a   --pipe   --pipe   --creat \
# --trunc --wronly $c   --creat --append --wronly $d --command \
# 0 2 6 sort   --command 1 4 6 cat $b - --command 3 5 6 tr 'A-Z' 'a-z' --wait > $e
# cat $e | grep "sort" > /dev/null || { echo "FAIL: --wait: incomplete output"; exit 1;}
# cat $e | grep "tr A-Z a-z" > /dev/null || { echo "FAIL: --wait: incomplete output"; exit 1;}
# cat $e | grep "cat" > /dev/null || { echo "FAIL: --wait: incomplete output"; exit 1;}

# # close
# ./simpsh --rdonly $a --wronly $b --wronly $c --close 2 --command 0 1 2 cat 2>&1 | grep "Error: Invalid access to file descriptor" > /dev/null ||  { echo "FAIL: --close does not work"; exit 1;}

# ignore and abort
# ./simpsh --rdonly $a --wronly $b --wronly $c --ignore 11 --abort --close 2 --command 0 1 2 cat 2>&1 | grep "Error: Invalid access to file descriptor" > /dev/null ||  { echo "FAIL: --ignore failed to ignore --abort"; exit 1;}


exit 0

