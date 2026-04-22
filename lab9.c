/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
// Please finish the following functions for lab 8.
// Lab 8 will perform the following functions:
//   1. Fetch the code stored in memory
//   2. Decode the code and prepare for the execution of the code.
//   3. Setup the execution function for CPU.

// Lab 9 will perform the following functions:
//   4. Execute the code stored in the memory and print the results. 
#include "header.h"
#include "lab8header.h"

extern char *regNameTab[N_REG];
unsigned int PCRegister = 0; // PC register, always pointing to the next instruction.

// Main CPU cycle: fetch → decode → execute until program ends
void CPU(char *mem){
    unsigned int machineCode = 0;   
    unsigned char opcode = 0;
    PCRegister = CODESECTION;  // at the beginning, PCRegister is the starting point

    do{
      printf("\nPC:%x\n", PCRegister);

      // Fetch 32-bit instruction from memory at current PC
      machineCode = CPU_fetchCode(mem, PCRegister);

      if (machineCode == 0)  // quit when machineCode is 0
          break;

      // Decode instruction to identify operation type
      opcode = CPU_Decode(machineCode);
      printf("Decoded Opcode is: %02X. \n", opcode);

      // Lab 9: Execute the decoded instruction
      CPU_Execution(opcode, machineCode, mem);

    }while (1);

    printRegisterFiles();     // After execution, print all register contents
    printDataMemoryDump(mem); // After execution, print the memory dump of the data section
}

// Fetch 4 bytes from memory and combine into a 32-bit instruction
unsigned int CPU_fetchCode(char *mem, int codeOffset){
    unsigned int machineCode = 0;

    // Code bytes are stored in little-endian order in memory
    machineCode |= ((unsigned char)mem[codeOffset]);
    machineCode |= ((unsigned char)mem[codeOffset + 1]) << 8;
    machineCode |= ((unsigned char)mem[codeOffset + 2]) << 16;
    machineCode |= ((unsigned char)mem[codeOffset + 3]) << 24;

    return machineCode;
}

// Decode machine code into opcode and instruction fields
unsigned char CPU_Decode(unsigned int machineCode)
{
    unsigned char opcode;
    unsigned char rs, rt, rd, funct;
    unsigned short immediate;

    // Extract opcode (first 6 bits)
    opcode = (machineCode >> 26) & 0x3F;
    // Extract source register rs
    rs     = (machineCode >> 21) & 0x1F;
    // Extract target register rt
    rt     = (machineCode >> 16) & 0x1F;
    // Extract destination register rd
    rd     = (machineCode >> 11) & 0x1F;
    // Extract function code (for R-type instructions)
    funct  = machineCode & 0x3F;
    immediate = machineCode & 0xFFFF;

    printf("\nInstruction: 0x%08X\n", machineCode);
    printf("Opcode: %02X\n", opcode);
    printf("rs: %d, rt: %d, rd: %d\n", rs, rt, rd);
    printf("funct: %02X\n", funct);
    printf("immediate: %04X\n", immediate);

    if (opcode == 0)
        return funct + 0x40;   // R-type instruction, shifted to avoid conflict with I-type opcodes
    else
        return opcode;         // I-type / J-type instruction
}

