/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "apex_cpu.h"
#include "apex_macros.h"


//reference https://prepinsta.com/c-program/insertion-in-a-queue/
//Used to insert elements in a queue
void insert(int data, APEX_CPU *cpu)
{
    if(cpu->rear == (100 - 1)){
        printf("Overflow");
    }
    else
    {
        if(cpu->front == -1)
        {
            cpu->front =0;
        }
            cpu->rear++;
            cpu->queue[cpu->rear]= data;
        
    }
   
}


//Used to delete elements in a queue
void delete(int data, APEX_CPU *cpu){
    for(int i=cpu->front;i<=cpu->rear;i++){
        if(cpu->queue[i]==data){
            for(int j=i;j<=cpu->rear;j++){
                cpu->queue[j]=cpu->queue[j+1];
            }
            cpu->rear=cpu->rear - 1;
            break;
        }
    }
}

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
         {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
         case OPCODE_CMP:
         {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_ADDL:
         {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }
        case OPCODE_SUBL:
         {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }
        case OPCODE_SUB:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_MUL:{
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_DIV:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_AND:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_OR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

        case OPCODE_LOAD:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }
        case OPCODE_LDR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_STR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rs3,
                   stage->rs1, stage->rs2);
            break;
        }

        case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_BZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }
        case OPCODE_BNZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }
        case OPCODE_NOP:
        {
            printf("%s", stage->opcode_str);
            break;
        }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------------STATE OF ARCHITECTURAL REGISTER FILE---------------");
    printf("\n");
    printf("\n");
    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn)
    {
        cpu->fetch.elapse=0;
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        // printf("Helooooo");
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.rs3 = current_ins->rs3;
        cpu->fetch.imm = current_ins->imm;
        cpu->fetch.instruction_pointer = current_ins->instruction_pointer;
     

        // printf("%d hhhhhhh" + cpu->fetch.imm );
       
        /* Update PC for next instruction */
        cpu->pc += 4;

        /* Copy data from fetch latch to decode latch*/
        cpu->decode = cpu->fetch;

        if (ENABLE_DEBUG_MESSAGES || (cpu->function_value == DISPLAY) || (cpu->function_value == SINGLE_STEP))
        {
            // printf("---------------------CLOCK CYCLE %d-----------------------", cpu->clock);
            printf("\n");
            print_stage_content("Instruction at Fetch Stage--->", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT)
        {
            cpu->fetch.has_insn = FALSE;
        }
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    //Stalling logic for part 2
    switch(cpu->decode.opcode){
        case OPCODE_ADD:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break;    
        }
        case OPCODE_NOP:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break;    
        }
        case OPCODE_SUB:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break; 
        }
        case OPCODE_DIV:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break;     
        }
        case OPCODE_ADDL:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break;      
        }
        case OPCODE_SUBL:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break;    
        }
        case OPCODE_AND:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break;     
        }
        case OPCODE_OR:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break;  
        }
        case OPCODE_XOR:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break;    
        }
        case OPCODE_MOVC:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break; 
        }
        case OPCODE_CMP:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break; 
        }
        case OPCODE_MUL:
        {
            if(cpu->MultiplierFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break; 
        }
        case OPCODE_LOAD:
        {
            if(cpu->LoadStoreFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break; 
        }
        case OPCODE_STORE:
        {
            if(cpu->LoadStoreFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break; 
        }
        case OPCODE_LDR:
        {
            if(cpu->LoadStoreFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break; 
        }
        case OPCODE_STR:
        {
            if(cpu->LoadStoreFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break; 
        }
        case OPCODE_BZ:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break; 
        }
        case OPCODE_BNZ:
        {
            if(cpu->IntegerFU.has_insn){
                cpu->fetch_from_next_cycle=TRUE;
                return;
            }
            break; 
        }
    }
    
    //Pass the instruction only if the register is free(checking that in regs_check)
    if((cpu->regs_check[cpu->decode.rd] == 0) && (cpu->regs_check[cpu->decode.rs3] == 0) && (cpu->regs_check[cpu->decode.rs2] == 0) && (cpu->regs_check[cpu->decode.rs1] == 0) )
    {

    if (cpu->decode.has_insn)
    {
        //These instructions don't posesss destination registers
        if((cpu->decode.opcode != OPCODE_STORE) && (cpu->decode.opcode != OPCODE_STR) && (cpu->decode.opcode != OPCODE_NOP) && (cpu->decode.opcode != OPCODE_BZ) && (cpu->decode.opcode != OPCODE_BNZ) && (cpu->decode.opcode != OPCODE_CMP)){

            /* Read operands from register file based on the instruction type */
            
            //Making the destination register busy by setting it 1
            cpu->regs_check[cpu->decode.rd]=1;
        }
        switch (cpu->decode.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                cpu->IntegerFU = cpu->decode;
                break;
            }
            case OPCODE_NOP:
            {
                cpu->IntegerFU = cpu->decode;
                break;
            }
            case OPCODE_LOAD:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->LoadStoreFU = cpu->decode;
                break;
            }
             case OPCODE_LDR:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                cpu->LoadStoreFU = cpu->decode;
                break;
            }
             case OPCODE_STR:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                 cpu->decode.rs3_value = cpu->regs[cpu->decode.rs3];
                 cpu->LoadStoreFU = cpu->decode;
                break;
            }
             case OPCODE_STORE:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                cpu->LoadStoreFU = cpu->decode;
                break;
            }

            case OPCODE_ADDL:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->IntegerFU = cpu->decode;
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                 cpu->IntegerFU = cpu->decode;
                break;
            }
             case OPCODE_SUB:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                cpu->IntegerFU = cpu->decode;
                break;
            }
            case OPCODE_MUL:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                 cpu->MultiplierFU = cpu->decode;
                break;
            }
            case OPCODE_CMP:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                cpu->IntegerFU = cpu->decode;
                break;
            }
            case OPCODE_DIV:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                cpu->IntegerFU = cpu->decode;
                break;
            }

            case OPCODE_AND:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                 cpu->IntegerFU = cpu->decode;
                break;
            }
            case OPCODE_OR:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                 cpu->IntegerFU = cpu->decode;
                break;
            }
            case OPCODE_XOR:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                 cpu->IntegerFU = cpu->decode;
                break;
            }
            case OPCODE_BNZ:
            {
                 cpu->IntegerFU = cpu->decode;
                break;
            }
            case OPCODE_BZ:
            {
                 cpu->IntegerFU = cpu->decode;
                break;
            }

            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                cpu->IntegerFU = cpu->decode;
                break;
            }
            
            case OPCODE_HALT:
            {
                /* MOVC doesn't have register operands */
                cpu->IntegerFU = cpu->decode;
                break;
            }
        }
     
            //creating a corresponding value for each instruction in instruction_pointer and inserting in queue.
            insert(cpu->decode.instruction_pointer, cpu);
        
        /* Copy data from decode latch to execute latch*/
        
        cpu->decode.has_insn = FALSE;

        //checking terminal value display
        if (ENABLE_DEBUG_MESSAGES || (cpu->function_value == DISPLAY) || (cpu->function_value == SINGLE_STEP))
        {
            print_stage_content("Instruction at Decode_RF Stage--->", &cpu->decode);
        }
        
    }
    }
    else{
            cpu->fetch_from_next_cycle = TRUE;
        }
} 

