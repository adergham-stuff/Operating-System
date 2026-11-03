Project C - Loading Files and Executing Programs
Andrew Dergham, Mason Andrade, Chris Felumero
1) what we did
- kernel.c
Andrew
  ReadFile() searches the disk to find a file.
   If the file exists then the computer will load the data so the computer can use it
  executeProgram() finds a program then loads it to the computers memory and then runs it
Chris
  terminate() Returns to the shell after a program is done running
  handleInterupt21() Handles requests from the shell and other programs
  These requests include printing text and reading inputs and finding files and running programs
  main() starts the operating system by setting up the system calls then launching the shell
    
Mason
  type(filename) finds a file then displayhs what is inside it
  exec(filename) find a program then runs it
  Invalid commands get displayed as Bad command! if the user enters a incorrect command

2) HOW TO VERIFY
    ./compileOS.sh
    java -jar simulator.jar        (disk image: diskc.img)
