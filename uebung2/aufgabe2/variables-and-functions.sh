#/bin/bash

doublestring_without_interface() {
	echo "$param$param"
}


doublestring() {
	echo "$1$1"
}


echo "Input: $1"
param=$1
echo "doublestring: `doublestring $1`"
echo "doublestring_without_interface: `doublestring_without_interface`"

