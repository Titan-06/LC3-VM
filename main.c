#include <stdio.h>
#include <stdlib.h>


#define MEMORY_MAX ( 1 << 16 )

__uint16_t memory[MEMORY_MAX];
__uint16_t registers[8]; // The 8 general purpose registers

__uint16_t PC; // Program Counter ( Contains Which Memory address to fetch next )
__uint16_t PSR; // Processor status register

enum GENERAL_REGISTERS{
    R0 = 0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
};

int load_program(int argc, char * argv[])
{
    if(argc != 2)
    {
        printf("Usage: ./main [PATH OF PROGRAM]\n");
        return 0;
    }
    FILE *fptr;

    fptr = fopen(argv[1],"rb");

    if(fptr == NULL){
        printf("No such file found.\n");
        return 0;
    }

    int address = 0x3000;
    while(fread(&memory[address++],sizeof(__uint16_t),1,fptr));

    fclose(fptr);

    return 1;
}

int main(int argc, char * argv[])
{

    // Load the program into memory 
    if(!load_program(argc,argv))
    {
        printf("Something went wrong.\n");
        exit(1);
    }


    PSR = 1 << 1; // Initially set to zero condition code and supervisor mode 
    PC = 0x3000; // Starting position of memory for the user programs
    printf("memory[0x3000]=%u memory[0x3001]=%u\n", memory[PC], memory[PC + 1]);
    return  0;
}