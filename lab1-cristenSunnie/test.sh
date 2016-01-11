#!/bin/sh
# test.sh
# CS111 Winter 2016
# Lab1 Cristen Anderson, Sunnie So

# exit with 0 upon success.
# exit with 1 when at least one test fails.

success()
{
	if [ $? -ne 0 ]; then
		echo "FAIL: $1"
		exit 1
	fi
}

failure()
{
	if [ $? -eq 0 ]; then
		echo "FAIL: $1"; 
		exit 1
	fi
}

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
./simpsh --rdonly noFile 2>&1 | grep "Error: open returned unsuccessfully" > /dev/null
success "--rdonly: report invalid filename."
./simpsh --rdonly $a 2>&1 | grep "Error: open returned unsuccessfully" > /dev/null
failure "--rdonly: open valid file."

# --wronly
./simpsh --wronly noFile 2>&1 | grep "Error: open returned unsuccessfully" > /dev/null
success "--wronly: report invalid filename"
./simpsh --wronly $a 2>&1 | grep "Error: open returned unsuccessfully" > /dev/null
failure "--wronly: open valid file"

# --command

./simpsh --rdonly $a --wronly $b --wronly $c --command 0 1 2 cat - > /dev/null
diff -u $a $b 
success "--command: execute simple command 'cat' "

./simpsh --rdonly $a --wronly $b --wronly $c --command 0 1 cat - 2>&1 | grep "Error: Incorrect usage of --command. Requires integer argument." > /dev/null
success "--command: report none digit file descriptor"

./simpsh --rdonly $a --wronly $b --wronly $c --command 0 1 2 3 cat - 2>&1 > /dev/null
cat $c |grep "Error: Unknown command"  > /dev/null
success "--command: report invalid number of arguments"

./simpsh --command 0 1 2 echo "hi" 2>&1 | grep "Error: Invalid use of file descriptor" > /dev/null
success "--command: report invalid use of file descriptor"

# --verbose

./simpsh --verbose --rdonly $a --wronly $b --wronly $c --command 0 1 2 cat - > $d
echo '--rdonly /tmp/a ' > $e; echo '--wronly /tmp/b ' >> $e; echo '--wronly /tmp/c ' >> $e; echo '--command 0 1 2 cat - ' >> $e
diff -u $d $e > /dev/null
success "--verbose: valid output when verbose is in the beginning"

./simpsh --rdonly $a --wronly $b --verbose --wronly $c --command 0 1 2 cat - > $d
echo '--wronly /tmp/c ' > $e; echo '--command 0 1 2 cat - ' >> $e
diff -u $d $e > /dev/null
success "--verbose: valid output when verbose is in the middle of arguments"




# delete temp files
rm "$a"
rm "$b"
rm "$c"

exit 0