static void
APEX_IntegerFU(APEX_CPU *cpu)
{
    
     if (cpu->IntegerFU.has_insn)
    {
        switch (cpu->IntegerFU.opcode)
        {
            case OPCODE_ADD:
            {
                
                cpu->IntegerFU.result_buffer
                    = cpu->IntegerFU.rs1_value + cpu->IntegerFU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->IntegerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_SUB:
            {
               
                cpu->IntegerFU.result_buffer
                    = cpu->IntegerFU.rs1_value - cpu->IntegerFU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->IntegerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_DIV:
            {
                
                cpu->IntegerFU.result_buffer
                    = cpu->IntegerFU.rs1_value / cpu->IntegerFU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->IntegerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_AND:
            {
               
                cpu->IntegerFU.result_buffer
                    = cpu->IntegerFU.rs1_value & cpu->IntegerFU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->IntegerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_OR:
            {
                
                cpu->IntegerFU.result_buffer
                    = cpu->IntegerFU.rs1_value | cpu->IntegerFU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->IntegerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_XOR:
            {
                
                cpu->IntegerFU.result_buffer
                    = cpu->IntegerFU.rs1_value ^ cpu->IntegerFU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->IntegerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_BZ:
            {
                if (cpu->zero_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->IntegerFU.pc + cpu->IntegerFU.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            case OPCODE_BNZ:
            {
                if (cpu->zero_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->IntegerFU.pc + cpu->IntegerFU.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            case OPCODE_MOVC: 
            {
                cpu->IntegerFU.result_buffer = cpu->IntegerFU.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->IntegerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_CMP: 
            {
                
            
                /* Set the zero flag based on the condition */
                if ((cpu->IntegerFU.rs1_value - cpu->IntegerFU.rs2_value))
                {
                    cpu->zero_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = TRUE;
                }
                break;
            }
            case OPCODE_ADDL: 
            {
                cpu->IntegerFU.result_buffer
                    = cpu->IntegerFU.rs1_value + cpu->IntegerFU.imm;
                

                /* Set the zero flag based on the result buffer */
                if (cpu->IntegerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_SUBL: 
            {
                cpu->IntegerFU.result_buffer
                    = cpu->IntegerFU.rs1_value - cpu->IntegerFU.imm;
                

                /* Set the zero flag based on the result buffer */
                if (cpu->IntegerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
        }
        if(cpu->queue[cpu->front] == cpu->IntegerFU.instruction_pointer){
            cpu->writeback = cpu->IntegerFU;
            cpu->IntegerFU.has_insn = FALSE;
        }


        if (ENABLE_DEBUG_MESSAGES || (cpu->function_value == DISPLAY) || (cpu->function_value == SINGLE_STEP))
        {
            print_stage_content("Instruction at Integer FU Stage--->", &cpu->IntegerFU);
        }
    }

}


static void
APEX_MultiplierFU(APEX_CPU *cpu)
{
    if (cpu->MultiplierFU.has_insn)
    {
         switch (cpu->MultiplierFU.opcode)
                {
                    case OPCODE_MUL:
                    {
                    
                        cpu->MultiplierFU.result_buffer
                            = cpu->MultiplierFU.rs1_value * cpu->MultiplierFU.rs2_value;

                        /* Set the zero flag based on the result buffer */
                        if (cpu->MultiplierFU.result_buffer == 0)
                        {
                            cpu->zero_flag = TRUE;
                        } 
                        else 
                        {
                            cpu->zero_flag = FALSE;
                        }
                        break;
                    }
                
                        
                }
         if(cpu->MultiplierFU.elapse == 2 )
         {
           
                    if(cpu->queue[cpu->front] == cpu->MultiplierFU.instruction_pointer)
                {
                  cpu->writeback = cpu->MultiplierFU;
                  cpu->MultiplierFU.has_insn = FALSE;
                }
         }
         else
         {
            cpu->MultiplierFU.elapse = cpu->MultiplierFU.elapse + 1;
         }
         if (ENABLE_DEBUG_MESSAGES || (cpu->function_value == DISPLAY) || (cpu->function_value == SINGLE_STEP))
                {
                    print_stage_content("Instruction at Multiplier FU Stage--->", &cpu->MultiplierFU);
                }

        
    }
        
}
    

static void
APEX_LoadStoreFU(APEX_CPU *cpu)
{
    if (cpu->LoadStoreFU.has_insn)
    {
    

            switch (cpu->LoadStoreFU.opcode)
        {

            case OPCODE_LOAD:
            {
               
                cpu->LoadStoreFU.memory_address
                    = cpu->LoadStoreFU.rs1_value + cpu->LoadStoreFU.imm;
                cpu->LoadStoreFU.result_buffer
                    = cpu->data_memory[cpu->LoadStoreFU.memory_address];
                break;
            }
            case OPCODE_LDR:
            {
                cpu->LoadStoreFU.memory_address
                    = cpu->LoadStoreFU.rs1_value + cpu->LoadStoreFU.rs2_value;
                cpu->LoadStoreFU.result_buffer
                    = cpu->data_memory[cpu->LoadStoreFU.memory_address];
                break;
            }
            case OPCODE_STORE:
            {
                cpu->LoadStoreFU.memory_address
                    = cpu->LoadStoreFU.rs2_value + cpu->LoadStoreFU.imm;
                cpu->data_memory[cpu->LoadStoreFU.memory_address]=cpu->LoadStoreFU.rs1_value;
                break;
            }
            case OPCODE_STR:
            {
                cpu->LoadStoreFU.memory_address
                    = cpu->LoadStoreFU.rs1_value + cpu->LoadStoreFU.rs2_value;
                cpu->data_memory[cpu->LoadStoreFU.memory_address]=cpu->LoadStoreFU.rs3_value;
                break;
            }
        }
        if(cpu->LoadStoreFU.elapse==3)
        {
        
        if(cpu->queue[cpu->front] == cpu->LoadStoreFU.instruction_pointer){
            cpu->writeback = cpu->LoadStoreFU;
            cpu->LoadStoreFU.has_insn = FALSE;
        }
             

            
        }
        else{
        
            cpu->LoadStoreFU.elapse = cpu->LoadStoreFU.elapse + 1;
        }
        if (ENABLE_DEBUG_MESSAGES || (cpu->function_value == DISPLAY) || (cpu->function_value == SINGLE_STEP))
            {
            print_stage_content("Instruction at Load Store FU Stage--->", &cpu->LoadStoreFU);
            }
        
       
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        cpu->regs_check[cpu->writeback.rd]=0;
        /* Write result to register file based on instruction type */
        delete(cpu->writeback.instruction_pointer,cpu);
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
            case OPCODE_SUB:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
            case OPCODE_MUL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
            case OPCODE_DIV:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
            case OPCODE_AND:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
            case OPCODE_OR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
            case OPCODE_XOR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
            case OPCODE_STORE:
            {
                
                break;
            }
            case OPCODE_STR:
            {
                
                break;
            }
            case OPCODE_LDR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
            case OPCODE_ADDL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
        }

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES || (cpu->function_value == DISPLAY) || (cpu->function_value == SINGLE_STEP))
        {
            print_stage_content("Instruction at Writeback Stage--->", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->front = -1;
    cpu->rear = -1;
    cpu->itemCount = 0;
    cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}
 
int Apex_function(const char *function_value){
    if((strcmp(function_value,"simulate"))==0){
        return SIMULATE;
    }
    else if((strcmp(function_value,"display"))==0){
        return DISPLAY;
    }
    else if((strcmp(function_value,"single_step"))==0){
        
        return SINGLE_STEP;
    }
    else if((strcmp(function_value,"initialize"))==0){
        
        return INITIALIZE;
    }
    else if((strcmp(function_value,"show_mem"))==0){
        return SHOW_MEM;
    }else{
        return 0;
    }
}
/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        if (ENABLE_DEBUG_MESSAGES || (cpu->function_value == DISPLAY) || (cpu->function_value == SIMULATE) || (cpu->function_value == SINGLE_STEP) )
        {
            if((cpu->clock == cpu->digit)&&(cpu->digit != 0)){
                printf("\n");
                printf("\n");
                print_reg_file(cpu);
                printf("\n");
                printf("\n");
                printf("------------------STATE OF DATA MEMORY-------------------");
                printf("\n");
                printf("\n");
                for(int i=0; i<DATA_MEMORY_SIZE;i++){
                        if(cpu->data_memory[i] != 0){
                            printf("MEM[%d]   ",i);
                        printf(" Data Value = %d",cpu->data_memory[i]);
                        printf("\n");
                }
            }
                printf("Press any key to advance CPU Clock or <q> to quit:\n");
                scanf("%c", &user_prompt_val);

                if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
                  {
                    printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                     break;
                   }

            }
            if((cpu->function_value == DISPLAY) || (cpu->function_value == SINGLE_STEP)){
                    printf("--------------------------------------------\n");
                printf("Clock Cycle #: %d\n", cpu->clock);
                printf("--------------------------------------------\n");
            }
            
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            if((cpu->function_value == SHOW_MEM) && (cpu->digit !=0)){
                // for(int i=0;i<DATA_MEMORY_SIZE;i++){
                //     if(i == cpu->digit){
                //         printf("MEM[%d]   ",i);
                //         printf(" Data Value = %d",cpu->data_memory[i]);
                //         printf("\n");
                //     }
                // }
                        printf("MEM[%d]   ",cpu->digit);
                        printf(" Data Value = %d",cpu->data_memory[cpu->digit]);
                        printf("\n");
            }
            if((cpu->function_value == DISPLAY)||(cpu->function_value == SIMULATE)){
                print_reg_file(cpu);
                    printf("\n");
                printf("\n");
                printf("------------------STATE OF DATA MEMORY-------------------");
                printf("\n");
                printf("\n");
                for(int i=0; i<DATA_MEMORY_SIZE;i++){
                    if(cpu->data_memory[i] != 0){
                        printf("MEM[%d]   ",i);
                        printf(" Data Value = %d",cpu->data_memory[i]);
                        printf("\n");
                    }
                
                }
                }
            break;
        }
        APEX_LoadStoreFU(cpu);
        APEX_MultiplierFU(cpu);
        APEX_IntegerFU(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

      //  print_reg_file(cpu);
       // print_reg_file_stats(cpu);

        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }
    // printf("helllo %d ",cpu->data_memory[1]);
}

/*  
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}