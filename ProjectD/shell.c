/* shell.c - Project C shell
 * Andrew Dergham, Mason Andrade, Chris Felumero
 *
 * All I/O goes through syscall() -> interrupt 0x21. No kernel code is reused.
 * main() must stay the FIRST function in this file.
 */

void print(char*);
int  isCommand(char*, char*);
int  getFileName(char*, int, char*);

int main()
{
    char line[80];
    char name[7];
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
 * (at most 6 characters, like loadFile). returns its length, 0 if none. */
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
