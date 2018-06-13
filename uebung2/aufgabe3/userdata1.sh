#/bin/sh

username=$1

result=echo "Benutzername: `less /etc/passwd | grep $username | cut -f1 -d:`"

if ["$result"=""]
then
	echo "Username not found"
fi

echo "Homeverzeichnis: `less /etc/passwd | grep $username | cut -f6 -d:`"
echo "Bash: `less /etc/passwd | grep $username | cut -f7 -d:`"
