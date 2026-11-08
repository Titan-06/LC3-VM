#include <stdio.h>
#include <stdlib.h>


#define MEMORY_MAX ( 1 << 16 )
#define START_ADDRESS 0x3000

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

enum OPCODES{ // 15 OPCODES in LC 3 and One reserved code
    OP_BR = 0,
    OP_ADD,
    OP_LD,
    OP_ST,
    OP_JSR,
    OP_AND,
    OP_LDR,
    OP_STR,
    OP_RTI,
    OP_NOT,
    OP_LDI,
    OP_STI,
    OP_RET,
    OP_RESERVED,
    OP_LEA,
    OP_TRAP
};

// Op code helper functions
__uint16_t sext(__uint16_t num,__uint8_t sig_dig)
{
    __uint16_t mask = (0xFFFF << sig_dig);
    if( !((1 << sig_dig) & num) ) mask = 0;
    return num | mask;
}
__uint16_t setcc(int16_t number)
{
    if(number > 0){
        PSR = 0b1;
    }else if(number < 0){
        PSR = 0b100;
    }else{
        PSR = 0b10; 
    }
}

// Op code instruction 
void op_add(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9; // Get the DR out
    __uint8_t sr1 = (instr & 0b111000000) >> 6; // Get the first source register
    if(instr & 0b100000)
    {
        u_int8_t imm5 = instr & 0b11111;
        registers[dr] = registers[sr1] + sext(imm5,5);
    }else{
        __uint8_t sr2 = instr & 0b111;
        registers[dr] = registers[sr1] + registers[sr2];
    }
    setcc(registers[dr]);
}
void op_and(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9; // Get the DR out
    __uint8_t sr1 = (instr & 0b111000000) >> 6; // Get the first source register
    if(instr & 0b100000)
    {
        __uint8_t imm5 = instr & 0b11111;
        registers[dr] = registers[sr1] & sext(imm5,5);
    }else{
        __uint8_t sr2 = instr & 0b111;
        registers[dr] = registers[sr1] & registers[sr2];
    }
    setcc(registers[dr]);
}


int load_program(int argc, char * argv[]) // Loads the program returns the size of the program
{
    if(argc != 2)
    {
        printf("Usage: ./main [PATH OF PROGRAM]\n");
        return 0;
    }
    int size = 0;
    FILE *fptr;

    fptr = fopen(argv[1],"rb");

    if(fptr == NULL){
        printf("No such file found.\n");
        return 0;
    }

    int address = START_ADDRESS;
    // Shifting to acheive big endian 
    __uint16_t temp;
    while(fread(&temp,sizeof(__uint16_t),1,fptr))
    {
        __uint16_t first = temp << 8;
        temp = temp >> 8;
        memory[address++] = first | temp;
        size++;

    }

    fclose(fptr);

    return size;
}

int main(int argc, char * argv[])
{
    u_int16_t program_size = load_program(argc,argv);
    // Load the program into memory 
    if(!program_size)
    {
        printf("Something went wrong.\n");
        exit(1);
    }
    printf("Instructions loaded in memory: %u\n",program_size);

    PSR = 1 << 1; // Initially set to zero condition code and supervisor mode 
    PC = START_ADDRESS; // Starting position of memory for the user programs
    while(PC != START_ADDRESS + program_size)
    {
        __uint16_t instr = memory[PC];
        __uint8_t opcode = instr >> 12;
        printf("opcode ran: %u\n",opcode);
        switch (opcode)
        {
            case OP_BR:
                /* code */
                break;
            case OP_ADD:
                op_add(instr);
                break;
            case OP_LD:
                /* code */
                break;
            case OP_ST:
                /* code */
                break;
            case OP_JSR:
                /* code */
                break;
            case OP_AND:
                op_and(instr);
                break;
            case OP_LDR:
                /* code */
                break;
            case OP_STR:
                /* code */
                break;
            case OP_RTI:
                /* code */
                break;
            case OP_NOT:
                /* code */
                break;
            case OP_LDI:
                /* code */
                break;
            case OP_STI:
                /* code */
                break;
            case OP_RET:
                /* code */
                break;
            case OP_LEA:
                /* code */
                break;
            case OP_TRAP:
                /* code */
                break;

            default:
                break;
        }
        printf("CC set to: %d\n",PSR);
        PC++;
    }
    printf("REG 0=%d REG 1=%d REG 2=%d\n", registers[0], registers[1], registers[2]);
    return  0;
}