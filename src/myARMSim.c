
/* 

The project is developed as part of Computer Architecture class
Project Name: Functional Simulator for subset of ARM Processor

Developer's Name: Rounaq Jhunjhunu Wala and Shrey Bagroy
Developer's Email id: rounaq14089@iiitd.ac.in | shrey14099@iiitd.ac.in
Date: 15th March 2015

*/


/* myARMSim.cpp
   Purpose of this file: implementation file for myARMSim
*/

#include "myARMSim.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>   //for memset and other string operations.

//Register file
static unsigned int R[16];
//flags
static int N,C,V,Z;
//memory
static unsigned char MEM[4000];

//intermediate datapath and control path signals
static unsigned int instruction_word;
static unsigned int operand1;
static unsigned int operand2;
static unsigned int immediate;
static unsigned int result_reg;
static unsigned int update_flags;
static unsigned int format;
static unsigned int Opcode;
unsigned int ACC;

void run_armsim() {
  while(1) {
    fetch();
    decode();
    execute();
    mem();
    write_back();
    printf("\n\n");
  }
}

// it is used to set the reset values
//reset all registers and memory content to 0
void reset_proc() {
  memset(R,0,sizeof(R));
  R[13] = 0x00000F9C;
  memset(MEM,0,sizeof(MEM));
  N = C = 0;
  V = Z = 0;
  instruction_word = update_flags = 0;
  format = 0;
}

//load_program_memory reads the input memory, and pupulates the instruction 
// memory
void load_program_memory(char *file_name) {
  FILE *fp;
  unsigned int address, instruction;
  fp = fopen(file_name, "r");
  if(fp == NULL) {
    printf("Error opening input mem file\n");
    exit(1);
  }
  while(fscanf(fp, "%x %x", &address, &instruction) != EOF) {
    write_word(MEM, address, instruction);
  }
  fclose(fp);
}

//writes the data memory in "data_out.mem" file
void write_data_memory() {
  FILE *fp;
  unsigned int i;
  fp = fopen("data_out.mem", "w");
  if(fp == NULL) {
    printf("Error opening dataout.mem file for writing\n");
    return;
  }
  
  for(i=0; i < 4000; i = i+4){
    fprintf(fp, "%x %x\n", i, read_word(MEM, i));
  }
  fclose(fp);
}

void opprint()
{
  switch(format)
  {
    case 0:
    switch(Opcode)
    {
      case 0:printf("AND");break;
      case 1:printf("EOR");break;
      case 2:printf("SUB");break;
      case 3:printf("RSB");break;
      case 4:printf("ADD");break;
      case 5:printf("ADC");break;
      case 6:printf("SBC");break;
      case 7:printf("RSC");break;
      case 8:printf("TST");break;
      case 9:printf("TEQ");break;
      case 10:printf("CMP");break;
      case 11:printf("CMN");break;
      case 12:printf("ORR");break;
      case 13:printf("MOV");break;
      case 14:printf("BIC");break;
      case 15:printf("MVN");break; 
    };break;
    case 1:
      switch(update_flags)
      {
        case 0:printf("STR");break;	
        case 1:printf("LDR");break;
      };break;
    case 2:printf("B");
    if(instruction_word & 0x01000000) printf("L");break;
    case 3:printf("SWI");break;
  }
  if(format==DP && (Opcode<8 || Opcode>11) && update_flags) printf("S");
  switch(instruction_word >> 28)
  {
    case 0:printf("EQ");break;
    case 1:printf("NE");break;
    case 2:printf("CS");break;
    case 3:printf("CC");break;
    case 4:printf("MI");break;
    case 5:printf("PL");break;
    case 6:printf("VS");break;
    case 7:printf("VC");break;
    case 8:printf("HI");break;
    case 9:printf("LS");break;
    case 10:printf("GE");break;
    case 11:printf("LT");break;
    case 12:printf("GT");break;
    case 13:printf("LE");break;
    //case 14:printf("AL");break;
  }
}

short int sign_bit(unsigned int ans)
{
  return (ans & (0x80000000LL)) >> 31;
}

unsigned int sign_extend()
{
    unsigned int value = (0x00FFFFFF & instruction_word);
    unsigned int mask = 0x00800000;
    if (mask & instruction_word) {
      value += 0xFF000000;
    }
    return value;
}

