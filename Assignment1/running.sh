gcc motor_x.c -o motor_x
gcc motor_z.c -o motor_z
gcc command_console.c -o command_console
gcc inspection_console.c -o inspection_console
gcc watchdog.c -o watchdog
gcc master.c -o master

./master
