////////////////////////////////
// CONFIGURATION
////////////////////////////////

#ifndef MAX_ARRAY_ELEMS_PER_LINE
#define MAX_ARRAY_ELEMS_PER_LINE 4
#endif

#ifndef SHOW_WARNING_INVALID_INSTRUCTION
#define SHOW_WARNING_INVALID_INSTRUCTION 0
#endif

#ifndef SHOW_INSTR_USED
#define SHOW_INSTR_USED 0
#endif

#ifndef HAS_AC_END_SIMULATION
#define HAS_AC_END_SIMULATION 0
#endif

#ifndef COUNT_SYSCALLS
#define COUNT_SYSCALLS 0
#endif

/* #ifndef PROCESSOR_OPTIMIZATIONS */
/* #define PROCESSOR_OPTIMIZATIONS 0 */
/* #endif */
int PROCESSOR_OPTIMIZATIONS=0;

#ifndef REGION_SIZE
#define REGION_SIZE 9
#endif

/* #ifndef REGION_BLOCK_SIZE */
/* #define REGION_BLOCK_SIZE 30 */
/* #endif */
int REGION_BLOCK_SIZE=300;

#ifndef EXIT_ADDRESS
#define EXIT_ADDRESS 0x64
#endif

////////////////////////////////
// COUNT_SYSCALLS
////////////////////////////////

#if COUNT_SYSCALLS == 0
#define COUNT_SYSCALLS_INC(NAME) ""
#else
#define COUNT_SYSCALLS_INC(NAME) "syscall_count[_" #NAME "]++;"
#endif



void COUNT_SYSCALLS_Init(FILE *output)
{
  fprintf( output,
           "//!Count the number of executions for each system call\n"
           "enum syscall_enum {\n  ");

#define AC_SYSC(NAME,LOCATION) fprintf(output, "_" #NAME ", ")
#include "ac_syscall.def"
#undef AC_SYSC

  fprintf( output,
           "\n"
           "  SYSCALLS_NUM\n"
           "};\n"
           "\n"
           "int syscall_count[SYSCALLS_NUM];\n"
           "\n"
           );
}


void COUNT_SYSCALLS_EmitPrintStat(FILE *output)
{
  fprintf(output, "  fprintf(stderr, \"--- Syscalls ---\\n\");\n");

#define AC_SYSC(NAME,LOCATION) fprintf(output, "  fprintf(stderr, \"    " #NAME ": %%d\\n\", syscall_count[_" #NAME "]);\n")
#include "ac_syscall.def"
#undef AC_SYSC

}





////////////////////////////////
// PROCESSOR_OPTIMIZATIONS
////////////////////////////////

//#define MAX_BRANCH 30
/* char branch_instr[][10] = {"call", "ba", "bn", "bne", "be", "bg", "ble", "bge", "bl", "bgu", */
/*                            "bleu", "bcc", "bcs", "bpos", "bneg", "bvc", "bvs", "jmpl_reg", "jmpl_imm", 0}; */



void PROCESSOR_OPTIMIZATIONS_PutBreakOrNot(FILE *output, ac_dec_instr *pinstr, int j)
{
  static int is_branch = 0;
  int index;

  if (pinstr->cflow)
    is_branch = pinstr->cflow->delay_slot + 1;
  if (is_branch > 0) {
    is_branch--;

  if (ACMulticoreFlag == 1)
	  {
	    fprintf( output, "      ac_instr_counter += (0x%x - old_pc)/4 + 1;\n", j);
	    fprintf( output, "      old_pc = ac_pc;\n");
	  }
	  fprintf( output, "      break;\n");
   }

}


//Prototypes
void PROCESSOR_OPTIMIZATIONS_EmitInstr(FILE *output, int j);
void PROCESSOR_OPTIMIZATIONS_EmitInstr_1(FILE *output, int j);
void PROCESSOR_OPTIMIZATIONS_EmitInstr_2(FILE *output, int j);
void PROCESSOR_OPTIMIZATIONS_EmitInstr_3(FILE *output, int j);
void PROCESSOR_OPTIMIZATIONS_EmitInstr_4(FILE *output, int j);
void PROCESSOR_OPTIMIZATIONS_EmitInstr_5(FILE *output, int j);


void PROCESSOR_OPTIMIZATIONS_EmitInstr(FILE *output, int j)
{
  //Selects which optimization to call
  switch (PROCESSOR_OPTIMIZATIONS) {

  case 1:
    //Optimization 1:
    //  - original branch optimization: remove breaks when not jump
    PROCESSOR_OPTIMIZATIONS_EmitInstr_1(output, j);
    break;

  case 2:
    //Optimization 2:
    //  - detect control: generate special code and break only after them (or delay slots)
    PROCESSOR_OPTIMIZATIONS_EmitInstr_2(output, j);
    break;

  case 3:
    //Optimization 3:
    //  - detect leaders: label only the leaders
    //  - detect control: break only after them (or delay slots)
    PROCESSOR_OPTIMIZATIONS_EmitInstr_3(output, j);
    break;

  default:
    AC_ERROR("Processor optimization selection not found.\n");
  }
}    