//should be called when instruction is swi_exit
void swi_exit() {
  write_data_memory();
  exit(0);
}

unsigned int ALU(int a,int b,int op)
{
  unsigned int ans = 0;
  unsigned int op2 = 0;
  switch(op)
  {
    case 0:printf("AND %d and %d\n",a,b);ans = a&b;break;
    case 1:printf("XOR %d and %d\n",a,b);ans = a^b;break;
    case 2:printf("SUB %d from %d\n",b,a);ans = a-b;op2 = -b;break;
    case 3:printf("SUB %d from %d\n",a,b);ans = b-a;op2 = -a;break;
    case 4:printf("ADD %d and %d\n",a,b);ans = a+b;op2 = b;break;
    case 5:printf("ADD %d and %d with carry\n",a,b);ans = a+b+C;op2 = b+C;break;
    case 6:printf("SUB %d from %d with borrow\n" ,b,a);ans = a-b+C-1;op2 = -b+C-1;break;
    case 7:printf("SUB %d from %d with borrow\n",a,b);ans = b-a+C-1;op2 = -a+C-1;break;
    case 8:printf("TST %d and %d\n",a,b);ans = a&b;break;
    case 9:printf("TEQ %d and %d\n",a,b);ans = a^b;break;
    case 10:printf("CMP %d and %d\n",a,b);ans = a-b;op2 = -b;break;
    case 11:printf("CMP %d and NOT %d\n",a,b);ans = a+b;op2 = b;break;
    case 12:printf("OR %d and %d\n",a,b);ans = a|b;break;
    case 13:printf("return %d\n",b);ans = b;break;
    case 14:printf("AND %d and NOT %d\n",a,b);ans = a & (~b);break;
    case 15:printf("return NOT %d\n",b);ans= ~b;break;
  }
  if(update_flags)
  {
    N = (ans & (0x80000000)) >> 31;
    Z = (ans==0)?1:0;
    C = (ans < a || ans < op2)?1:0;
    V = (sign_bit(a)==sign_bit(op2)&&(sign_bit(ans)!=sign_bit(a)))?1:0;
    printf("CPSR Flags:N: %d, Z: %d, C: %d, V: %d\n",N,Z,C,V);
  }
  return ans;
}

int condition_check(int x)
{
  switch(x)
  {
    case 0:return Z==1?1:0;
    case 1:return Z==0?1:0;
    case 2:return C==1?1:0;
    case 3:return C==0?1:0;
    case 4:return N==1?1:0;
    case 5:return N==0?1:0;
    case 6:return V==1?1:0;
    case 7:return V==0?1:0;
    case 8:return (C==1 && Z==0)?1:0;
    case 9:return (C==1 && Z==0)?0:1;
    case 10:return N==V?1:0;
    case 11:return N!=V?1:0;
    case 12:return (Z==0 && N==V)?1:0;
    case 13:return (Z==1 || N!=V)?1:0;
    case 14:return 1;
  }
  return 1;
}

