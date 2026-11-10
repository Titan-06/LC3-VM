#include <stdio.h>
#include <stdlib.h>


#define MEMORY_MAX ( 1 << 16 )
#define START_ADDRESS 0x3000

__uint16_t memory[MEMORY_MAX];
__uint16_t registers[8]; // The 8 general purpose registers

__uint16_t PC; // Program Counter Register ( Contains Which Memory address to fetch next )
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

void op_br(__uint16_t instr)
{
    __uint8_t conditions = instr >> 9;
    __uint16_t offset = instr & 0b111111111;
    if(!conditions){
        PC = PC + sext(offset,9);
        return;
    }
    if(conditions & PSR){
        PC = PC + sext(offset,9);
    }
}

void op_jmp_ret(__uint16_t instr)
{
    __uint8_t baser = (instr & 0b111000000) >> 6;
    PC = registers[baser];
}

void op_jsr(__uint16_t instr)
{
    registers[R7] = PC;
    if(instr & 0b100000000000){
        __uint16_t offset = instr & 0b11111111111;
        PC = PC + sext(offset,11);
    }else{
        __uint8_t baser = (instr & 0b111000000) >> 6;
        PC = registers[baser];
    }
}

void op_ld(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9;
    __uint16_t offset = instr & 0b111111111;

    registers[dr] = memory[PC + sext(offset,9)];
    setcc(registers[dr]);
}

void op_ldi(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9;
    __uint16_t offset = instr & 0b111111111;

    __uint16_t target = memory[PC + sext(offset,9)];
    registers[dr] = memory[target];

    setcc(registers[dr]);
}

void op_ldr(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9;
    __uint8_t sr = (instr & 0b111000000) >> 6;
    __uint8_t offset = (instr & 0b111111);

    registers[dr] = memory[registers[sr] + sext(offset,6)];
    setcc(registers[dr]);
}

void op_lea(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9;
    __uint16_t offset = instr & 0b111111111;

    registers[dr] = PC + sext(offset,9);
}

void op_not(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9;
    __uint8_t sr = (instr & 0b111000000) >> 6;

    registers[dr] = ~registers[sr];
    setcc(registers[dr]);
}

void op_st(__uint16_t instr)
{
    __uint8_t sr = (instr & 0b111000000000) >> 9;
    __uint16_t offset = instr & 0b111111111;

    memory[PC + sext(offset,9)] = registers[sr];
}

void op_sti(__uint16_t instr)
{
    __uint8_t sr = (instr & 0b111000000000) >> 9;
    __uint16_t offset = instr & 0b111111111;

    __uint16_t target = memory[PC + sext(offset,9)];

    memory[target] = registers[sr];
}

void op_str(__uint16_t instr)
{
    __uint8_t sr = (instr & 0b111000000000) >> 9;
    __uint8_t baser = (instr & 0b111000000) >> 6;
    __uint8_t offset = (instr & 0b111111);

    memory[registers[baser] + sext(offset,6)] = registers[sr];

}

void op_trap(){
    exit(0);
}
/* Necessary Functions */

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

        PC++; // Move on to the next PC

        printf("opcode ran: %u\n",opcode);
        switch (opcode)
        {
            case OP_BR:
                op_br(instr);
                break;
            case OP_ADD:
                op_add(instr);
                break;
            case OP_LD:
                op_ld(instr);
                break;
            case OP_ST:
                op_st(instr);
                break;
            case OP_JSR:
                op_jsr(instr);
                break;
            case OP_AND:
                op_and(instr);
                break;
            case OP_LDR:
                op_ldr(instr);
                break;
            case OP_STR:
                op_str(instr);
                break;
            case OP_RTI:
                /* code */
                break;
            case OP_NOT:
                op_not(instr);
                break;
            case OP_LDI:
                op_ldi(instr);
                break;
            case OP_STI:
                op_sti(instr);
                break;
            case OP_RET:
                op_jmp_ret(instr);
                break;
            case OP_LEA:
                op_lea(instr);
                break;
            case OP_TRAP:
                printf("REG 1=%d REG 2=%d REG 3 = %d memory = %d memoory = %d", registers[1], registers[2],registers[3], memory[0x300A],memory[0x300B]);
                op_trap();
                break;

            default:
                break;
        }
        printf("CC set to: %d\n",PSR);
    }
    printf("REG 0=%d REG 1=%d REG 2=%d REG 3 = %d REG 7 = %d\n", registers[0], registers[1], registers[2],registers[3], registers[R7]);
    return  0;
}