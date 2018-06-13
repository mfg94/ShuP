#/bin/sh

read username

echo "Benutzername: `less /etc/passwd | grep $username | cut -f1 -d:`"
echo "Homeverzeichnis: `less /etc/passwd | grep $username | cut -f6 -d:`"
echo "Bash: `less /etc/passwd | grep $username | cut -f7 -d:`"
