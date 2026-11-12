/* kernel.c - Project C: Loading Files and Executing Programs
 * Andrew Dergham, Mason Andrade, Chris Felumero
 *
 * NOTE: main() must stay the FIRST function in this file (the bootloader
 * jumps to offset 0 of the kernel).
 */

void printChar(char);
void printString(char*);
void readString(char*);
void readSector(char*, int);
void readFile(char*, char*, int*);
void executeProgram(char*);
void terminate();
void handleInterrupt21(int, int, int, int);
void writeSector(char*, int);
void deleteFile(char* filename);
void writeFile(char* buffer, char* filename, int numberOfSectors);

int main()
{
    char shellname[6];

    makeInterrupt21();

    /* build "shell" on the stack (see terminate() for why) */
    shellname[0] = 's';
    shellname[1] = 'h';
    shellname[2] = 'e';
    shellname[3] = 'l';
    shellname[4] = 'l';
    shellname[5] = '\0';

    /* load and run the shell; this call never returns */
    interrupt(0x21, 4, shellname, 0, 0);

    while (1) {}
    return 0;
}

/* ---------------- Project B functions ---------------- */

void printChar(char c)
{
    interrupt(0x10, 0xE * 256 + c, 0, 0, 0);
}

void printString(char* msg)
{
    int i = 0;
    while (msg[i] != '\0')
    {
        interrupt(0x10, 0xE * 256 + msg[i], 0, 0, 0);
        i++;
    }
}

void readString(char* input)
{
    int i = 0;
    char letter;
    while (1)
    {
        letter = interrupt(0x16, 0, 0, 0, 0);

        if (letter == 0xD)                  /* ENTER */
        {
            interrupt(0x10, 0xE * 256 + 0xD, 0, 0, 0);
            interrupt(0x10, 0xE * 256 + 0xA, 0, 0, 0);
            input[i] = 0xA;
            i++;
            input[i] = 0x0;
            break;
        }
        else if (letter == 0x8)             /* BACKSPACE */
        {
            if (i > 0)
            {
                i--;
                interrupt(0x10, 0xE * 256 + 0x8, 0, 0, 0);
                interrupt(0x10, 0xE * 256 + ' ', 0, 0, 0);
                interrupt(0x10, 0xE * 256 + 0x8, 0, 0, 0);
            }
        }
        else if (i < 78)                    /* leave room for 0xA and 0x0 */
        {
            input[i] = letter;
            i++;
            interrupt(0x10, 0xE * 256 + letter, 0, 0, 0);
        }
    }
}

void readSector(char* buffer, int sector)
{
    int ax = 2 * 256 + 1;           /* AH=2 (read), AL=1 sector */
    int cx = 0 * 256 + (sector + 1);/* CH=track 0, CL=relative sector */
    int dx = 0 * 256 + 0x80;        /* DH=head 0, DL=hard disk */
    interrupt(0x13, ax, buffer, cx, dx);
}

/* ---------------- Project C functions ---------------- */

/* Load the file called `name` into `buffer`.
 * *sectorsRead is set to the number of sectors loaded (0 = not found). */
void readFile(char* name, char* buffer, int* sectorsRead)
{
    char dir[512];
    int fileentry;
    int i;
    int j;
    int match;
    int sector;

    *sectorsRead = 0;

    readSector(dir, 2);                     /* directory lives at sector 2 */

    for (fileentry = 0; fileentry < 512; fileentry = fileentry + 32)
    {
        if (dir[fileentry] == 0)            /* empty directory slot */
            continue;

        /* compare up to 6 characters; stop early when the name ends */
        match = 1;
        for (i = 0; i < 6; i++)
        {
            if (name[i] != dir[fileentry + i])
            {
                match = 0;
                break;
            }
            if (name[i] == 0)               /* both names ended together */
                break;
        }
        if (match == 0)
            continue;

        /* found it: read every sector listed, until a 0 entry */
        for (j = 0; j < 26; j++)
        {
            sector = dir[fileentry + 6 + j] & 0xFF;  /* chars are signed */
            if (sector == 0)
                break;
            readSector(buffer, sector);
            buffer = buffer + 512;
            *sectorsRead = *sectorsRead + 1;
        }
        return;
    }
}

/* Load program `name` into segment 0x2000 and jump to it.
 * Only returns if the program could not be found. */
void executeProgram(char* name)
{
    char buffer[13312];
    int sectorsRead;
    int i;

    readFile(name, buffer, &sectorsRead);
    if (sectorsRead == 0)
        return;                             /* caller reports the error */

    for (i = 0; i < sectorsRead * 512; i++)
        putInMemory(0x2000, i, buffer[i]);

    launchProgram(0x2000);
}

