#!/bin/bash
more project_des.txt

cd /$1/watchdog
more -p watchdog.txt

cd /$1/motor_z
more -p motor_z.txt

cd /$1/motor_x
more -p motor_x.txt

cd /$1/master
more -p master.txt

cd /$1/inspection_console
more -p inspection.txt

cd /$1/command_console
more -p command_console.txt

