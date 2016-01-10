#!/bin/sh
# test.sh
# CS111 Winter 2016
# Lab1 Cristen Anderson, Sunnie So

# exit with 0 upon success.
# exit with 1 when at least one test fails.




# set -x
# tmp files
								# exit if fails to create files
a=$(mktemp /tmp/a.XXXXXXXXXX) || exit 1
b=$(mktemp /tmp/b.XXXXXXXXXX) || exit 1
c=$(mktemp /tmp/c.XXXXXXXXXX) || exit 1

# for lab 1a
# be able to run simple --rdonly, --wronly, --command
echo "hello from file a" > $a
cmd="./main.o --rdonly $a --wronly $b --wronly $c --command 0 1 2 cat - "
eval $cmd
diff -u $a $b 
if [ $? -ne 0 ]
then
	echo "Test Fails: $cmd" >&2
else 
	echo "Test Success: $cmd" >&2
fi


# delete temp files
rm "$a"
rm "$b"
rm "$c"