/* Program finished: go back to the shell.
 * "shell" is built on the stack, NOT written as a string literal, because
 * literals live in the data segment of whichever program is running, and
 * that program has just ended. */
void terminate()
{
    char shellname[6];

    shellname[0] = 's';
    shellname[1] = 'h';
    shellname[2] = 'e';
    shellname[3] = 'l';
    shellname[4] = 'l';
    shellname[5] = '\0';

    executeProgram(shellname);
    while (1) {}                            /* shell missing: hang */
}

/* Interrupt 0x21 dispatcher.
 *   AX=0 print string     BX=string
 *   AX=1 read string      BX=buffer
 *   AX=2 read sector      BX=buffer, CX=sector
 *   AX=3 read file        BX=name, CX=buffer, DX=int* sectors read
 *   AX=4 execute program  BX=name
 *   AX=5 terminate program
 *
 * When a user program (the shell) calls us, DS is the USER's segment, so
 * string literals in this file would read garbage. The error path below
 * therefore prints with character constants, not a string literal. */
void handleInterrupt21(int ax, int bx, int cx, int dx)
{
    if (ax == 0)
        printString((char*)bx);
    else if (ax == 1)
        readString((char*)bx);
    else if (ax == 2)
        readSector((char*)bx, cx);
    else if (ax == 3)
        readFile((char*)bx, (char*)cx, (int*)dx);
    else if (ax == 4)
        executeProgram((char*)bx);
    else if (ax == 5)
        terminate();
    else if (ax == 6)
        writeSector((char*)bx, cx);
    else if (ax == 7)
        deleteFile((char*)bx);
    else if(ax == 8)
        writeFile((char*)bx, (char*)cx, dx);
    else
    {
        printChar('B'); printChar('a'); printChar('d'); printChar(' ');
        printChar('s'); printChar('y'); printChar('s'); printChar('c');
        printChar('a'); printChar('l'); printChar('l');
        printChar('\r'); printChar('\n');
    }
}

/* Write the contents of buffer to the specified disk sector. */
void writeSector(char* buffer, int sector)
{
    int ax = 3 * 256 + 1;            /* AH=3 (write), AL=1 sector */
    int cx = 0 * 256 + (sector + 1);/* CH=track 0, CL=relative sector */
    int dx = 0 * 256 + 0x80;        /* DH=head 0, DL=hard disk */
    interrupt(0x13, ax, buffer, cx, dx);
}

/* Find the file called `filename` and delete it.
 * Free its sectors in the disk map and remove its directory entry. */
void deleteFile(char* filename)
{
    char map[512];
    char directory[512];
    int i;
    int j;
    int k;
    int match;
    int sector;

    /* Read disk map and directory */
    readSector(map, 1);
    readSector(directory, 2);

    /* Check each directory entry */
    for (i = 0; i < 512; i = i + 32)
    {
        /* Skip empty or deleted files */
        if (directory[i] == '\0')
            continue;

        match = 1;

        /* Compare filenames */
        for (j = 0; j < 6; j++)
        {
            if (filename[j] != directory[i + j])
            {
                match = 0;
                break;
            }

            if (filename[j] == '\0')
                break;
        }

        /* Skip files that don't match */
        if (match == 0)
            continue;

        /* Free the file's sectors */
        for (k = 0; k < 26; k++)
        {
            sector = directory[i + 6 + k] & 0xFF;

            if (sector == 0)
                break;

            map[sector] = 0;
        }

        /* Mark the file as deleted */
        directory[i] = '\0';

        /* Save updated map and directory */
        writeSector(map, 1);
        writeSector(directory, 2);

        return;
    }
}

void writeFile(char* buffer, char* filename, int numberOfSectors){
    char map[512];
    char dir[512];
    char sectorBuf[512];
    int entry, i, j, sector, namefinish;
   
    readSector(map, 1);
    readSector(dir,2);


    for (entry =0; entry < 16; entry++)
        if (dir[entry * 32] == '\0') {break;}
    if (entry == 16) {return;}


    namefinish = 0;
    for(i = 0; i<6; i++) {
        if (!namefinish && filename[i] != '\0')
            dir[entry *32 +i] = filename[i];
        else {
            namefinish = 1;
            dir[entry * 32 + i] = '\0';
        }
    }
   
    for (j =0; j <numberOfSectors; j++){
        for (sector = 3; sector < 256; sector++)
            if(map[sector] == 0) {break;}
        if(sector == 256) {return;}


        map[sector] = 0xFF;
        dir[entry * 32 + 6 +j] = sector;


        for(i =0; i<512; i++)
            sectorBuf[i] = buffer[j * 512 + i];
        writeSector(sectorBuf,sector);
    }


    for(j = numberOfSectors; j<26;j++)
        dir[entry * 32 +6 +j] =0;


    writeSector(map, 1);
    writeSector(dir,2);
    
}
