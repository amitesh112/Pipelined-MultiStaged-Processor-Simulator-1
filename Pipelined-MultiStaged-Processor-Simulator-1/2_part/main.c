/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>

#include "apex_cpu.h"

int
main(int argc, char const *argv[])
{
    APEX_CPU *cpu;
    int function_value;
    int digit;
    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);

    if((argc == 3) || (argc == 4)){
        function_value= Apex_function(argv[2]);
        if(!function_value){
            printf("ERROR");
            exit(1);
        }
        if(argc == 4){
            digit = atoi(argv[3]);
        }
    }

    // if (argc != 2)
    // {
    //     fprintf(stderr, "APEX_Help: Usage %s <input_file>\n", argv[0]);
    //     exit(1);
    // }

    cpu = APEX_cpu_init(argv[1]);
    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    
    }
    cpu->function_value=function_value;
    cpu->digit=digit;
    if(function_value == SINGLE_STEP){
        cpu->single_step= 1;
    }
    if(function_value != INITIALIZE){
        APEX_cpu_run(cpu);
        APEX_cpu_stop(cpu);
    }
    return 0;
}