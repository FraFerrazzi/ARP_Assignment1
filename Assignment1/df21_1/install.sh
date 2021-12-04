#!/bin/bash

unzip sources.zip
mv sources $1

mkdir /$1/executable

cd /$1/master
gcc master.c -o master
#mv master $1/executable

cd /$1/command_console
gcc command_console.c -o command_console
#mv command_console $1/executable

cd /$1/motor_x
gcc motor_x.c -o motor_x
#mv motor_x $1/executable

cd /$1/motor_z
gcc motor_z.c -o motor_z
#mv motor_z $1/executable

cd /$1/inspection_console
gcc inspection_console.c -o inspection_console
#mv inspection_console $1/executable

cd /$1/watchdog
gcc watchdog.c -o watchdog
#mv watchdog $1/executable