// Execute the instruction based on decoded opcode
void CPU_Execution(unsigned char opcode, unsigned int machineCode, char *mem){
    unsigned char rs = 0, rt = 0, rd = 0;
    short immediate = 0;
    int address = 0;

    switch (opcode)
    {
        case 0b101111:   //"la" instruction
            rt = (machineCode & 0x001F0000) >> 16;
            regFile[rt] = machineCode & 0x0000FFFF;
            PCRegister += 4;

            if (DEBUG_CODE){
                printf("Code Executed: %08X\n", machineCode);
                printf("LA: regFile[%d] = %08X\n", rt, regFile[rt]);
                printf("****** PC Register is %08X ******\n", PCRegister);
            }
            break;

        case 0x60:   //"add" instruction (R-type funct 0x20 + 0x40)
            rd = (machineCode & 0x0000F800) >> 11;
            rs = (machineCode & 0x03E00000) >> 21;
            rt = (machineCode & 0x001F0000) >> 16;

            regFile[rd] = regFile[rs] + regFile[rt];
            PCRegister += 4;

            if (DEBUG_CODE){
                printf("Code Executed: %08X\n", machineCode);
                printf("ADD: regFile[%d] = regFile[%d] + regFile[%d] = %08X\n",
                       rd, rs, rt, regFile[rd]);
                printf("****** PC Register is %08X ******\n", PCRegister);
            }
            break;

        case 0b001000:   //"addi" instruction
            rs = (machineCode & 0x03E00000) >> 21;
            rt = (machineCode & 0x001F0000) >> 16;
            immediate = machineCode & 0x0000FFFF;

            regFile[rt] = regFile[rs] + immediate;
            PCRegister += 4;

            if (DEBUG_CODE){
                printf("Code Executed: %08X\n", machineCode);
                printf("ADDI: regFile[%d] = regFile[%d] + %d = %08X\n",
                       rt, rs, immediate, regFile[rt]);
                printf("****** PC Register is %08X ******\n", PCRegister);
            }
            break;

        case 0b100000:   //"lb" instruction
            rs = (machineCode & 0x03E00000) >> 21;
            rt = (machineCode & 0x001F0000) >> 16;
            immediate = machineCode & 0x0000FFFF;

            address = regFile[rs] + immediate;
            regFile[rt] = (signed char) read_byte(mem, address);
            PCRegister += 4;

            if (DEBUG_CODE){
                printf("Code Executed: %08X\n", machineCode);
                printf("LB: regFile[%d] = mem[%08X] = %08X\n",
                       rt, address, regFile[rt]);
                printf("****** PC Register is %08X ******\n", PCRegister);
            }
            break;
        
        case 0b100011:   //"lw" instruction
            rs = (machineCode & 0x03E00000) >> 21;
            rt = (machineCode & 0x001F0000) >> 16;
            immediate = machineCode & 0x0000FFFF;

            address = regFile[rs] + immediate;
            regFile[rt] = read_dword(mem, address);
            PCRegister += 4;

            if (DEBUG_CODE){
                printf("Code Executed: %08X\n", machineCode);
                printf("LW: regFile[%d] = mem[%08X] = %08X\n",
                       rt, address, regFile[rt]);
                printf("****** PC Register is %08X ******\n", PCRegister);
            }
            break;
        
        case 0b101011:   //"sw" instruction
            rs = (machineCode & 0x03E00000) >> 21;
            rt = (machineCode & 0x001F0000) >> 16;
            immediate = machineCode & 0x0000FFFF;

            address = regFile[rs] + immediate;
            write_dword(mem, address, regFile[rt]);
            PCRegister += 4;

            if (DEBUG_CODE){
                printf("Code Executed: %08X\n", machineCode);
                printf("SW: mem[%08X] = regFile[%d] = %08X\n",
                       address, rt, regFile[rt]);
                printf("****** PC Register is %08X ******\n", PCRegister);
            }
            break;

        case 0b000010:   //"j" instruction
        {
            unsigned int target;

            target = (machineCode & 0x03FFFFFF) << 2;
            PCRegister = target;

            if (DEBUG_CODE){
                printf("Code Executed: %08X\n", machineCode);
                printf("JUMP to address: %08X\n", PCRegister);
            }
        }
            break;
        
        case 0b000001:   //"bge" instruction
            rs = (machineCode & 0x03E00000) >> 21;
            rt = (machineCode & 0x001F0000) >> 16;

            if (regFile[rs] >= regFile[rt])
                PCRegister = (machineCode & 0x0000FFFF) << 2;
            else
                PCRegister += 4;

            if (DEBUG_CODE){
                printf("Code Executed: %08X\n", machineCode);
                printf("BGE: if regFile[%d] >= regFile[%d]\n", rs, rt);
                printf("****** PC Register is %08X ******\n", PCRegister);
            }
            break;

        
        default:
            printf("Wrong instruction! You need to fix this instruction %02X %08X\n", opcode, machineCode);
            system("PAUSE");
            exit(3);
            break;
    }
}

// Display all register values after program execution
void printRegisterFiles(){
    int i;
    printf("\n===== Register File Dump =====\n");
    for (i = 0; i < N_REG; i++){
        printf("%s = 0x%08X\n", regNameTab[i], regFile[i]);
    }
}

// Display first 256 bytes of data memory
void printDataMemoryDump(char *mem){
    printf("\n===== Data Memory Dump =====\n");
    memory_dump(mem, DATASECTION, 256);
}
