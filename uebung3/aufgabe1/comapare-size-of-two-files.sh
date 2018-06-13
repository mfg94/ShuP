#/bin/sh

param1=$1
param2=$2

echo "$#"

if [ -f "${param1}" ] && [ -f "${param2}" ]; then
	if ( ls "$param1" | grep '\.txt$' ) && ( ls "$param2" | grep '\.txt$' ) ; then
	
		#Vergleiche Zeichen
		c1=`wc -m ${param1} | cut -d' ' -f1`;
		c2=`wc -m ${param2} | cut -d' ' -f1`;
		if [ $c1 -gt $c2 ] ; then
			echo "${param1}(${c1}) has more chars than ${param2}(${c2})";
		elif [ $c2 -gt $c1 ] ; then
			echo "${param2}(${c2}) has more chars than ${param1}(${c1})";
		else
			echo "Number of chars in ${param1} and ${param2} are equal with ${c2}";
		fi
		

		#Vergleiche Wörter
		c1=`wc -w ${param1} | cut -d' ' -f1`;
		c2=`wc -w ${param2} | cut -d' ' -f1`;
		if [ $c1 -gt $c2 ]; then
			echo "${param1}(${c1}) has more words than ${param2}($c2})";
		elif [ $c2 -gt $c1 ]; then
			echo "${param2}(${c2}) has more words than ${param1}($c1})";
		else
			echo "Number of words in ${param1} and ${param2} are equal with ${c1}";
		fi	
	
		#Vergleiche Zeilen 
		c1=`wc -l ${param1} | cut -d' ' -f1`;
		c2=`wc -l ${param2} | cut -d' ' -f1`;
		if [ $c1 -gt $c2 ]; then
			echo "${param1}(${c1}) has more lines than ${param2}($c2})";
		elif [ $c2 -gt $c1 ]; then
			echo "${param2}(${c2}) has more lines than ${param1}($c1})";
		else
			echo "Number of lines in ${param1} and ${param2} are equal with ${c1}";	
		fi	
	else
		echo "Please provide two .txt files";
	fi
else
	echo "${param1} or ${param2} is no file.";
fi