void PROCESSOR_OPTIMIZATIONS_EmitInstr_1(FILE *output, int j)
{
  ac_dec_instr *pinstr = GetInstrByID(decoder->instructions, decode_table[j]->dec_vector[0]);
  fprintf( output, "    case 0x%x:\n", j);
  accs_EmitInstrExtraTop(output, j, pinstr, 6);
  accs_EmitInstrBehavior(output, j, pinstr, 6);
  accs_EmitInstrExtraBottom(output, j, pinstr, 6);
  PROCESSOR_OPTIMIZATIONS_PutBreakOrNot(output, pinstr, j);
  fprintf( output, "\n");
}


void PROCESSOR_OPTIMIZATIONS_EmitInstr_2(FILE *output, int j)
{
  int instr_index;
  ac_dec_instr *pinstr = GetInstrByID(decoder->instructions, decode_table[j]->dec_vector[0]);
  ac_control_flow *cflow = pinstr->cflow;
  int jump_instr_size = pinstr->size;

  if (!cflow) {
    fprintf( output, "    case 0x%x:\n", j);
    accs_EmitInstrExtraTop(output, j, pinstr, 6);
    accs_EmitInstrBehavior(output, j, pinstr, 6);
    accs_EmitInstrExtraBottom(output, j, pinstr, 6);
    PROCESSOR_OPTIMIZATIONS_PutBreakOrNot(output, pinstr, j);
    fprintf( output, "\n");
  }
  else {
    fprintf( output, "    case 0x%x:\n", j);
    fprintf( output, "      //instr: %s, delay=%d (%s)\n", pinstr->name,
             cflow->delay_slot, cflow->delay_slot_cond);

    accs_EmitInstrExtraTop(output, j, pinstr, 6);
    fprintf( output, "      if (%s) {\n", accs_SubstFields(cflow->cond, j));
    fprintf( output, "        tmp_pc = %s;\n", accs_SubstFields(cflow->target, j));
    if ((cflow->action) && (cflow->action[0]))
      fprintf( output, "        %s\n", accs_SubstFields(cflow->action, j));
    fprintf( output, "      }\n");
    fprintf( output, "      else {\n");
    fprintf( output, "        tmp_pc = %#x;\n", j + pinstr->size + (cflow->delay_slot * pinstr->size));
    fprintf( output, "      }\n");
 
    if (ACMulticoreFlag == 1)
	    fprintf( output, "      ac_instr_counter += (0x%x-old_pc)/4 + 1;\n",j);
    	
    accs_EmitInstrExtraBottom(output, j, pinstr, 6);
  
    //Check for delay slot and execute it too
    pinstr = ((decode_table[j+jump_instr_size]) && (decode_table[j+jump_instr_size]->dec_vector)) ?
      GetInstrByID(decoder->instructions, decode_table[j+jump_instr_size]->dec_vector[0]) :
      0;

    if (cflow->delay_slot) {

      //If delay slot instruction is decoded
      if (pinstr) {
        char *args = accs_Fields2Str(decode_table[j+jump_instr_size]);
        fprintf( output, "      if (%s) {\n", accs_SubstFields(cflow->delay_slot_cond, j));
        accs_EmitInstrExtraTop(output, j+jump_instr_size, pinstr, 8);
        accs_EmitInstrBehavior(output, j+jump_instr_size, pinstr, 8);

	if (ACMulticoreFlag == 1)
		fprintf( output, "        ac_instr_counter++;\n");

        accs_EmitInstrExtraBottom(output, j+jump_instr_size, pinstr, 8);
        fprintf( output, "      }\n");
      }
      else {
        fprintf( output, "      fprintf(stderr, \"Instruction at ac_pc=%#x reached but not decoded.\\n\");\n", j+jump_instr_size);
        fprintf( output, "      ac_stop(1);\n");
      }
    }

    fprintf( output, "      ac_pc = tmp_pc;\n");
    if (ACMulticoreFlag == 1)
	    fprintf( output, "      old_pc = ac_pc;\n");
    fprintf( output, "      break;\n");

    fprintf( output, "\n");
  }
}


void PROCESSOR_OPTIMIZATIONS_EmitInstr_3(FILE *output, int j)
{
  static unsigned NEXT_START_REGION=0;
  ac_dec_instr *pinstr = GetInstrByID(decoder->instructions, decode_table[j]->dec_vector[0]);
  char *args = accs_Fields2Str(decode_table[j]);

  //put case in leaders
  if (decode_table[j]->is_leader) {
    fprintf( output, "    // IS LEADER from %#x:\n", decode_table[j]->is_leader);
    fprintf( output, "    case 0x%x:\n", j);
  }
  //put case in start Region (which is the first instruction >= the start of a region)
  else if (j >= NEXT_START_REGION) {
    fprintf( output, "    // IS LEADER from START REGION\n");
    fprintf( output, "    case 0x%x:\n", j);
    NEXT_START_REGION += (1 << REGION_SIZE);
  }
  else {
    fprintf( output, "                               // 0x%x\n", j);
  }

  accs_EmitInstrExtraTop(output, j, pinstr, 6);
  accs_EmitInstrBehavior(output, j, pinstr, 6);
  accs_EmitInstrExtraBottom(output, j, pinstr, 6);
  PROCESSOR_OPTIMIZATIONS_PutBreakOrNot(output, pinstr, j);
  fprintf( output, "\n");
}


