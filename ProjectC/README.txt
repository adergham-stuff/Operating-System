Project C - Loading Files and Executing Programs
Andrew Dergham, Mason Andrade, Chris Felumero

1) WHAT WE DID
- kernel.c
    * readFile(name, buffer, &sectorsRead): reads the directory (sector 2), matches
      the name (up to 6 chars), then loads each sector listed in the entry into the
      buffer (+512 each time). sectorsRead = 0 if the file is not found.
    * executeProgram(name): readFile, copy the program to segment 0x2000 with
      putInMemory, then launchProgram(0x2000).
    * terminate(): reloads the shell. "shell" is built on the stack character by
      character (not a string literal) because literals live in the data segment of
      the program that just ended.
    * handleInterrupt21 now supports: 0 print, 1 read string, 2 read sector,
      3 read file, 4 execute program, 5 terminate. Anything else prints "Bad syscall".
    * main() only calls makeInterrupt21() and executes the shell.
- shell.c: loops printing "SHELL> ", reads a line, and runs
      type <file>   loads and prints a file ("File not found" if missing)
      exec <file>   runs a program ("Program not found" if missing)
      anything else prints "Bad Command!"   (blank line just re-prompts)
  All shell I/O is done with syscall(); it reuses no kernel functions.
- kernel.asm (adds launchProgram), userlib.asm (syscall), tstpr1.c, tstpr2.c as provided.
- loadFile.c as provided, with one fix: it used to write the EOF marker (0xFF) into
  the file after its last byte, which showed up as a stray character when typing a
  text file.
- compileOS.sh builds everything and loads kernel, shell, tstpr1, tstpr2, message.txt
  onto diskc.img (kernel first so it lands at sector 3).

2) HOW TO VERIFY
    ./compileOS.sh
    java -jar simulator.jar        (disk image: diskc.img)
At the SHELL> prompt:
    type messag     -> prints "If this message prints out, then your readFile function is working correctly!"
    exec tstpr2     -> prints "tstpr2 is working!" and returns to the SHELL> prompt
    exec tstpr1     -> prints "tstpr1 is working!" and hangs (expected, tstpr1 never exits)
    type nothere    -> File not found
    exec nothere    -> Program not found
    hello           -> Bad Command!
(File names are limited to 6 characters, so message.txt is "messag".)
We also boot-tested the same diskc.img in QEMU with scripted keystrokes.
