#/bin/sh

echo "Skriptbeginn: $0"


d1=$@
echo "Parameter 1: $d1"



exec < "$d1"
read i
echo "Zeile1: $i"
read i
echo "Zeile2: $i"
read i
echo "Zeile3: $i"


exec < /dev/tty
echo -n "Bitte eine Eingabe:"
read

echo "Skriptende: $0"



