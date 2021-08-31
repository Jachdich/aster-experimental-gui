#!/bin/sh
if [ $USER = "root" ]
then
    echo "Run this script as a regular user"
    exit
fi
cp -r aster ${HOME}/.config/
sudo cp gui /usr/local/bin/aster
