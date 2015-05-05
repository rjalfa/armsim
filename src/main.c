/* 

The project is developed as part of Computer Architecture class
Project Name: Functional Simulator for subset of ARM Processor

Developer's Name: Rounaq Jhunjhunu Wala and Shrey Bagroy
Developer's Email id: rounaq14089@iiitd.ac.in | shrey14099@iiitd.ac.in
Date: 15th March 2015

*/


/* main.cpp 
   Purpose of this file: The file handles the input and output, and
   invokes the simulator
*/


#include "myARMSim.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  char* prog_mem_file; 
  if(argc < 2) {
    printf("Incorrect number of arguments. Please invoke the simulator \n\t./myARMSim <input mem file> \n");
    exit(1);
  }
  
  //reset the processor
  reset_proc();
  //load the program memory
  load_program_memory(argv[1]);
  //run the simulator
  run_armsim();

  return 1;
}