//reads from the instruction memory and updates the instruction register
void fetch() {
  // Big-Endian Storage (MSB in the first bit)

  if(PC>sizeof(MEM)/sizeof(char)) {printf("error: PC : Invalid reference - out of range\n");swi_exit();}
  if(PC%4) {printf("error: PC: Unaligned Memory Access\n");swi_exit();}
  instruction_word = read_word(MEM,PC);  // Fetching Instruction from PC
  printf("FETCH     :Fetch instruction 0x%x from address 0x%x\n",instruction_word,PC);
  PC+=4;  //Incrementing PC to next Instruction
}
//reads the instruction register, reads operand1, operand2 from register file, decides the operation to be performed in execute stage
void decode() {
  //Instruction in instruction_word
  immediate = 0;
  // Using Masking and bit-shifts to decode instruction.
  //Format: {31-28}COND{27-26}FMT{25}I{24-21}Opcode{20}S{19-16}Rn{15-12}Rd{11-0}Op2
  operand1 = R[(instruction_word & (0xF0000))>>16];
  if(instruction_word & (0x2000000)) {immediate=1;operand2 = instruction_word & (0xFF);} //If Immediate operand
  else operand2 = R[instruction_word & (0xF)]; //If not immediate.
  
  result_reg = (instruction_word & (0xF000)) >> 12;  // Index of result Register
  if(instruction_word & (0x100000)) update_flags = 1; // Check S bit
  else update_flags = 0;
  Opcode = (instruction_word >> 21) & 0xF;
  format = (instruction_word & (0xC000000)) >> 26;
  if(format==BRANCH) operand2 = instruction_word & 0x7FFFFF;
  if(format==DP && Opcode >= 8 && Opcode <=11) update_flags = 1; //Update for CMP,CMN,TST,TEQ
  //For Data Transfer Instructions,I is inverted
  if(format == DT)
  {
    if(instruction_word & (0x2000000)) operand2 = R[instruction_word & (0xF)];
    else {operand2 = instruction_word & (0xFFF);immediate=1;}
    operand2 = (instruction_word & (0x800000))?operand2:(~operand2)+1; //For negative offsets
  }
  printf("DECODE    :Operation is ");opprint();
  if(format == DT || format == DP) 
  {
    if(format==DP && (Opcode==13 || Opcode==15)) printf(", first operand ignored");
    else printf(", first operand R%d",(instruction_word & (0xF0000))>>16);
    printf(", Second operand ");
    if(immediate) printf("%d,",operand2);
    else printf("R%d,",instruction_word & (0xF));
    if(format==DT && update_flags==0) printf(" source register R%d\n",result_reg); //STR Instruction
    else printf(" destination register R%d\n",result_reg);
  }
  else if(format == BRANCH) printf(", Offset %d\n",sign_extend()+1);
  else if(format == SWI) printf(", interrupt parameter 0x%x\n",instruction_word & 0xFFFFFF);
}
//executes the ALU operation based on ALUop
// Supported Formats: BRANCH, DP ,SWI
void execute() {
  printf("EXECUTE   :");
  if(format == DT) {printf("no execute operation\n");return;}
  if(condition_check(instruction_word >> 28))
  { 
    switch(format)
    {
      case DP:ACC = ALU(operand1,operand2,Opcode);break;
      case SWI: if(instruction_word == 0xEF000011) {printf("software interrupt: terminate execution\n");swi_exit();}
              else {printf("error: SWI Code Unrecognizable\n");swi_exit();}
      case BRANCH:printf("branching to instruction stored at 0x%x",PC+(sign_extend()+1)*4);
      if(instruction_word & 0x01000000) {printf(",return address 0x%x stored to link register",PC);LR=PC;}
      PC+= (sign_extend()+1)*4;printf("\n");break;
    }
  }
  else printf("execution condition not satisfied.\n");
}
//perform the memory operation
//Supported Formats: DT
void mem()
{
  printf("MEMORY    :");  
  if(format != DT) {printf("No Memory operation\n");return;}
  if(condition_check(instruction_word >> 28))
  {
    if((operand1+operand2)>sizeof(MEM)/sizeof(char)) {printf("error: DTI : Invalid reference - out of range\n");swi_exit();}
    if((operand1+operand2)%4) {printf("error: DTI: Unaligned Memory Access\n");swi_exit();}
    if(update_flags) 
    {
      printf("loading data from memory address 0x%x to accumulator.\n",operand1+operand2);
      ACC = read_word(MEM,operand1+operand2); //LDR
    }
    else 
    {
      printf("storing data to memory address 0x%x\n",operand1+operand2);
      write_word(MEM,operand2+operand1,R[result_reg]);
    }
  }
  else printf("execution condition not satisfied.\n");
}
//writes the results back to register file
void write_back() {
  int i;
  short DoWrite = 1;
  if(format == DP) switch(Opcode) {case 8:case 9:case 10:case 11:DoWrite = 0;}
  else if(format == DT) { if(!update_flags) DoWrite = 0; } //STR has S bit 0
  else if(format == BRANCH || format == SWI) DoWrite=0;
  if(DoWrite && condition_check(instruction_word >> 28)) 
  { printf("WRITEBACK :write %d to R%d\n",ACC,result_reg);R[result_reg] = ACC;}
  else printf("WRITEBACK :no writeback operation\n");
}


int read_word(char *mem, unsigned int address) {
  int *data;
  data =  (int*) (mem + address);
  return *data;
}

void write_word(char *mem, unsigned int address, unsigned int data) {
  int *data_p;
  data_p = (int*) (mem + address);
  *data_p = data;
}