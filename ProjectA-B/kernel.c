// Andrew Dergham, Mason Andrade, Chris Felumero
void printString(char*);
void readString(char*);
void readSector(char* , int);
void handleInterrupt21(int, int, int, int);
void makeInterrupt21();
void printChar(char);

int main()
{
    char line[80];
    char diskBuffer[512];

    makeInterrupt21();

    // Test system call 0: printString
    interrupt(0x21, 0, (int)"System calls are working!", 0, 0);

    printChar('\r');
    printChar('\n');

    // Test system call 1: readString
    interrupt(0x21, 1, (int)line, 0, 0);

    // Test system call 0: printString (echo input)
    interrupt(0x21, 0, (int)line, 0, 0);

    printChar('\r');
    printChar('\n');

    // Test system call 2: readSector
    interrupt(0x21, 2, (int)diskBuffer, 30, 0);

    // Null terminate the buffer
    diskBuffer[511] = '\0';

    // Print sector contents
    interrupt(0x21, 0, (int)diskBuffer, 0, 0);

    printChar('\r');
    printChar('\n');

    while(1){}

    return 0;
}




    //char input[80];
    //char buffer[512];
    
    // Step 4
    //makeInterrupt21();
    //interrupt(0x21,0,0,0,0);

    //readSector(buffer, 30);
    //printString(buffer);

    //readString(input); // read first and fill array
    //printString(input);


void printChar(char c)
{
    interrupt(0x10, 0xE *256+c,0, 0, 0);
}

void printString(char* msg)
{
    int letterCount = 0;
    while(msg[letterCount] !='\0') //\'0' is a special character that shows up at 
    //the end of char*
    {
        // name of the interrupt 0x10, 0xe*256 is some math i dont understand , the 
        //other zeros are the other registrys
        interrupt(0x10, 0xe*256+msg[letterCount], 0, 0, 0);
        letterCount++;
    }
}


void readSector(char* buffer, int sector){
    int ah = 2;
    int al = 1;
    int ch = 0;
    int cl = sector + 1;
    int dh = 0;
    int dl = 0x80;
    int ax = ah * 256 + al;
    int cx = ch * 256 + cl;
    int dx = dh * 256 + dl;
    interrupt(0x13, ax, buffer, cx, dx);

}



void readString(char* input)
{
    int i = 0;
    char letter;
    while(1)
    {
        letter = interrupt(0x16, 0, 0, 0, 0);
        
        if(letter == 0xD)//this means if enter is pressed exit
        {
            interrupt(0x10, 0xe*256+0xD, 0, 0, 0); //moves cursor to the start of line
            interrupt(0x10, 0xe*256+0xA, 0, 0, 0); // move cursor down a line
    
            input[i] = 0xA; //moves cursor down a line in printString
            i++;
            input[i] = 0x0; //where printString ends

            break;
            
        }
        else if(letter == 0x8) //this is backspace handling
        {
            if(i > 0)
            {
            i--;

            interrupt(0x10, 0xe*256+0x8, 0, 0, 0); // moves the cursor back
            interrupt(0x10, 0xe*256+' ', 0, 0, 0); // visbly gets rid of letter 
            //after backspace
            interrupt(0x10, 0xe*256+0x8, 0, 0, 0);

            }
        }
        else if(i <78)//prevents overflow
        {
        input[i] = letter;
        i++;
        
        interrupt(0x10,0xe*256+letter, 0, 0, 0);//prints the letter as you write
        }
    }
}


// Step 4: called by interrupt21ServiceRoutine (kernel.asm) on every interrupt 0x21
void handleInterrupt21(int ax, int bx, int cx, int dx)
{
    //printString("Hello world\r\n");
    if(ax == 0){printString((char*)bx);}
    else if(ax == 1){readString((char*)bx);}
    else if(ax == 2){readSector((char*)bx, cx);}
    else{printString("Error: Invalid system call");
    printChar('\r');
    printChar('\n');
         }
}

