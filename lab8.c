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

// Main CPU loop: fetches, decodes instructions until program ends
void CPU( char *mem){
    unsigned int machineCode = 0;   
    unsigned char opcode = 0;
    PCRegister = CODESECTION;  // at the beginning, PCRegister is the starting point,
                       // it should be a global integer defined in header.h
    do{
      printf("\nPC:%x\n", PCRegister);
      machineCode = CPU_fetchCode(mem, PCRegister); // Fetch 32-bit instruction from memory at current PC
      if (machineCode == 0)  // quit the program when machineCode is 0, that is the end of the code.
          break;  // break the infinite loop. 
      PCRegister += 4;                                                     // update the program counter
      opcode = CPU_Decode(machineCode); // Decode instruction to identify operation type
      printf("Decoded Opcode is: %02X. \n", opcode);

      // Lab 9: Finish the execution of the code.
      // Only finish this part when the CPU_Decode is done.
      // CPU_Execution(opcode, machineCode, mem);
    }while (1);  // This is an infinite while loop
                 // When you fetch a machineCode of 00000000, the loop breaks.
    printRegisterFiles();     // After the code execution, print all the register contents on screen.
    printDataMemoryDump(mem); // After the code execution, print the memory dump of the data section.
}

// Lab 8 - Step 1. Finish the CPU_fectchCode function to
//         read the code section from memory and
//         get the 32-bit machine code from the memory.
// Fetch 4 bytes from memory and combine into a 32-bit instruction
unsigned int CPU_fetchCode(char *mem, int codeOffset){
    unsigned int machineCode = 0;
    // Combine 4 consecutive bytes into one 32-bit instruction (big-endian format)
    machineCode |= ((unsigned char)mem[codeOffset]) << 24;
    machineCode |= ((unsigned char)mem[codeOffset + 1]) << 16;
    machineCode |= ((unsigned char)mem[codeOffset + 2]) << 8;
    machineCode |= ((unsigned char)mem[codeOffset + 3]);

    return machineCode;
}

// Lab 8 - Step 2. Finish the CPU_Decode function to
//         decode the instruction and return the
//         opcode/function of the instruction.
//         Hints: Need to consider how to find the opcode/function from different types of instructions:
//                i.e., I-, J- and R-type instructions. 
//                The return value should be a 6-bit bianry code. 
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
        return funct;   // R-type instruction
    else
        return opcode;  // I-type / J-type instruction
}
// Lab 9: Finish the function CPU_Execution to run all the instructions.
// Executes instruction based on opcode (Lab 9)
void CPU_Execution(unsigned char opcode, unsigned int machineCode, char *mem){
    unsigned char rt = 0;
    switch (opcode)  // execute different functions when opcode is set differently. 
    {
		// This is an example how lab will be executed. Please follow this example and finish exections of the code.
		// Hint: you need to implement the following instructions here: 
		//       la, add, lb, bge, lw, sw, addi, j
        case 0b101111:   //"la" instruction. 
            // assign the address rt = immediate address stored in machineCode;
            // first find the rt index in the register array. 
            rt = (machineCode & 0x001F0000) >> 16; 
            // assign the address stored in immediate field to regFile[rt];
            regFile[rt] = machineCode & 0x0000FFFF;  // get the last 16 bit as address. 
            // update PCregister ???? Pay special attention to branch instructions. 
            PCRegister += 4;
            if (DEBUG_CODE){   // print the hints to the user in DEBUG_MODE
                printf("Code Executed: %08X\n", machineCode);
                printf("****** PC Register is %08X ******\n", PCRegister);                
            }            
            break;
        case 0b100000://"lb" instruction. 
		    //....
			break;
		
        // continue to all the other cases used in the program.
        // case ......:    
            
            
        // Should never go to default part when complete. Otherwise, that is a mistake. 
        default:
            printf("Wrong instruction! You need to fix this instruction %02X %08X\n", opcode,  machineCode);
            system("PAUSE");
            exit(3);  // exit the program if running here.     
            break;
    }
}
// Lab 8 - Step 3. Print all the 32 registers in regFile and names saved in
//         regNameTab. For example, it should print
//         $zero = 0x00000000
//         $at  = ... ... etc.
// Display all register values after program execution
void printRegisterFiles(){
    int i;
    printf("\n===== Register File Dump =====\n");
    for (i = 0; i < N_REG; i++){
        printf("%s = 0x%08X\n", regNameTab[i], regFile[i]);
    }
}

 // Lab 8 - Step 4. Call function memory_dump and pass the proper parameters to dump the first 256
//          bytes from Data section.
// Display first 256 bytes of data memory
void printDataMemoryDump(char *mem){
    printf("\n===== Data Memory Dump =====\n");
    memory_dump(mem, DATASECTION, 256);
}
