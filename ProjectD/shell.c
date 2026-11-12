/* shell.c - Project D shell
 * Andrew Dergham, Mason Andrade, Chris Felumero
 *
 * All I/O goes through syscall() -> interrupt 0x21.
 * main() must stay the FIRST function in this file.
 *
 * Commands: type, exec, dir, del, copy, create
 */

void print(char*);
int  isCommand(char*, char*);
int  getFileName(char*, int, char*);
int  getSecondName(char*, int, char*);
void dir();
void create(char*, char*);


int main()
{
    char line[80];
    char name[7];
    char name2[7];
    char buffer[13313];     /* max file size (13312) + room for a 0 */
    int sectors;

    while (1)
    {
        print("SHELL> ");
        syscall(1, line, 0, 0);             /* read a line */

        if (line[0] == 0xA || line[0] == 0)
            continue;                       /* empty line: prompt again */

        if (isCommand(line, "type"))
        {
            if (getFileName(line, 4, name) == 0)
            {
                print("Usage: type filename\r\n");
                continue;
            }
            sectors = 0;
            syscall(3, name, buffer, &sectors);     /* read file */
            if (sectors > 0)
            {
                buffer[sectors * 512] = 0;          /* make sure it ends */
                print(buffer);
                print("\r\n");
            }
            else
                print("File not found\r\n");
        }
        else if (isCommand(line, "exec"))
        {
            if (getFileName(line, 4, name) == 0)
            {
                print("Usage: exec filename\r\n");
                continue;
            }
            syscall(4, name, 0, 0);     /* only returns if it failed */
            print("Program not found\r\n");
        }
        else if (isCommand(line, "dir"))
        {
            dir();
        }
        else if (isCommand(line, "del"))
        {
            if (getFileName(line, 3, name) == 0)
            {
                print("Usage: del filename\r\n");
                continue;
            }
            syscall(7, name, 0, 0);     /* delete file */
        }
        else if (isCommand(line, "copy"))
        {
            if (getFileName(line, 4, name) == 0 ||
                getSecondName(line, 4, name2) == 0)
            {
                print("Usage: copy file1 file2\r\n");
                continue;
            }
            sectors = 0;
            syscall(3, name, buffer, &sectors);     /* read source */
            if (sectors == 0)
                print("File not found\r\n");
            else
                syscall(8, buffer, name2, sectors); /* write copy */
        }
        else if (isCommand(line, "create"))
        {
            if (getFileName(line, 6, name) == 0)
            {
                print("Usage: create filename\r\n");
                continue;
            }
            create(name, buffer);       /* reuses main's buffer */
        }
        else
            print("Bad Command!\r\n");
    }
}

/* print via system call 0 */
void print(char* s)
{
    syscall(0, s, 0, 0);
}

/* 1 if line starts with cmd followed by a space, end of line, or end of string */
int isCommand(char* line, char* cmd)
{
    int i = 0;
    while (cmd[i] != 0)
    {
        if (line[i] != cmd[i])
            return 0;
        i++;
    }
    if (line[i] == ' ' || line[i] == 0xA || line[i] == 0)
        return 1;
    return 0;
}

/* copy the argument that starts after position `start` into `name`
 * (at most 6 characters). returns its length, 0 if none. */
int getFileName(char* line, int start, char* name)
{
    int i = start;
    int n = 0;

    while (line[i] == ' ')                  /* skip spaces */
        i++;
    while (n < 6 && line[i] != ' ' && line[i] != 0xA && line[i] != 0)
    {
        name[n] = line[i];
        n++;
        i++;
    }
    name[n] = 0;
    return n;
}

/* skips the first argument after `start`, copies the second one into `name`.
 * returns its length, 0 if there isn't one. */
int getSecondName(char* line, int start, char* name)
{
    int i = start;
    int n = 0;

    while (line[i] == ' ')                                          /* spaces before 1st arg */
        i++;
    while (line[i] != ' ' && line[i] != 0xA && line[i] != 0)        /* skip 1st arg */
        i++;
    while (line[i] == ' ')                                          /* spaces before 2nd arg */
        i++;

    while (n < 6 && line[i] != ' ' && line[i] != 0xA && line[i] != 0)
    {
        name[n] = line[i];
        n++;
        i++;
    }
    name[n] = 0;
    return n;
}

/* list every file that is not deleted */
void dir()
{
    char directory[512];
    char filename[7];
    int i;
    int j;

    syscall(2, directory, 2, 0);            /* read directory (sector 2) */

    for (i = 0; i < 512; i = i + 32)
    {
        if (directory[i] != '\0')           /* skip deleted/empty entries */
        {
            for (j = 0; j < 6; j++)
                filename[j] = directory[i + j];
            filename[6] = '\0';             /* 6-char names have no terminator on disk */

            print(filename);
            print("\r\n");
        }
    }
}

/* prompt for lines until an empty one, then write them all to file `name`.
 * `textBuf` is the shell's shared buffer (13313 bytes). */
void create(char* name, char* textBuf)
{
    char line[80];
    int pos;
    int i;
    int sectors;

    for (i = 0; i < 13313; i++)
        textBuf[i] = 0;
    pos = 0;

    while (1)
    {
        print("> ");
        syscall(1, line, 0, 0);

        if (line[0] == 0xA || line[0] == 0)         /* empty line ends input */
            break;

        for (i = 0; line[i] != 0xA && line[i] != 0 && pos < 13300; i++)
        {
            textBuf[pos] = line[i];
            pos++;
        }
        textBuf[pos] = '\r';
        pos++;
        textBuf[pos] = '\n';
        pos++;
    }

    if (pos > 0)
    {
        sectors = (pos + 511) /512;            /* use your div helper if / isn't available */
        syscall(8, textBuf, name, sectors);
    }
}
