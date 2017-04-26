/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      acsim.c
 * @author    Sandro Rigo
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:20 -0300
 *
 * @brief     The ArchC pre-processor.
 *            This file contains functions to control the ArchC
 *            to emit the source files that compose the behavioral
 *            simulator.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "acsim.h"
#include "acpp.h"
#include "stdlib.h"
#include "string.h"
 #include <stdbool.h>


//#define DEBUG_STORAGE

// High Level Trace
#ifdef HLT_SUPPORT

// default configuration
#define HLT_OBJ "ac_hltrace.o"
#define HLT_HEADER "ac_hltrace.H"
#else

// if ArchC configured with --disable-hlt
#define HLT_OBJ
#define HLT_HEADER
#endif




//Defining Traces and Dasm strings
#define PRINT_TRACE "%strace_file << hex << ac_pc << dec <<\"\\n\";\n"

//Command-line options flags
int  ACABIFlag=1;                               //!<Indicates whether an ABI was provided or not
int  ACDebugFlag=0;                             //!<Indicates whether debugger option is turned on or not
int  ACHLTraceFlag=0;                           //!<Indicates whether high level trace option is turned on or not
int  ACDecCacheFlag=1;                          //!<Indicates whether the simulator will cache decoded instructions or not
int  ACDelayFlag=0;                             //!<Indicates whether delay option is turned on or not
int  ACDDecoderFlag=0;                          //!<Indicates whether decoder structures are dumped or not
int  ACStatsFlag=0;                             //!<Indicates whether statistics collection is enable or not
int  ACVerboseFlag=0;                           //!<Indicates whether verbose option is turned on or not
int  ACGDBIntegrationFlag=0;                    //!<Indicates whether gdb support will be included in the simulator
int  ACWaitFlag=1;                              //!<Indicates whether the instruction execution thread issues a wait() call or not
int  ACThreading=1;                             //!<Indicates if Direct Threading Code is turned on or not
int  ACSyscallJump=1;                           //!<Indicates if Syscall Jump Optimization is turned on or not
int  ACForcedInline=1;                          //!<Indicates if Forced Inline in Interpretation Routines is turned on or not
int  ACLongJmpStop=1;                           //!<Indicates if New Stop using longjmp is turned on or not
int  ACIndexFix=0;                              //!<Indicates if Index Decode Cache Fix Optimization is turned on or not
int  ACPCAddress=1;                             //!<Indicates if PC bounds is verified or not
int  ACFullDecode=0;                            //!<Indicates if Full Decode Optimization is turned on or not
int  ACCurInstrID=1;                            //!<Indicates if Current Instruction ID is save in dispatch
int  ACPowerEnable=0;                           //!<Indicates if Power Estimation is enabled

char ACOptions[500];                            //!<Stores ArchC recognized command line options
char *ACOptions_p = ACOptions;                  //!<Pointer used to append options in ACOptions
char *arch_filename;                            //!<Stores ArchC arquitecture file

int ac_host_endian;                             //!<Indicates the endianness of the host machine
extern int ac_tgt_endian;                       //!<Indicates the endianness of the host machine
int ac_match_endian;                            //!<Indicates whether host and target endianness match on or not

//! This structure describes one command-line option mapping.
/*!  It is used to manage command line options, following gcc style. */
struct option_map
{
  const char *name;                          //!<The long option's name.
  const char *equivalent;                    //!<The equivalent short-name for options.
  const char *arg_desc;                      //!<The option description.
  /* Argument info.  A string of flag chars; NULL equals no options.
     r => argument required.
     o => argument optional.
     * => require other text after NAME as an argument.  */
  const char *arg_info;                      //!<Argument info.  A string of flag chars; NULL equals no options.
};

/*!Decoder object pointer */
ac_decoder_full *decoder;

/*!Storage device used for loading applications */
ac_sto_list* load_device=0;

/*! This is the table of mappings.  Mappings are tried sequentially
  for each option encountered; the first one that matches, wins.  */
struct option_map option_map[] = {
  {"--abi-included"    , "-abi","Indicate that an ABI for system call emulation was provided." ,"o"},
  {"--abi-not-included", "-noabi","Indicate that an ABI for system call emulation was NOT provided." ,"o"},
  {"--debug"           , "-g"  ,"Enable simulation debug features: traces, update logs." ,"o"},
#ifdef HLT_SUPPORT
  {"--high-level-trace", "-hlt","Enable generation of high level traces" ,"o"},
#endif
  {"--delay"           , "-dy" ,"Enable delayed assignments to storage elements." ,"o"},
  {"--dumpdecoder"     , "-dd" ,"Dump the decoder data structure." ,"o"},
  {"--help"            , "-h"  ,"Display this help message."       , 0},
  {"--no-dec-cache"    , "-ndc","Disable cache of decoded instructions." ,"o"},
  {"--stats"           , "-s"  ,"Enable statistics collection during simulation." ,"o"},
  {"--verbose"         , "-vb" ,"Display update logs for storage devices during simulation.", "o"},
  {"--version"         , "-vrs","Display ACSIM version.", 0},
  {"--gdb-integration" , "-gdb","Enable support for debbuging programs running on the simulator.", 0},
  {"--no-wait"         , "-nw" ,"Disable wait() at execution thread.", 0},
  {"--no-threading"    , "-nt" ,"Disable Direct Threading Code.", 0},
  {"--no-syscall-jump" , "-nsj","Disable Syscall Jump Optimization.", 0},
  {"--no-forced-inline", "-nfi","Disable Forced Inline in Interpretation Routines.", 0},
  {"--no-new-stop"     , "-nns","Disable New Stop Optimization.", 0},
  {"--index-fix"       , "-idx","Enable Index Decode Cache Fix Optimization.", 0},
  {"--no-pc-addr-ver"  , "-npv","Disable PC address verification.", 0},
  {"--full-decode"     , "-fdc","Enable Full Decode Optimization.", 0},
  {"--no-curr-instr-id", "-nci","Disable Current Instruction ID save in dispatch.", 0},
  {"--power"           , "-pw" ,"Enable Power Estimation.", 0},
  { }
};


/*! Display the command line options accepted by ArchC.  */
static void DisplayHelp (){
  int i;
  char line[]="====================";

  line[strlen(ACVERSION)+1] = '\0';

  printf ("===============================================%s\n", line);
  printf (" This is the ArchC Simulator Generator version %s\n", ACVERSION);
  printf ("===============================================%s\n\n", line);
  printf ("Usage: acsim input_file [options]\n");
  printf ("       Where input_file stands for your AC_ARCH description file.\n\n");
  printf ("Options:\n");

  for( i=0; i< ACNumberOfOptions; i++)
    printf ("    %-18s, %-7s %s\n", option_map[i].name,
            option_map[i].equivalent, option_map[i].arg_desc);

  printf ("\nFor more information please visit www.archc.org\n\n");
}

/*! Function for decoder to get bits from the instruction */
/*    PARSER-TIME VERSION: Should not be called */
unsigned long long GetBits(void *buffer, int *quant, int last, int quantity, int sign)
{
  AC_ERROR("GetBits(): This function should not be called in parser-time for interpreted simulator.\n");
  return 1;
};

/*!If target is little endian, this function inverts fields in each format. This
  is necessary in order to the decoder works in little endian architectures.
  \param formats The list of parsed formats containing fields to be inverted.
 */
void invert_fields(ac_dec_format *format)
{
  ac_dec_field *field;

  while (format) {
    int size = format->size;
    field = format->fields;
    while (field) {
      field->first_bit = size - 2 - field->first_bit + field->size;
      field = field->next;
    }
    format = format->next;
  }
}


/**********************************************************/
/*!Writes a standard comment at the beginning of each file
   generated by ArchC.
   OBS: Description must have 50 characteres at most!!
  \param output The output file pointer.
  \param description A brief description of the file being emitted.*/
/**********************************************************/
void print_comment( FILE* output, char* description ){
  fprintf( output, "/******************************************************\n");
  fprintf( output, " * %-50s *\n",description);
  fprintf( output, " * This file is automatically generated by ArchC      *\n");
  fprintf( output, " * WITHOUT WARRANTY OF ANY KIND, either express       *\n");
  fprintf( output, " * or implied.                                        *\n");
  fprintf( output, " * For more information on ArchC, please visit:       *\n");
  fprintf( output, " * http://www.archc.org                               *\n");
  fprintf( output, " *                                                    *\n");
  fprintf( output, " * The ArchC Team                                     *\n");
  fprintf( output, " * Computer Systems Laboratory (LSC)                  *\n");
  fprintf( output, " * IC-UNICAMP                                         *\n");
  fprintf( output, " * http://www.lsc.ic.unicamp.br                       *\n");
  fprintf( output, " ******************************************************/\n");
  fprintf( output, " \n\n");
}


//////////////////////////////////////////
/*!Main routine of  ArchC pre-processor.*/
//////////////////////////////////////////
int main(int argc, char** argv) {
  extern char *project_name, *isa_filename;
  extern int wordsize;
  extern int fetchsize;

  // Structures to be passed to the decoder generator
  extern ac_dec_format *format_ins_list;
  extern ac_dec_instr *instr_list;

  // Structures to be passed to the simulator generator
  extern ac_stg_list *stage_list;
  extern ac_pipe_list *pipe_list;
  extern int HaveFormattedRegs;
  extern int HaveTLMIntrPorts;
/***/
  extern int HaveTLM2IntrPorts;

  extern ac_decoder_full *decoder;

  //Uncomment the line bellow if you want to debug the parser.
  //extern int yydebug;

  int argn,i,j;
  endian a, b;

  int error_flag=0;

  //Uncomment the line bellow if you want to debug the parser.
  //yydebug =1;

  //Initializes the pre-processor
  acppInit(0);

  ++argv, --argc;  /* skip over program name */

  //First argument must be the file or --help option.
  if ( argc > 0 ){
    if( !strcmp(argv[0], "--help") | !strcmp(argv[0], "-h")){
      DisplayHelp();
      return 0;
    }

    if( !strcmp(argv[0], "--version") | !strcmp(argv[0], "-vrs")){
      printf("This is ArchC version %s\n", ACVERSION);
      return 0;
    }

    if(!acppLoad(argv[0])){
      AC_ERROR("Invalid input file: %s\n", argv[0]);
      printf("   Try acsim --help for more information.\n");
      return EXIT_FAILURE;
    }
    arch_filename = argv[0];
  }
  else{
    AC_ERROR("No input file provided.\n");
    printf("   Try acsim --help for more information.\n");
    return EXIT_FAILURE;
  }

  ++argv, --argc;  /* skip over arch file name */

  if( argc > 0) {
    argn = argc;
    /* Handling command line options */
    for(j = 0; j < argn; j++) {
      /* Searching option map.*/

      for(i = 0; i < ACNumberOfOptions; i++) {
        if( (!strcmp(argv[0], option_map[i].name)) ||
            (!strcmp(argv[0], option_map[i].equivalent))){


          switch (i) {
            case OPABI:
              ACABIFlag = 1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPNOABI:
              ACABIFlag = 0;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPDebug:
              ACDebugFlag = 1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
#if HLT_SUPPORT
            case OPHLTrace:
              ACHLTraceFlag = 1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
#endif
            case OPDelay:
              ACDelayFlag = 1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPDDecoder:
              ACDDecoderFlag = 1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPDecCache:
              ACDecCacheFlag = 0;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPStats:
              ACStatsFlag = 1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPVerbose:
              ACVerboseFlag = 1;
              AC_MSG("Simulator running on verbose mode.\n");
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPGDBIntegration:
              ACGDBIntegrationFlag=1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPWait:
              ACWaitFlag = 0;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPDTC:
              ACThreading = 0;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPSysJump:
              ACSyscallJump = 0;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPForcedInline:
              ACForcedInline = 0;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPLongJmpStop:
              ACLongJmpStop = 0;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPIndexFix:
              ACIndexFix = 1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPPCAddress:
              ACPCAddress = 0;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPFullDecode:
              ACFullDecode = 1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPCurInstrID:
              ACCurInstrID = 0;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPPower:
              ACPowerEnable = 1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
            default:
              break;
          }
          ++argv, --argc;  /* skip over founded argument */
          break;
        }
      }
    }
  }

  if(argc > 0) {
    AC_ERROR("Invalid argument %s.\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ( !ACDecCacheFlag ) ACFullDecode = 0;

  //Loading Configuration Variables
  ReadConfFile();

  /*Parsing Architecture declaration file. */
  AC_MSG("Parsing AC_ARCH declaration file: %s\n", arch_filename);
  if( acppRun()){
    AC_ERROR("Parser terminated unsuccessfully.\n");
    error_flag =1;
    return EXIT_FAILURE;
  }
  acppUnload();

  /* Opening ISA File */
  if(isa_filename == NULL)
    AC_ERROR("No ISA file defined");

  if( !acppLoad(isa_filename)){
    AC_ERROR("Could not open ISA input file: %s", isa_filename);
    AC_ERROR("Parser terminated unsuccessfully.\n");
    return EXIT_FAILURE;
  }

  //Parsing ArchC ISA declaration file.
  AC_MSG("Parsing AC_ISA declaration file: %s\n", isa_filename);
  if( acppRun()){
    AC_ERROR("Parser terminated unsuccessfully.\n");
    error_flag =1;
  }
  acppUnload();

  if (error_flag)
    return EXIT_FAILURE;

  if( wordsize == 0){
    AC_MSG("Warning: No wordsize defined. Default value is 32 bits.\n");
    wordsize = 32;
  }

  if( fetchsize == 0){
    AC_MSG("Warning: No fetchsize defined. Default is to be equal to wordsize (%d).\n",
           wordsize);
    fetchsize = wordsize;
  }

  //Testing host endianness.
  a.i = 255;
  b.c[0] = 0;
  b.c[1] = 0;
  b.c[2] = 0;
  b.c[3] = 255;

  if( a.i == b.i )
    ac_host_endian = 1;    //Host machine is big endian
  else
    ac_host_endian = 0;   //Host machine is little endian

  ac_match_endian = (ac_host_endian == ac_tgt_endian);

  //If target is little endian, invert the order of fields in each format.
  //This is the way the little endian decoder expects format fields.
  if (ac_tgt_endian == 0)
    invert_fields(format_ins_list);

  //Creating decoder to get Field Number info and write its static version.
  //This object will be used by some Create functions.
  if( ACDDecoderFlag ){
    AC_MSG("Dumping decoder structures:\n");
    ShowDecInstr(instr_list);
    ShowDecFormat(format_ins_list);

    printf("\n\n");
  }
  decoder = CreateDecoder(format_ins_list, instr_list);

  if( ACDDecoderFlag )
    ShowDecoder(decoder -> decoder, 0);


  /*cache*/
  EnumerateCaches();
  GetFetchDevice();
  GetLoadDevice();
  GetFirstLevelDataDevice();

  //Creating Resources Header File
  CreateArchHeader();
  CreateArchRefHeader();

  //Creating Resources Impl File
  CreateArchImpl();
  CreateArchRefImpl();

  //Creating ISA Header File
  CreateISAHeader();

  //Now, declare stages if a pipeline was declared
  //Otherwise, declare one sc_module to simulate the processor datapath
  //OBS: For pipelined architectures there must be a stage_list or a pipe_list,
  //     but never both of them.

  if( stage_list )  //List of ac_stage declarations.
    AC_MSG("Warning: stage_list is no more used by acsim.\n");
  if( pipe_list )  //Pipeline list exist.
    AC_MSG("Warning: pipe_list is no more used by acsim.\n");

  //Creating Processor Files
  CreateProcessorHeader();
  CreateProcessorImpl();

  //Creating Formatted Registers Header and Implementation Files.
  if( HaveFormattedRegs )
    CreateRegsHeader();

  if (HaveTLMIntrPorts) {
    CreateIntrHeader();
    CreateIntrMacrosHeader();
    CreateIntrTmpl();
  }

/****/
  if (HaveTLM2IntrPorts) {
    CreateIntrTLM2Header();
    CreateIntrTLM2MacrosHeader();
    CreateIntrTLM2Tmpl();
  }


  //Creating Simulation Statistics class header file.
  if (ACStatsFlag) {
    CreateStatsHeaderTmpl();
    CreateStatsImplTmpl();
  }

  //Creating model syscall header file.
  if( ACABIFlag ) {
    CreateArchSyscallHeader();
    CreateArchSyscallTmpl();
  }

  /* Create the template for the .cpp instruction and format behavior file */
  CreateImplTmpl();

  /* Creating Parameters Header File */
  CreateParmHeader();

  /* Create the template for the main.cpp  file */
  CreateMainTmpl();

  /* Create the Makefile */
  CreateMakefile();

  //Issuing final messages to the user.
  AC_MSG("%s model files generated.\n", project_name);

  return 0;
}


////////////////////////////////////////////////////////////////////////////////////
// Create functions ...                                                           //
// These Functions are used to create the behavioral simulation files               //
// All of them use structures built by the parser.                                //
////////////////////////////////////////////////////////////////////////////////////

/*!Create ArchC Resources Header File */
void CreateArchHeader() {
    extern ac_sto_list *storage_list;
    extern char *project_name;
    extern char *upper_project_name;
    extern int HaveFormattedRegs, HaveMemHier, HaveTLMPorts, HaveTLMIntrPorts,
        HaveTLM2NBPorts, HaveTLM2Ports, HaveTLM2IntrPorts;

    ac_sto_list *pstorage;

    FILE *output;
    char filename[256];

    sprintf(filename, "%s_arch.H", project_name);

    if (!(output = fopen(filename, "w"))) {
        perror("ArchC could not open output file");
        exit(1);
    }

    print_comment(output, "ArchC Resources header file.");
    fprintf(output, "#ifndef  %s_ARCH_H\n", upper_project_name);
    fprintf(output, "#define  %s_ARCH_H\n\n", upper_project_name);

    fprintf(output, "#include  \"%s_parms.H\"\n", project_name);
    fprintf(output, "#include  \"ac_arch_dec_if.H\"\n");
    fprintf(output, "#include  \"ac_mem.H\"\n");
    fprintf(output, "#include  \"ac_memport.H\"\n");
    fprintf(output, "#include  \"ac_regbank.H\"\n");
    fprintf(output, "#include  \"ac_reg.H\"\n");

    if (HaveTLMPorts)
        fprintf(output, "#include  \"ac_tlm_port.H\"\n");

    if (HaveTLMIntrPorts)
        fprintf(output, "#include  \"ac_tlm_intr_port.H\"\n");

    if (HaveTLM2Ports)
        fprintf(output, "#include  \"ac_tlm2_port.H\"\n");

    if (HaveTLM2NBPorts)
        fprintf(output, "#include  \"ac_tlm2_nb_port.H\"\n");

    if (HaveTLM2IntrPorts)
        fprintf(output, "#include  \"ac_tlm2_intr_port.H\"\n");

    if (HaveFormattedRegs)
        fprintf(output, "#include  \"%s_fmt_regs.H\"\n", project_name);

    fprintf(output, " \n");

    if (ACGDBIntegrationFlag) {
        fprintf(output, "// AC_GDB template class forward declaration\n");
        fprintf(output, "template <typename ac_word> class AC_GDB;\n\n");
    }

    /*cache*/
    if (HaveMemHier) {
        fprintf(output, "#include \"ac_cache.H\"\n");
        fprintf(output, "#include \"ac_fifo_replacement_policy.H\"\n");
        fprintf(output, "#include \"ac_random_replacement_policy.H\"\n");
        fprintf(output, "#include \"ac_plrum_replacement_policy.H\"\n");
        fprintf(output, "#include \"ac_lru_replacement_policy.H\"\n");
        fprintf(output, "#include \"ac_cache_if.H\"\n");
    }

    // Declaring Architecture Resources class.
    COMMENT(INDENT[0],
            "ArchC class for model-specific architectural resources.\n");
    fprintf(output, "class %s_arch : public ac_arch_dec_if<%s_parms::ac_word, "
                    "%s_parms::ac_Hword> {\n",
            project_name, project_name, project_name);

    fprintf(output, "public:\n\n");

    /* Declaring Program Counter */
    COMMENT(INDENT[1], "Program Counter.");
    fprintf(output, "%sac_reg<unsigned> ac_pc;\n\n", INDENT[1]);
    /* Declaring Processor Id */
    COMMENT(INDENT[1], "Processor id.");
    fprintf(output, "%sac_reg<unsigned> ac_id;\n\n", INDENT[1]);

    /* Declaring storage devices */
    COMMENT(INDENT[1], "Storage Devices.");
    for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next) {
        switch (pstorage->type) {
        case REG:
            // Formatted registers have a special class.
            if (pstorage->format != NULL) {
                fprintf(output, "%s%s_fmt_%s %s;\n", INDENT[1], project_name,
                        pstorage->name, pstorage->name);
            } else {
                switch ((unsigned)(pstorage->width)) {
                case 0:
                    fprintf(output, "%sac_reg<%s_parms::ac_word> %s;\n",
                            INDENT[1], project_name, pstorage->name);
                    break;
                case 1:
                    fprintf(output, "%sac_reg<bool> %s;\n", INDENT[1],
                            pstorage->name);
                    break;
                case 8:
                    fprintf(output, "%sac_reg<unsigned char> %s;\n", INDENT[1],
                            pstorage->name);
                    break;
                case 16:
                    fprintf(output, "%sac_reg<unsigned short> %s;\n", INDENT[1],
                            pstorage->name);
                    break;
                case 32:
                    fprintf(output, "%sac_reg<unsigned long> %s;\n", INDENT[1],
                            pstorage->name);
                    break;
                case 64:
                    fprintf(output, "%sac_reg<unsigned long long> %s;\n",
                            INDENT[1], pstorage->name);
                    break;
                default:
                    AC_ERROR("Register width not supported: %d\n",
                             pstorage->width);
                    break;
                }
            }
            break;

        case REGBANK:
            // Emitting register bank. Checking is a register width was
            // declared.
            switch ((unsigned)(pstorage->width)) {
            case 0:
                fprintf(output, "%sac_regbank<%d, %s_parms::ac_word, "
                                "%s_parms::ac_Dword> %s;\n",
                        INDENT[1], pstorage->size, project_name, project_name,
                        pstorage->name);
                break;
            case 8:
                fprintf(output,
                        "%sac_regbank<%d, unsigned char, unsigned char> %s;\n",
                        INDENT[1], pstorage->size, pstorage->name);
                break;
            case 16:
                fprintf(output,
                        "%sac_regbank<%d, unsigned short, unsigned long> %s;\n",
                        INDENT[1], pstorage->size, pstorage->name);
                break;
            case 32:
                fprintf(
                    output,
                    "%sac_regbank<%d, unsigned long, unsigned long long> %s;\n",
                    INDENT[1], pstorage->size, pstorage->name);
                break;
            case 64:
                fprintf(
                    output,
                    "%sac_regbank<%d, unsigned long long, unsigned long> %s;\n",
                    INDENT[1], pstorage->size, pstorage->name);
                break;
            default:
                AC_ERROR("Register width not supported: %d\n", pstorage->width);
                break;
            }
            break;

        case CACHE:
        case ICACHE:
        case DCACHE:
            if (!HaveMemHier) { // It is a generic cache. Just emit a base
                                // container object.
                fprintf(output, "%sac_mem %s;\n", INDENT[1], pstorage->name);
                fprintf(output, "%sac_memport<%s_parms::ac_word, "
                                "%s_parms::ac_Hword> %s_mport;\n",
                        INDENT[1], project_name, project_name, pstorage->name);
            } else {
                // It is an ac_cache object.
                ac_cache_parms *p = pstorage->parms;
                if (p == NULL)
                    abort();

                fprintf(output, "%s%s %s;\n", INDENT[1],
                        pstorage->class_declaration, pstorage->name);

                fprintf(output, "%sac_cache_if<%s_parms::ac_word, "
                                "%s_parms::ac_Hword, %s >"
                                " %s_if;\n",
                        INDENT[1], project_name, project_name,
                        pstorage->class_declaration, pstorage->name);
                fprintf(output,
                        "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword> "
                        "%s_mport;\n",
                        INDENT[1], project_name, project_name, pstorage->name);
            }
            break;

        case MEM:
            fprintf(output, "%sac_mem %s;\n", INDENT[1], pstorage->name);
            fprintf(output, "%sac_memport<%s_parms::ac_word, "
                            "%s_parms::ac_Hword> %s_mport;\n",
                    INDENT[1], project_name, project_name, pstorage->name);
            break;

        case TLM_PORT:
            fprintf(output, "%sac_tlm_port %s;\n", INDENT[1], pstorage->name);
            fprintf(output, "%sac_memport<%s_parms::ac_word, "
                            "%s_parms::ac_Hword> %s_mport;\n",
                    INDENT[1], project_name, project_name, pstorage->name);
            break;

        case TLM2_PORT:
            fprintf(output, "%sac_tlm2_port %s;\n", INDENT[1], pstorage->name);
            fprintf(output, "%sac_memport<%s_parms::ac_word, "
                            "%s_parms::ac_Hword> %s_mport;\n",
                    INDENT[1], project_name, project_name, pstorage->name);
            break;

        case TLM2_NB_PORT:
            fprintf(output, "%sac_tlm2_nb_port %s;\n", INDENT[1],
                    pstorage->name);
            fprintf(output, "%sac_memport<%s_parms::ac_word, "
                            "%s_parms::ac_Hword> %s_mport;\n",
                    INDENT[1], project_name, project_name, pstorage->name);
            break;

        default:
            fprintf(output, "%sac_mem %s;\n", INDENT[1], pstorage->name);
            fprintf(output, "%sac_memport<%s_parms::ac_word, "
                            "%s_parms::ac_Hword> %s_mport;\n",
                    INDENT[1], project_name, project_name, pstorage->name);
            break;
        }
    }

    if (HaveTLMIntrPorts || HaveTLM2IntrPorts) {
        fprintf(output, "#define SLEEP_AWAKE_MODE\n");
        fprintf(output, "%sac_reg<%s_parms::ac_word> intr_reg;\n", INDENT[1],
                project_name);
    }

    fprintf(output, "\n\n");

    // ac_resources constructor declaration
    COMMENT(INDENT[1], "Constructor.");
    fprintf(output, "%sexplicit %s_arch();\n\n", INDENT[1], project_name);

    COMMENT(INDENT[1], "Module initialization method.");
    fprintf(output, "%svirtual void init(int ac, char* av[]) = 0;\n\n",
            INDENT[1]);

    COMMENT(INDENT[1], "Module finalization method.");
    fprintf(output, "%svirtual void stop(int status = 0) = 0;\n\n", INDENT[1]);

    if (ACGDBIntegrationFlag) {
        COMMENT(INDENT[1], "GDB stub access virtual method declaration.");
        fprintf(output,
                "%svirtual AC_GDB<%s_parms::ac_word>* get_gdbstub() = 0;\n\n",
                INDENT[1], project_name);
    }

    COMMENT(INDENT[1], "Virtual destructor declaration.");
    fprintf(output, "%svirtual ~%s_arch() {};\n\n", INDENT[1], project_name);

    fprintf(output, "%sstatic int globalId;\n", INDENT[1]);
    fprintf(output, "%sint getId() { return ac_id.read(); }\n", INDENT[1]);

    fprintf(output, "};\n\n"); // End of ac_resources class

    fprintf(output, "#endif  //_%s_ARCH_H\n", upper_project_name);
    fclose(output);
}

/*!Create ArchC Resources Reference Header File */
void CreateArchRefHeader() {
    extern ac_sto_list *storage_list;
    extern char* project_name;
    extern char* upper_project_name;
    extern int HaveFormattedRegs, HaveMemHier, HaveTLMIntrPorts, HaveTLM2IntrPorts;

    ac_sto_list *pstorage;

    FILE *output;
    char filename[256];

    sprintf(filename, "%s_arch_ref.H", project_name);

    if ( !(output = fopen( filename, "w"))){
        perror("ArchC could not open output file");
        exit(1);
    }

    print_comment( output, "ArchC Resources header file.");
    fprintf( output, "#ifndef  _%s_ARCH_REF_H\n", upper_project_name);
    fprintf( output, "#define  _%s_ARCH_REF_H\n\n", upper_project_name);

    fprintf( output, "#include  \"%s_parms.H\"\n", project_name);
    fprintf( output, "#include  \"ac_arch_ref.H\"\n");
    fprintf( output, "#include  \"ac_memport.H\"\n");
    fprintf( output, "#include  \"ac_reg.H\"\n");
    fprintf( output, "#include  \"ac_regbank.H\"\n");

    if (HaveTLMIntrPorts)
        fprintf(output, "#include  \"ac_tlm_intr_port.H\"\n");

    fprintf(output, "\n");

    if (HaveTLM2IntrPorts)
        fprintf(output, "#include  \"ac_tlm2_intr_port.H\"\n");

    fprintf(output, "\n");


    if (HaveMemHier) {
        fprintf(output, "#include \"ac_cache.H\"\n");
        fprintf(output, "#include \"ac_fifo_replacement_policy.H\"\n");
        fprintf(output, "#include \"ac_random_replacement_policy.H\"\n");
        fprintf(output, "#include \"ac_plrum_replacement_policy.H\"\n");
        fprintf(output, "#include \"ac_lru_replacement_policy.H\"\n");
    }



    COMMENT(INDENT[0], "Forward class declaration, needed to compile.");
    fprintf(output, "class %s_arch;\n\n", project_name);

    //Declaring Architecture Resource references class.
    COMMENT(INDENT[0],"ArchC class for model-specific architectural resources.");
    fprintf( output, "class %s_arch_ref : public ac_arch_ref<%s_parms::ac_word, %s_parms::ac_Hword> {\n", project_name, project_name, project_name);
    fprintf( output, "public:\n");
    fprintf( output, " \n");

    /* Declaring Program Counter */
    COMMENT(INDENT[1], "Program Counter.");
    fprintf(output, "%sac_reg<unsigned>& ac_pc;\n\n", INDENT[1]);

    /* Declaring Processor Id */
    COMMENT(INDENT[1], "Processor Id.");
    fprintf(output, "%sac_reg<unsigned>& ac_id;\n\n", INDENT[1]);

    /* Declaring storage devices */
    COMMENT(INDENT[1],"Storage Devices.");
    for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){
        switch( pstorage->type ) {
            case REG:
                //Formatted registers have a special class.
                if( pstorage->format != NULL ){
                    fprintf( output, "%s%s_fmt_%s& %s;\n", INDENT[1], project_name, pstorage->name, pstorage->name);
                }
                else{
                    switch((unsigned)(pstorage->width)) {
                        case 0:
                            fprintf( output, "%sac_reg<%s_parms::ac_word>& %s;\n",
                                    INDENT[1], project_name, pstorage->name);
                            break;
                        case 1:
                            fprintf( output, "%sac_reg<bool>& %s;\n",
                                    INDENT[1], pstorage->name);
                            break;
                        case 8:
                            fprintf( output, "%sac_reg<unsigned char>& %s;\n",
                                    INDENT[1], pstorage->name);
                            break;
                        case 16:
                            fprintf( output, "%sac_reg<unsigned short>& %s;\n",
                                    INDENT[1], pstorage->name);
                            break;
                        case 32:
                            fprintf( output, "%sac_reg<unsigned long>& %s;\n",
                                    INDENT[1], pstorage->name);
                            break;
                        case 64:
                            fprintf( output, "%sac_reg<unsigned long long>& %s;\n",
                                    INDENT[1], pstorage->name);
                            break;
                        default:
                            AC_ERROR("Register width not supported: %d\n",
                                    pstorage->width);
                            break;
                    }
                }
                break;

            case REGBANK:
                //Emitting register bank. Checking is a register width was declared.
                switch((unsigned)(pstorage->width)) {
                    case 0:
                        fprintf( output, "%sac_regbank<%d, %s_parms::ac_word, %s_parms::ac_Dword>& %s;\n",
                                INDENT[1], pstorage->size, project_name,
                                project_name, pstorage->name);
                        break;
                    case 8:
                        fprintf( output, "%sac_regbank<%d, unsigned char, unsigned char>& %s;\n",
                                INDENT[1], pstorage->size, pstorage->name);
                        break;
                    case 16:
                        fprintf( output, "%sac_regbank<%d, unsigned short, unsigned long>& %s;\n",
                                INDENT[1], pstorage->size, pstorage->name);
                        break;
                    case 32:
                        fprintf( output, "%sac_regbank<%d, unsigned long, unsigned long long>& %s;\n",
                                INDENT[1], pstorage->size, pstorage->name);
                        break;
                    case 64:
                        fprintf( output, "%sac_regbank<%d, unsigned long long, unsigned long long>& %s;\n",
                                INDENT[1], pstorage->size, pstorage->name);
                        break;
                    default:
                        AC_ERROR("Register width not supported: %d\n",
                                pstorage->width);
                        break;
                }
                break;

            case CACHE:
            case ICACHE:
            case DCACHE:
            case MEM:
                fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword>& %s;\n", INDENT[1], project_name, project_name, pstorage->name);
                break;

            default:
                fprintf( output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword>& %s;\n", INDENT[1], project_name, project_name, pstorage->name);
                break;
        }
    }

    if (HaveTLMIntrPorts || HaveTLM2IntrPorts)
        fprintf( output, "%sac_reg<%s_parms::ac_word>& intr_reg;\n",INDENT[1], project_name);

    fprintf(output, "\n");

    //ac_resources constructor declaration
    COMMENT(INDENT[1],"Constructor.");
    fprintf(output, "%s %s_arch_ref(%s_arch& arch);\n", INDENT[1], project_name, project_name);
    fprintf( output, "\n");
    fprintf( output, "};\n\n"); //End of _arch_ref class
    fprintf( output, "#endif  //_%s_ARCH_REF_H\n", upper_project_name);
    fclose( output);
}


/*!Create ArchC Resources Reference Implementation File */
void CreateArchRefImpl() {
    extern ac_sto_list *storage_list;
    extern char* project_name;
    extern int HaveFormattedRegs, HaveMemHier;

    ac_sto_list *pstorage;

    FILE *output;
    char filename[256];

    sprintf(filename, "%s_arch_ref.cpp", project_name);

    if ( !(output = fopen( filename, "w"))){
        perror("ArchC could not open output file");
        exit(1);
    }

    print_comment( output, "ArchC Resources implementation file.");

    fprintf( output, "#include  \"%s_arch.H\"\n", project_name);
    fprintf( output, "#include  \"%s_arch_ref.H\"\n\n", project_name);

    //Declaring Architecture Resource references class.
    COMMENT(INDENT[0],"/Default constructor.");
    fprintf(output, "%s_arch_ref::%s_arch_ref(%s_arch& arch) : ac_arch_ref<%s_parms::ac_word, %s_parms::ac_Hword>(arch),\n",
            project_name, project_name, project_name, project_name, project_name);

    fprintf(output, "%sac_pc(arch.ac_pc), ac_id(arch.ac_id),\n", INDENT[1]);

    /* Declaring storage devices */
    for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next) {
        if ( pstorage->has_memport )
            fprintf(output, "%s%s(arch.%s_mport)", INDENT[1], pstorage->name, pstorage->name);
        else
            fprintf(output, "%s%s(arch.%s)", INDENT[1], pstorage->name, pstorage->name);

        if (pstorage->next != NULL) {
            fprintf(output, ", ");
        }
    }

    if (HaveTLMIntrPorts || HaveTLM2IntrPorts)
        fprintf(output, ", intr_reg(arch.intr_reg) ");

    fprintf(output, " {}\n\n");
    fclose( output);

}


//!Creates Decoder Header File
void CreateParmHeader() {
  extern ac_dec_format *format_ins_list;
  extern int instr_num;
  extern int declist_num;
  extern int format_num, largest_format_size;
  extern int wordsize, fetchsize, HaveMemHier, HaveCycleRange;
  extern ac_sto_list* load_device;
  extern ac_decoder_full *decoder;
  extern char* project_name;
  extern char* upper_project_name;

  char filename[256];

  //! File containing decoding structures
  FILE *output;

  sprintf(filename, "%s_parms.H", project_name);
  if ( !(output = fopen(filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, "ArchC Parameters header file.");
  fprintf( output, "#ifndef  _%s_PARMS_H\n", upper_project_name);
  fprintf( output, "#define  _%s_PARMS_H\n\n", upper_project_name);

  /* options defines */
  if( ACVerboseFlag ) {
    fprintf( output, "#define  AC_UPDATE_LOG \t //!< Update log generation turned on.\n");
    fprintf( output, "#define  AC_VERBOSE \t //!< Indicates Verbose mode. Where update logs are dumped on screen.\n");
  }

  if( ACDebugFlag )
    fprintf( output, "#define  AC_DEBUG \t //!< Indicates that debug option is turned on.\n\n");

  if( ACDelayFlag )
    fprintf( output, "#define  AC_DELAY \t //!< Indicates that delay option is turned on.\n\n");

  if( ACStatsFlag )
    fprintf( output, "#define  AC_STATS \t //!< Indicates that statistics collection is turned on.\n");

  if( HaveMemHier )
    fprintf( output, "#define  AC_MEM_HIERARCHY \t //!< Indicates that a memory hierarchy was declared.\n\n");

  if( HaveCycleRange )
    fprintf( output, "#define  AC_CYCLE_RANGE \t //!< Indicates that cycle range for instructions were declared.\n\n");

  if( ACLongJmpStop || ACThreading )
    fprintf( output, "#define  AC_ACTION_STOP 2\t //!< Indicates action value to stop used by longjmp.\n\n");

  /* parms namespace definition */
  fprintf(output, "namespace %s_parms {\n\n", project_name);

  fprintf( output, "\nstatic const unsigned int AC_DEC_FIELD_NUMBER = %d; \t //!< Number of Fields used by decoder.\n",
           decoder->nFields);
  fprintf( output, "static const unsigned int AC_DEC_INSTR_NUMBER = %d; \t //!< Number of Instructions declared.\n",
           instr_num);
  fprintf( output, "static const unsigned int AC_DEC_FORMAT_NUMBER = %d; \t //!< Number of Formats declared.\n",
           format_num);
  fprintf( output, "static const unsigned int AC_DEC_LIST_NUMBER = %d; \t //!< Number of decodification lists used by decoder.\n",
           declist_num);
  fprintf( output, "static const unsigned int AC_MAX_BUFFER = %d; \t //!< This is the size needed by decoder buffer. It is equal to the biggest instruction size.\n",
           largest_format_size/8);
  fprintf( output, "static const unsigned int AC_WORDSIZE = %d; \t //!< Architecture wordsize in bits.\n",
           wordsize);
  fprintf( output, "static const unsigned int AC_FETCHSIZE = %d; \t //!< Architecture fetchsize in bits.\n",
           fetchsize);
  fprintf( output, "static const unsigned int AC_MATCH_ENDIAN = %d; \t //!< If the simulated arch match the endian with host.\n",
           ac_match_endian);
  fprintf( output, "static const unsigned int AC_PROC_ENDIAN = %d; \t //!< The simulated arch is big endian?\n",
           ac_tgt_endian);
  fprintf( output, "static const unsigned int AC_RAMSIZE = %uU; \t //!< Architecture RAM size in bytes (storage %s).\n",
           load_device->size, load_device->name);
  fprintf( output, "static const unsigned int AC_RAM_END = %uU; \t //!< Architecture end of RAM (storage %s).\n",
           load_device->size, load_device->name);

  fprintf( output, "\n\n");
  COMMENT(INDENT[0],"Word type definitions.");

  //Emitting ArchC word types.
  switch( wordsize ) {
    case 8:
      fprintf( output, "typedef  unsigned char ac_word; \t //!< Unsigned word.\n");
      fprintf( output, "typedef  unsigned char ac_Uword; \t //!< Unsigned word.\n");
      fprintf( output, "typedef  char ac_Sword; \t //!< Signed word.\n");
      fprintf( output, "typedef  unsigned char ac_Hword; \t //!< Signed half word.\n");
      fprintf( output, "typedef  unsigned char ac_UHword; \t //!< Unsigned half word.\n");
      fprintf( output, "typedef  char ac_SHword; \t //!< Signed half word.\n");
      fprintf( output, "typedef  unsigned short ac_Dword; \t //!< Signed double word.\n");
      fprintf( output, "typedef  unsigned short ac_UDword; \t //!< Unsigned double word.\n");
      fprintf( output, "typedef  short ac_SDword; \t //!< Signed double word.\n");
      break;
    case 16:
      fprintf( output, "typedef  unsigned short int ac_word; \t //!< Unsigned word.\n");
      fprintf( output, "typedef  unsigned short int ac_Uword; \t //!< Unsigned word.\n");
      fprintf( output, "typedef  short int ac_Sword; \t //!< Signed word.\n");
      fprintf( output, "typedef  unsigned char ac_Hword; \t //!< Signed half word.\n");
      fprintf( output, "typedef  unsigned char ac_UHword; \t //!< Unsigned half word.\n");
      fprintf( output, "typedef  char ac_SHword; \t //!< Signed half word.\n");
      fprintf( output, "typedef  unsigned int ac_Dword; \t //!< Signed double word.\n");
      fprintf( output, "typedef  unsigned int ac_UDword; \t //!< Unsigned double word.\n");
      fprintf( output, "typedef  int ac_SDword; \t //!< Signed double word.\n");
      break;
    case 32:
      fprintf( output, "typedef  unsigned int ac_word; \t //!< Unsigned word.\n");
      fprintf( output, "typedef  unsigned int ac_Uword; \t //!< Unsigned word.\n");
      fprintf( output, "typedef  int ac_Sword; \t //!< Signed word.\n");
      fprintf( output, "typedef  short int ac_SHword; \t //!< Signed half word.\n");
      fprintf( output, "typedef  unsigned short int  ac_UHword; \t //!< Unsigned half word.\n");
      fprintf( output, "typedef  unsigned short int  ac_Hword; \t //!< Unsigned half word.\n");
      fprintf( output, "typedef  unsigned long long ac_Dword; \t //!< Signed double word.\n");
      fprintf( output, "typedef  unsigned long long ac_UDword; \t //!< Unsigned double word.\n");
      fprintf( output, "typedef  long long ac_SDword; \t //!< Signed double word.\n");
      break;
    case 64:
      fprintf( output, "typedef  unsigned long long ac_word; \t //!< Unsigned word.\n");
      fprintf( output, "typedef  unsigned long long ac_Uword; \t //!< Unsigned word.\n");
      fprintf( output, "typedef  long long ac_Sword; \t //!< Signed word.\n");
      fprintf( output, "typedef  int ac_SHword; \t //!< Signed half word.\n");
      fprintf( output, "typedef  unsigned int  ac_UHword; \t //!< Unsigned half word.\n");
      fprintf( output, "typedef  unsigned int  ac_UHword; \t //!< Unsigned half word.\n");
      break;
    default:
      AC_ERROR("Wordsize not supported: %d\n", wordsize);
      break;
  }

  fprintf( output, "typedef  char ac_byte; \t //!< Signed byte word.\n");
  fprintf( output, "typedef  unsigned char ac_Ubyte; \t //!< Unsigned byte word.\n");

  fprintf( output, "\n\n");
  COMMENT(INDENT[0],"Fetch type definition.");

  switch( fetchsize ){
    case 8:
      fprintf( output, "typedef  unsigned char ac_fetch; \t //!< Unsigned word.\n");
      break;
    case 16:
      fprintf( output, "typedef  unsigned short int ac_fetch; \t //!< Unsigned word.\n");
      break;
    case 32:
      fprintf( output, "typedef  unsigned int ac_fetch; \t //!< Unsigned word.\n");
      break;
    case 64:
      fprintf( output, "typedef  unsigned long long ac_fetch; \t //!< Unsigned word.\n");
      break;
    default:
      AC_ERROR("Fetchsize not supported: %d\n", fetchsize);
      break;
  }
  fprintf( output, "\n\n");

  //This enum type is used for case identification inside the ac_behavior methods
  fprintf( output, "enum ac_stage_list {");
  fprintf( output, "ST0");

  //Closing enum declaration
  fprintf( output, "};\n\n");

  /* closing namespace declaration */
  fprintf( output, "}\n\n");

  //Create a compiler error if delay assignment is used without the -dy option
  COMMENT(INDENT[0],"Create a compiler error if delay assignment is used without the -dy option");
  fprintf( output, "#ifndef AC_DELAY\n");
  fprintf( output, "extern %s_parms::ac_word ArchC_ERROR___PLEASE_USE_OPTION_DELAY_WHEN_CREATING_SIMULATOR___;\n",
           project_name);
  fprintf( output, "#define delay(a,b) ArchC_ERROR___PLEASE_USE_OPTION_DELAY_WHEN_CREATING_SIMULATOR___\n");
  fprintf( output, "#endif\n\n\n");
  fprintf( output, "#endif  //_%s_PARMS_H\n", upper_project_name);
  fclose(output);
}


//!Creates the ISA Header File.
void CreateISAHeader() {
  extern ac_grp_list* group_list;
  extern ac_dec_format *format_ins_list;
  extern char *project_name;
  extern char *upper_project_name;
  extern char* helper_contents;
  extern ac_dec_instr *instr_list;
  extern int wordsize;
  extern ac_dec_field *common_instr_field_list;
  ac_grp_list* pgroup;
  ac_dec_format *pformat;
  ac_dec_instr *pinstr;
  ac_dec_field *pfield;

  char filename[256];
  char description[] = "Instruction Set Architecture header file.";

  // File containing ISA declaration
  FILE  *output;

  sprintf( filename, "%s_isa.H", project_name);

  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, description);
  fprintf( output, "#ifndef _%s_ISA_H\n", upper_project_name);
  fprintf( output, "#define _%s_ISA_H\n\n", upper_project_name);
  fprintf( output, "#include \"%s_parms.H\"\n", project_name);
  fprintf( output, "#include \"ac_instr.H\"\n");
  fprintf( output, "#include \"ac_decoder_rt.H\"\n");
  fprintf( output, "#include \"ac_instr_info.H\"\n");
  fprintf( output, "#include \"%s_arch.H\"\n", project_name);
  fprintf( output, "#include \"%s_arch_ref.H\"\n", project_name);
  if (ACABIFlag)
    fprintf( output, "#include \"%s_syscall.H\"\n", project_name);
  if (ACStatsFlag)
    fprintf(output, "#include \"%s_stats.H\"\n", project_name);
  fprintf( output, "\n");


  fprintf(output, "namespace %s_parms\n{\n", project_name);
  fprintf(output, "class %s_isa: public %s_arch_ref", project_name,
          project_name);
  if (ACStatsFlag)
    fprintf(output, ", public %s_all_stats", project_name);
  fprintf(output, " {\n");

  fprintf(output, "private:\n");
  fprintf(output, "typedef ac_instr<AC_DEC_FIELD_NUMBER> ac_instr_t;\n");
  for (pgroup = group_list; pgroup != NULL; pgroup = pgroup->next)
    fprintf(output, "%sstatic bool group_%s[AC_DEC_INSTR_NUMBER];\n",
            INDENT[1], pgroup->name);
  fprintf(output, "\n");

  fprintf(output, "public:\n");
  fprintf(output, "%sstatic ac_dec_field fields[AC_DEC_FIELD_NUMBER];\n",
          INDENT[1]);
  fprintf(output, "%sstatic ac_dec_format formats[AC_DEC_FORMAT_NUMBER];\n",
          INDENT[1]);
  fprintf(output, "%sstatic ac_dec_list dec_list[AC_DEC_LIST_NUMBER];\n",
          INDENT[1]);
  fprintf(output, "%sstatic ac_dec_instr instructions[AC_DEC_INSTR_NUMBER];\n",
          INDENT[1]);
  fprintf(output, "%sstatic const ac_instr_info instr_table[AC_DEC_INSTR_NUMBER + 1];\n\n",
          INDENT[1]);
  fprintf(output, "%sstatic const unsigned instr_format_table[AC_DEC_INSTR_NUMBER + 1];\n\n",
          INDENT[1]);

  fprintf( output, "%sac_decoder_full* decoder;\n\n", INDENT[1]);
  if (ACABIFlag)
    fprintf( output, "%s%s_syscall syscall;\n", INDENT[1], project_name );

  /* current instruction ID */
  if ( ACCurInstrID )
    fprintf(output, "%sint cur_instr_id;\n\n", INDENT[1]);

  /* ac_helper */
  if (helper_contents)
  {
    fprintf(output, "%s", helper_contents);
    fprintf(output, "\n");
  }

  //Emitting Constructor.
  COMMENT(INDENT[1], "Constructor.");
  fprintf( output,"%s%s_isa(%s_arch& ref) : %s_arch_ref(ref) ", INDENT[1],
            project_name, project_name, project_name);
  if (ACABIFlag)
    fprintf(output, ", syscall(ref)");
  fprintf( output," {\n");

  COMMENT(INDENT[2], "Building Decoder.");
  fprintf( output,"%sdecoder = ac_decoder_full::CreateDecoder(%s_isa::formats, %s_isa::instructions, &ref);\n",
           INDENT[2], project_name, project_name );

  /* Closing constructor declaration. */
  fprintf( output,"%s}\n\n", INDENT[1] );

  if ( ACCurInstrID ) {
    /* getter methods for current instruction */
    fprintf(output, "%sinline const char* get_name() { return instr_table[cur_instr_id].ac_instr_name; }\n",
            INDENT[1]);
    fprintf(output, "%sinline const char* get_mnemonic() { return instr_table[cur_instr_id].ac_instr_mnemonic; }\n",
            INDENT[1]);
    fprintf(output, "%sinline unsigned get_size() { return instr_table[cur_instr_id].ac_instr_size; };\n",
            INDENT[1]);
    fprintf(output, "%sinline unsigned get_cycles() { return instr_table[cur_instr_id].ac_instr_cycles; };\n",
            INDENT[1]);
    fprintf(output, "%sinline unsigned get_min_latency() { return instr_table[cur_instr_id].ac_instr_min_latency; };\n",
            INDENT[1]);
    fprintf(output, "%sinline unsigned get_max_latency() { return instr_table[cur_instr_id].ac_instr_max_latency; };\n\n",
            INDENT[1]);

    // Group query methods.
    for (pgroup = group_list; pgroup != NULL; pgroup = pgroup->next)
    {
      fprintf(output, "%sinline const bool belongs_to_%s()\n%s{\n",
              INDENT[2], pgroup->name, INDENT[2]);
      fprintf(output, "%sreturn group_%s[cur_instr_id];\n%s}\n",
              INDENT[3], pgroup->name, INDENT[2]);
      fprintf(output, "\n");
    }
  }

  //Turn-on or off Forced Inline in Interpretation Routines
  char finline[64] = "";
  if (ACForcedInline)
    strcpy(finline, "inline __attribute__((always_inline)) ");

  /* Instruction Behavior Method declarations */
  /* instruction */
  fprintf(output, "%s%svoid _behavior_instruction(", INDENT[1], finline);

  /* common_instr_field_list has the list of fields for the generic instruction. */
  for( pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next){
    if (!pfield->sign) fprintf(output, "u");

    if (pfield->size < 9) fprintf(output, "int8_t");
    else if (pfield->size < 17) fprintf(output, "int16_t");
    else if (pfield->size < 33) fprintf(output, "int32_t");
    else fprintf(output, "int64_t");

    fprintf(output, " %s", pfield->name);

    if (pfield->next != NULL)
      fprintf(output, ", ");
  }
  fprintf(output, ");\n\n");

  /* begin & end */
  fprintf(output, "%s%svoid _behavior_begin();\n", INDENT[1], finline);
  fprintf(output, "%s%svoid _behavior_end();\n\n", INDENT[1], finline);

  /* types/formats */
  for (pformat = format_ins_list; pformat!= NULL; pformat=pformat->next) {
    fprintf(output, "%s%svoid _behavior_%s_%s(", INDENT[1], finline,
            project_name, pformat->name);
    for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
      if (!pfield->sign) fprintf(output, "u");

      if (pfield->size < 9) fprintf(output, "int8_t");
      else if (pfield->size < 17) fprintf(output, "int16_t");
      else if (pfield->size < 33) fprintf(output, "int32_t");
      else fprintf(output, "int64_t");

      fprintf(output, " %s", pfield->name);

      if (pfield->next != NULL)
        fprintf(output, ", ");
    }
    fprintf(output, ");\n");
  }
  fprintf(output, "\n");

  /* instructions */
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    for (pformat = format_ins_list;
          (pformat != NULL) && strcmp(pinstr->format, pformat->name);
          pformat = pformat->next);
    fprintf(output, "%s%svoid behavior_%s(", INDENT[1], finline, pinstr->name);
    for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
      if (!pfield->sign) fprintf(output, "u");

      if (pfield->size < 9) fprintf(output, "int8_t");
      else if (pfield->size < 17) fprintf(output, "int16_t");
      else if (pfield->size < 33) fprintf(output, "int32_t");
      else fprintf(output, "int64_t");

      fprintf(output, " %s", pfield->name);

      if (pfield->next != NULL)
        fprintf(output, ", ");
    }
    fprintf(output, ");\n");
  }
  fprintf(output, "\n");

  /* Closing class declaration. */
  fprintf(output,"};\n");

  /* Closing namespace declaration. */
  fprintf(output,"};\n\n");

  /* END OF FILE */
  fprintf( output, "\n\n#endif //_%s_ISA_H\n\n", upper_project_name);
  fclose( output);

  /* opens behavior macros file */
  sprintf( filename, "%s_bhv_macros.H", project_name);
  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  fprintf( output, "#ifndef _%s_BHV_MACROS_H\n", upper_project_name);
  fprintf( output, "#define _%s_BHV_MACROS_H\n\n", upper_project_name);

  /* ac_memory TYPEDEF */
  fprintf(output, "typedef ac_memport<%s_parms::ac_word, %s_parms::ac_Hword> ac_memory;\n\n",
          project_name, project_name);

  /* ac_behavior main macro */
  fprintf( output, "#define ac_behavior(instr) AC_BEHAVIOR_##instr ()\n\n");

  /* ac_behavior 2nd level macros - generic instruction */
  fprintf(output, "#define AC_BEHAVIOR_instruction() %s_parms::%s_isa::_behavior_instruction(",
          project_name, project_name);

  /* common_instr_field_list has the list of fields for the generic instruction. */
  for( pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next){
    if (!pfield->sign) fprintf(output, "u");

    if (pfield->size < 9) fprintf(output, "int8_t");
    else if (pfield->size < 17) fprintf(output, "int16_t");
    else if (pfield->size < 33) fprintf(output, "int32_t");
    else fprintf(output, "int64_t");

    fprintf(output, " %s", pfield->name);

    if (pfield->next != NULL)
      fprintf(output, ", ");
  }
  fprintf(output, ")\n\n");

  /* ac_behavior 2nd level macros - pseudo-instructions begin, end */
  fprintf(output, "#define AC_BEHAVIOR_begin() %s_parms::%s_isa::_behavior_begin()\n",
          project_name, project_name);
  fprintf(output, "#define AC_BEHAVIOR_end() %s_parms::%s_isa::_behavior_end()\n",
          project_name, project_name);

  fprintf(output, "\n");

  /* ac_behavior 2nd level macros - instruction types */
  for( pformat = format_ins_list; pformat!= NULL; pformat=pformat->next) {
    fprintf(output, "#define AC_BEHAVIOR_%s() %s_parms::%s_isa::_behavior_%s_%s(",
            pformat->name, project_name, project_name,
            project_name, pformat->name);
    for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
      if (!pfield->sign) fprintf(output, "u");

      if (pfield->size < 9) fprintf(output, "int8_t");
      else if (pfield->size < 17) fprintf(output, "int16_t");
      else if (pfield->size < 33) fprintf(output, "int32_t");
      else fprintf(output, "int64_t");

      fprintf(output, " %s", pfield->name);

      if (pfield->next != NULL)
        fprintf(output, ", ");
    }
    fprintf(output, ")\n");
  }
  fprintf(output, "\n");

  /* ac_behavior 2nd level macros - instructions */
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    fprintf(output, "#define AC_BEHAVIOR_%s() %s_parms::%s_isa::behavior_%s(",
            pinstr->name, project_name, project_name, pinstr->name);
    for (pformat = format_ins_list;
          (pformat != NULL) && strcmp(pinstr->format, pformat->name);
          pformat = pformat->next);
    for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
      if (!pfield->sign) fprintf(output, "u");

      if (pfield->size < 9) fprintf(output, "int8_t");
      else if (pfield->size < 17) fprintf(output, "int16_t");
      else if (pfield->size < 33) fprintf(output, "int32_t");
      else fprintf(output, "int64_t");

      fprintf(output, " %s", pfield->name);

      if (pfield->next != NULL)
        fprintf(output, ", ");
    }
    fprintf(output, ")\n");
  }

  /* END OF FILE */
  fprintf( output, "\n\n#endif //_%s_BHV_MACROS_H\n\n", upper_project_name);
  fclose( output);
}


//!Creates Processor Module Header File
void CreateProcessorHeader() {
  extern char *project_name;
  extern char *upper_project_name;
  extern int HaveTLMIntrPorts, largest_format_size;

  extern int HaveTLM2IntrPorts;
  extern ac_sto_list *tlm2_intr_port_list;

  extern ac_sto_list *tlm_intr_port_list;
  ac_sto_list *pport;
  extern ac_dec_instr *instr_list;
  char filename[256];
  char description[] = "Architecture Module header file.";

  // File containing ISA declaration
  FILE  *output;

  sprintf( filename, "%s.H", project_name);

  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, description);

  fprintf( output, "#ifndef  _%s_H\n", upper_project_name);
  fprintf( output, "#define  _%s_H\n\n", upper_project_name);

  fprintf( output, "#include \"%s_parms.H\"\n", project_name);
  fprintf( output, "#include \"systemc.h\"\n");
  fprintf( output, "#include \"ac_module.H\"\n");
  fprintf( output, "#include \"ac_utils.H\"\n");
#ifdef HLT_SUPPORT
  fprintf( output, "#include \"ac_hltrace.H\"\n");
#endif
  fprintf( output, "#include \"%s_arch.H\"\n", project_name);
  fprintf( output, "#include \"%s_isa.H\"\n", project_name);

  // POWER ESTIMATION SUPPORT

  if (ACPowerEnable) {
    fprintf( output, "#ifdef POWER_SIM\n");
    fprintf( output, "#include \"arch_power_stats.H\"\n");
    fprintf( output, "#endif\n");
  }


  if (ACABIFlag)
    fprintf( output, "#include \"%s_syscall.H\"\n", project_name);

  if (HaveTLMIntrPorts) {
    fprintf(output, "#include \"ac_tlm_intr_port.H\"\n");
    fprintf(output, "#include \"%s_intr_handlers.H\"\n", project_name);
  }

if (HaveTLM2IntrPorts) {
         fprintf(output, "#include \"ac_tlm2_intr_port.H\"\n");
         fprintf(output, "#include \"%s_intr_handlers.H\"\n", project_name);
       }

  if(ACGDBIntegrationFlag) {
    fprintf( output, "#include \"ac_gdb_interface.H\"\n");
    fprintf( output, "#include \"ac_gdb.H\"\n");
  }

  fprintf(output, "\n\nclass %s: public ac_module, public %s_arch",
          project_name, project_name);
  if (ACGDBIntegrationFlag)
    fprintf(output, ", public AC_GDB_Interface<%s_parms::ac_word>",
            project_name);
  fprintf(output, " {\n");

  fprintf(output, "private:\n");
  if( ACDecCacheFlag ) {
    EmitDecCache(output, 1);
  }
  if( ACWaitFlag ) {
    for(int temp=1; temp<=5; temp++) {
      for (ac_dec_instr *pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
        if (pinstr->cycles == temp) {
          fprintf(output, "%ssc_time time_%dcycle;\n", INDENT[1], temp);
          break;
        }
      }
    }
    fprintf( output, "\n");
  }

  fprintf( output, "public:\n\n");

  // POWER ESTIMATION SUPPORT
  if (ACPowerEnable) {

    fprintf( output, "#ifdef POWER_SIM\n");
    fprintf( output, "power_stats ps;\n");
    fprintf( output, "#endif\n");
  }


  if (ACVerboseFlag)
    fprintf( output, "%ssc_signal<bool> done;\n\n", INDENT[1]);

  fprintf( output, "%sbool has_delayed_load;\n", INDENT[1]);
  fprintf( output, "%schar* delayed_load_program;\n", INDENT[1]);
  fprintf( output, "%s%s_parms::%s_isa ISA;\n",
           INDENT[1], project_name, project_name);

  if (HaveTLMIntrPorts) {
    for (pport = tlm_intr_port_list; pport != NULL; pport = pport->next) {
      fprintf(output, "%s%s_%s_handler %s_hnd;\n",
              INDENT[1], project_name, pport->name, pport->name);
      fprintf(output, "%sac_tlm_intr_port %s;\n\n",
              INDENT[1], pport->name);
    }
  }

 /******/
        if (HaveTLM2IntrPorts) {
          for (pport = tlm2_intr_port_list; pport != NULL; pport = pport->next) {
      fprintf(output, "%s%s_%s_handler %s_hnd;\n", INDENT[1], project_name, pport->name, pport->name);
      fprintf(output, "%sac_tlm2_intr_port %s;\n\n", INDENT[1], pport->name);
          }
        }



  if (ACThreading) {
    COMMENT(INDENT[1], "Address of Interpretation Routines.");
    fprintf( output, "%svoid** IntRoutine;\n\n", INDENT[1]);
  }

  if(ACDecCacheFlag){
    fprintf( output, "%sDecCacheItem* DEC_CACHE;\n", INDENT[1]);
    fprintf( output, "%sDecCacheItem* instr_dec;\n", INDENT[1]);
  }
  else
    fprintf( output, "%sunsigned* ins_cache;\n", INDENT[1]);

  //fprintf( output, "%sunsigned id;\n", INDENT[1]);
  fprintf( output, "%sbool start_up;\n", INDENT[1]);

  if (ACGDBIntegrationFlag) {
    fprintf(output, "%sAC_GDB<%s_parms::ac_word>* gdbstub;\n",
            INDENT[1], project_name);
  }

  fprintf( output, "\n");

  if (ACThreading) {
    COMMENT(INDENT[1], "Dispatch Method.");
    fprintf( output,
             "%sinline __attribute__((always_inline)) void* dispatch();\n\n",
             INDENT[1]);
  }

  COMMENT(INDENT[1], "Behavior execution method.");
  fprintf( output, "%svoid behavior();\n\n", INDENT[1]);

  if (ACVerboseFlag) {
    COMMENT(INDENT[1], "Verification method.");
    fprintf( output, "%svoid ac_verify();\n\n", INDENT[1]);
  }

  fprintf( output, "%sSC_HAS_PROCESS( %s );\n\n", INDENT[1], project_name);

  fprintf( output, "%ssc_event wake;\n\n", INDENT[1]);


  //!Declaring ARCH Constructor.
  COMMENT(INDENT[1], "Constructor.");

  // POWER ESTIMATION SUPPORT

  if (ACPowerEnable) {
    fprintf( output, "\n#ifdef POWER_SIM\n");
    fprintf( output, "%s%s( sc_module_name name_ ): ac_module(name_), %s_arch(), ISA(*this), ps((const char*)name_)",
           INDENT[1], project_name, project_name);
    fprintf( output, "\n#else\n");
    fprintf( output, "%s%s( sc_module_name name_ ): ac_module(name_), %s_arch(), ISA(*this)",
            INDENT[1], project_name, project_name);
    fprintf( output, "\n#endif\n");
  }

  else
  {
    fprintf( output, "%s%s( sc_module_name name_ ): ac_module(name_), %s_arch(), ISA(*this)",
            INDENT[1], project_name, project_name);

  }



  if (HaveTLMIntrPorts) {
    for (pport = tlm_intr_port_list; pport != NULL; pport = pport->next) {
      fprintf(output, ", %s_hnd(*this,&wake)", pport->name);
      fprintf(output, ", %s(\"%s\", %s_hnd)",
              pport->name, pport->name, pport->name);
    }
  }



   if (HaveTLM2IntrPorts) {
       for (pport = tlm2_intr_port_list; pport != NULL; pport = pport->next) {
    fprintf(output, ", %s_hnd(*this,&wake)", pport->name);
    fprintf(output, ", %s(\"%s\", %s_hnd)", pport->name, pport->name, pport->name);
         }
       }


  fprintf(output, " {\n");

  fprintf( output, "%sSC_THREAD( behavior );\n", INDENT[2]);
  fprintf( output, "%ssensitive << wake;\n", INDENT[2]);

  if (ACVerboseFlag) {
    fprintf( output, "%sSC_THREAD( ac_verify );\n", INDENT[2]);
    fprintf( output, "%ssensitive<< done;\n\n", INDENT[2]);
  }

  fprintf( output,"%shas_delayed_load = false; \n", INDENT[2]);

  fprintf( output, "%sstart_up=1;\n", INDENT[2]);
  fprintf( output, "%sac_id.write(globalId++);\n", INDENT[2]);


  if (ACWaitFlag)
    fprintf(output, "%sset_proc_freq(1000/module_period_ns);\n", INDENT[2]);

  fprintf( output, "%s}\n\n", INDENT[1]);  //end constructor

  if(ACDecCacheFlag) {
    fprintf( output, "%svoid init_dec_cache() {\n", INDENT[1]);
    fprintf( output, "%sDEC_CACHE = (DecCacheItem*) calloc(sizeof(DecCacheItem), (dec_cache_size",
             INDENT[2]);
    if( ACIndexFix ) fprintf( output, " / %d", largest_format_size / 8);
    fprintf( output, "));\n");
    fprintf( output, "%s}\n\n", INDENT[1]);  //end init_dec_cache
  }

  if(ACGDBIntegrationFlag) {
    fprintf( output, "%s/***********\n", INDENT[1]);
    fprintf( output, "%s * GDB Support - user supplied methods\n", INDENT[1]);
    fprintf( output, "%s * For further information, look at ~/src/aclib/ac_gdb/ac_gdb_interface.H\n",
             INDENT[1]);
    fprintf( output, "%s ***********/\n\n", INDENT[1]);

    fprintf( output, "%s/* Processor Feature Support */\n", INDENT[1]);
    fprintf( output, "%sbool get_ac_tgt_endian();\n\n", INDENT[1]);
    fprintf( output, "%svoid ac_stop();\n\n", INDENT[1]);

    fprintf( output, "%s/* Register access */\n", INDENT[1]);
    fprintf( output, "%sint nRegs(void);\n", INDENT[1]);
    fprintf( output, "%s%s_parms::ac_word reg_read(int reg);\n",
             INDENT[1], project_name);
    fprintf( output, "%svoid reg_write( int reg, %s_parms::ac_word value );\n",
             INDENT[1], project_name);

    fprintf( output, "%s/* Memory access */\n", INDENT[1]);
    fprintf( output, "%sunsigned char mem_read( unsigned int address );\n",
             INDENT[1]);
    fprintf( output, "%svoid mem_write( unsigned int address, unsigned char byte );\n",
             INDENT[1]);

    fprintf( output, "%s/* GDB stub access */\n", INDENT[1]);
    fprintf( output, "%sAC_GDB<%s_parms::ac_word>* get_gdbstub();\n",
             INDENT[1], project_name);
  }

  if (ACWaitFlag) {
    fprintf( output, "%svoid set_proc_freq(unsigned int proc_freq);\n\n", INDENT[1]);
  }

  fprintf( output, "%sunsigned get_ac_pc();\n\n", INDENT[1]);
  fprintf( output, "%svoid set_ac_pc( unsigned int value );\n\n", INDENT[1]);
  fprintf( output, "%svirtual void PrintStat();\n\n", INDENT[1]);
  fprintf( output, "%svoid init(int ac, char* av[]);\n\n", INDENT[1]);
  fprintf( output, "%svoid init();\n\n", INDENT[1]);
  fprintf( output, "%svoid set_prog_args();\n\n", INDENT[1]);
  fprintf( output, "%svoid load(char* program);\n\n", INDENT[1]);
  fprintf( output, "%svoid delayed_load(char* program);\n\n", INDENT[1]);
  fprintf( output, "%svoid stop(int status = 0);\n\n", INDENT[1]);

  if (ACGDBIntegrationFlag)
    fprintf(output, "%svoid enable_gdb(int port = 0);\n\n", INDENT[1]);

  fprintf( output, "%svirtual ~%s() {};\n\n", INDENT[1], project_name);

  //!Closing class declaration.
  fprintf( output,"%s};\n", INDENT[0] );
  fprintf( output, "#endif  //_%s_H\n\n", upper_project_name);

  fclose( output);
}


//!Creates Formatted Registers Header File
void CreateRegsHeader() {
  extern ac_dec_format *format_reg_list;
  extern ac_sto_list *storage_list;
  extern char* project_name;
  extern char* upper_project_name;

  ac_sto_list *pstorage;
  ac_dec_format *pformat;
  ac_dec_field *pfield;

  int flag = 1;
  FILE *output = NULL;
  char filename[256];

  sprintf( filename, "%s_fmt_regs.H", project_name);

  for( pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next ){
    if(( pstorage->type == REG ) && (pstorage->format != NULL )){
      if(flag){  //Print this just once.
        if ( !(output = fopen( filename, "w"))){
          perror("ArchC could not open output file");
          exit(1);
        }

        print_comment( output, "ArchC Formatted Registers header file.");
        fprintf( output, "#ifndef  %s_FMT_REGS_H\n", upper_project_name);
        fprintf( output, "#define  %s_FMT_REGS_H\n\n", upper_project_name);

        fprintf( output, "#include  <iostream>\n");
        fprintf( output, "#include  <string>\n");
        fprintf( output, "#include  \"ac_reg.H\"\n");
        fprintf( output, "#include  \"%s_parms.H\"\n", project_name);
        fprintf( output, "\n\n");

        fprintf( output, "using std::ostream;\n");
        fprintf( output, "using std::string;\n\n");

        COMMENT(INDENT[0],"ArchC classes for formatted registers.\n");
        flag = 0;
      }

      for( pformat = format_reg_list; pformat!= NULL; pformat=pformat->next){
        if(!(strcmp(pformat->name, pstorage->format))){
          break;
        }
      }
      //Declaring formatted register class.
      fprintf( output, "class %s_fmt_%s {\n", project_name, pstorage->name);
      fprintf( output, "%sstring name;\n", INDENT[1]);
      fprintf( output, "public:\n");

      //TO DO: Registers with parameterized size. The templated class ac_reg
      //       is still not working with sc_unit<x> types.
      for( pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
        fprintf( output,"%sac_reg<unsigned> %s;\n",INDENT[1],pfield->name );

      fprintf( output, "\n\n");

      //Declaring class constructor.
      if (ACDelayFlag)
        fprintf( output, "%s%s_fmt_%s(char* n, double& ts): \n",
                 INDENT[1], project_name, pstorage->name);
      else
        fprintf( output, "%s%s_fmt_%s(char* n): \n",
                 INDENT[1], project_name, pstorage->name);

      for( pfield = pformat->fields; pfield->next != NULL; pfield = pfield->next) {
        //Initializing field names with reg name. This is to enable Formatted Reg stats.
        //Need to be changed if we adopt statistics collection for each field individually.
        if (ACDelayFlag)
          fprintf( output,"%s%s(\"%s\",%d,ts),\n",
                   INDENT[2],pfield->name,pstorage->name, 0 );
        else
          fprintf( output,"%s%s(\"%s\",%d),\n",
                   INDENT[2],pfield->name,pstorage->name, 0 );
      }

      //Last field.
      if (ACDelayFlag)
        fprintf( output,"%s%s(\"%s\",%d,ts){name = n;}\n\n",
                 INDENT[2],pfield->name,pstorage->name, 0 );
      else
        fprintf( output,"%s%s(\"%s\",%d){name = n;}\n\n",
                 INDENT[2],pfield->name,pstorage->name, 0 );

      fprintf( output,"%svoid change_dump(ostream& output){}\n\n",INDENT[1] );
      fprintf( output,"%svoid reset_log(){}\n\n",INDENT[1] );
      if (ACDelayFlag) {
        fprintf( output,"%svoid commit_delays(double time)\n",INDENT[1] );
        fprintf( output,"%s{\n",INDENT[1] );
        for( pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
          fprintf( output,"%s%s.commit_delays(time);\n",
                   INDENT[2], pfield->name);
        }
        fprintf( output,"%s}\n",INDENT[1] );
      }
      fprintf( output, "};\n\n");
    }
  }

  if(!flag){ //We had at last one formatted reg declared.
    fprintf( output, "#endif // %s_FMT_REGS_H\n", upper_project_name);
    fclose(output);
  }
}


//!Create the header file for ArchC statistics collection class.
void CreateStatsHeaderTmpl(void){
  extern ac_sto_list *storage_list;
  extern char *project_name;
  extern char *upper_project_name;
  extern ac_dec_instr *instr_list;

  ac_dec_instr *pinstr;

  char filename[256];

  FILE *output;

  sprintf(filename, "%s_stats.H.tmpl", project_name);

  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment(output, "ArchC Processor statistics data header file.");

  fprintf( output, "#ifndef  %s_STATS_H\n", upper_project_name);
  fprintf( output, "#define  %s_STATS_H\n\n", upper_project_name);

  fprintf( output, "#include  <fstream>\n");
  fprintf( output, "#include  \"%s_parms.H\"\n", project_name);
  fprintf( output, "#include  \"ac_stats.H\"\n");
  fprintf( output, "\n");

  // Declaring processor stats
  fprintf(output, "AC_SET_STATS(%s, INSTRUCTIONS, SYSCALLS);\n", project_name);

  // Declaring instruction stats
  fprintf(output, "AC_SET_INSTR_STATS(%s, COUNT);\n\n", project_name);

  // Declaring proc_all_stats struct
  fprintf(output, "struct %s_all_stats {\n", project_name);

  // Declaring processor stats collector object
  fprintf(output, "%s%s_stats stats;\n\n", INDENT[1], project_name);

  // Declaring instruction stats collector objects
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    fprintf(output, "%s%s_instr_stats %s_istats;\n",
            INDENT[1], project_name, pinstr->name);
  }
  fprintf(output, "\n");

  // Declaring instruction stats collector object array
  fprintf(output,
      "%s%s_instr_stats* instr_stats[%s_parms::AC_DEC_INSTR_NUMBER + 1];\n\n",
      INDENT[1], project_name, project_name);

  // Defining constructor
  fprintf(output, "%s%s_all_stats();\n", INDENT[1], project_name);

  // Closing proc_stats struct
  fprintf(output, "}; // struct %s_stats\n", project_name);

  //END OF FILE!
  fprintf(output, "#endif // %s_STATS_H\n", upper_project_name);
  fclose(output);
}


//!Create the implementation file for ArchC statistics collection class.
void CreateStatsImplTmpl() {
  extern char *project_name;
  extern ac_dec_instr *instr_list;

  ac_dec_instr *pinstr;

  char filename[256];

  FILE *output;

  sprintf(filename, "%s_stats.cpp.tmpl", project_name);

  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment(output, "ArchC Processor statistics data implementation file.");

  fprintf(output, "#include \"%s_stats.H\"\n\n", project_name);

  // Defining processor stat list
  fprintf(output, "AC_CONF_STAT_LIST(%s, INSTRUCTIONS, SYSCALLS);\n",
      project_name);

  // Defining instruction stat list
  fprintf(output, "AC_CONF_INSTR_STAT_LIST(%s, COUNT);\n\n", project_name);

  // Defining constructor
  fprintf(output, "%s_all_stats::%s_all_stats() :\n", project_name,
      project_name);
  fprintf(output, "%sstats(\"%s\")\n", INDENT[1], project_name);
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    fprintf(output, "%s, %s_istats(\"%s\", stats)\n",
            INDENT[1], pinstr->name, pinstr->name);
  }
  fprintf(output, "{\n");

  COMMENT(INDENT[2], "Configuring stats collectors for each instruction");
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    fprintf(output, "%sinstr_stats[%d] = &%s_istats;\n",
            INDENT[2], pinstr->id, pinstr->name);
  }
  fprintf(output, "}\n");

  //END OF FILE!
  fclose(output);
}


/////////////////////////// Create Implementation Functions ////////////////////////////

//!Creates Processor Module Implementation File
void CreateProcessorImpl() {
    extern ac_sto_list *storage_list;
    extern char *project_name;
    extern int HaveMemHier, ACGDBIntegrationFlag, largest_format_size;
    ac_sto_list *pstorage;

    extern ac_dec_instr *instr_list;

    char* filename;
    FILE* output;

    filename = (char*) malloc(strlen(project_name)+strlen(".cpp")+1);
    sprintf( filename, "%s.cpp", project_name);

    if ( !(output = fopen( filename, "w"))){
        perror("ArchC could not open output file");
        exit(1);
    }

    print_comment( output, "Processor Module Implementation File.");
    fprintf( output, "#include  \"%s.H\"\n", project_name);
    fprintf( output, "#include  \"%s_isa.cpp\"\n\n", project_name);

    if( ACABIFlag )
        fprintf( output, "#include  \"%s_syscall.H\"\n\n", project_name);

    if( ACThreading )
        EmitDispatch(output, 0);

    fprintf( output, "void %s::behavior() {\n\n", project_name);
    if( ACDebugFlag ){
        fprintf( output, "%sextern bool ac_do_trace;\n", INDENT[1]);
        fprintf( output, "%sextern ofstream trace_file;\n", INDENT[1]);
    }

    if( ACThreading )
        EmitVetLabelAt(output, 1);
    else
        fprintf( output, "%sunsigned ins_id;\n", INDENT[1]);

    /* Delayed program loading */
    fprintf(output, "%sif (has_delayed_load) {\n", INDENT[1]);
    fprintf(output, "%s%s_mport.load(delayed_load_program);\n", INDENT[2], load_device->name);
    fprintf(output, "%sac_pc = ac_start_addr;\n", INDENT[2]);
    fprintf(output, "%shas_delayed_load = false;\n", INDENT[2]);
    fprintf(output, "%s}\n\n", INDENT[1]);

    /*if( HaveMemHier ) {
      fprintf( output, "%sif( ac_wait_sig ) {\n", INDENT[1]);
      fprintf( output, "%sreturn;\n", INDENT[2]);
      fprintf( output, "%s}\n\n", INDENT[1]);
      }*/

    if( ACFullDecode ) {
        fprintf(output, "%sfor (decode_pc = ac_pc; decode_pc < dec_cache_size; decode_pc += %d) {\n",
                INDENT[1], largest_format_size / 8);
        EmitDecodification(output, 2);
        fprintf( output, "%s}\n\n", INDENT[1]);
    }

    if ( ACThreading && ACABIFlag && ACDecCacheFlag) {
        fprintf( output, "%s#define AC_SYSC(NAME,LOCATION) \\\n", INDENT[1]);
        fprintf( output, "%sinstr_dec = (DEC_CACHE + (LOCATION", INDENT[1]);
        if( ACIndexFix )
            fprintf( output, " / %d", largest_format_size / 8);
        fprintf( output, ")); \\\n");

        if ( !ACFullDecode )
            fprintf( output, "%sinstr_dec->valid = true; \\\n", INDENT[1]);

        fprintf( output, "%sinstr_dec->id = 0; \\\n", INDENT[1]);
        fprintf( output, "%sinstr_dec->end_rot = &&Sys_##LOCATION;\n\n", INDENT[1]);

        fprintf( output, "%s#include <ac_syscall.def>\n", INDENT[1]);
        fprintf( output, "%s#undef AC_SYSC\n\n", INDENT[1]);
    }

    // Longjmp of ac_annul_sig and ac_stop_flag
    fprintf( output, "%sint action = setjmp(ac_env);\n", INDENT[1]);
    if (ACLongJmpStop || ACThreading)
        fprintf( output, "%sif (action == AC_ACTION_STOP) return;\n\n", INDENT[1]);

    //Emitting processor behavior method implementation.
    if( ACThreading )
        EmitInstrExec(output, 1);
    else
        EmitProcessorBhv(output, 1);

    fprintf( output, "%s} // behavior()\n\n", INDENT[0]);

    //Emitting Verification Method.
    if (ACVerboseFlag) {
        COMMENT(INDENT[0],"Verification method.\n");
        fprintf( output, "%svoid %s::ac_verify(){\n", INDENT[0], project_name);
        fprintf(output, "%sfor (;;) {\n\n", INDENT[1]);

        fprintf( output, "%sif( done.read() ) {\n", INDENT[1]);

        fprintf( output, "#ifdef AC_VERBOSE\n");
        for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){
            fprintf( output, "%s%s.change_dump(cerr);\n",
                    INDENT[2],pstorage->name );
        }
        fprintf( output, "#endif\n");

        fprintf( output, "#ifdef AC_UPDATE_LOG\n");
        for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){
            fprintf( output, "%s%s.reset_log();\n",
                    INDENT[2],pstorage->name );
        }
        fprintf( output, "#endif\n");

        fprintf( output, "%sdone.write(0);\n", INDENT[2]);
        fprintf( output, "%s}\n\n", INDENT[1]);

        fprintf(output, "%swait();\n\n", INDENT[1]);
        fprintf(output, "%s}\n", INDENT[1]);
        fprintf( output, "%s}\n\n", INDENT[0]);
    }

    /* SIGNAL HANDLERS */
    fprintf(output, "#include <ac_sighandlers.H>\n");
    fprintf(output, "#include <ac_args.H>\n\n");

    /* init() and stop() */
    /* init() with no parameters */
    fprintf(output, "void %s::init() {\n", project_name);

    /*CACHE*/
    for (pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next) {
        switch(pstorage->type) {
            case CACHE:
            case ICACHE:
            case DCACHE:
                fprintf(output, "%sif (ac_cache_traces.find(\"%s\") != ac_cache_traces.end()) "
                        "%s.set_trace(*ac_cache_traces[\"%s\"]);\n",
                        INDENT[1], pstorage->name, pstorage->name, pstorage->name);
            default:
                continue;
        }

    }

    fprintf(output, "\n#ifdef AC_VERIFY\n");
    fprintf(output, "%sset_queue(av[0]);\n", INDENT[1]);
    fprintf(output, "#endif\n\n");
    fprintf(output, "%sac_pc = ac_start_addr;\n", INDENT[1]);
    fprintf(output, "%sISA._behavior_begin();\n", INDENT[1]);
    fprintf(output, "%scerr << endl << \"ArchC: -------------------- Starting Simulation --------------------\" << endl;\n",
            INDENT[1]);
    fprintf(output, "%sInitStat();\n", INDENT[1]);

    //  if(ACABIFlag)
    //    fprintf( output, "%sISA.syscall.set_prog_args(argc, argv);\n",INDENT[1]);
    fprintf( output, "%sstart_up = 0;\n", INDENT[1]);
    if( ACDecCacheFlag )
        fprintf( output, "%sinit_dec_cache();\n", INDENT[1]);

    fprintf(output, "%ssignal(SIGINT, sigint_handler);\n", INDENT[1]);
    fprintf(output, "%ssignal(SIGTERM, sigint_handler);\n", INDENT[1]);
    fprintf(output, "%ssignal(SIGSEGV, sigsegv_handler);\n", INDENT[1]);
    fprintf(output, "%ssignal(SIGUSR1, sigusr1_handler);\n", INDENT[1]);
    fprintf(output, "#ifdef USE_GDB\n");
    fprintf(output, "%ssignal(SIGUSR2, sigusr2_handler);\n", INDENT[1]);
    fprintf(output, "#endif\n");
    fprintf(output, "%sset_running();\n", INDENT[1]);
    fprintf(output, "}\n\n");

    /* init() with 3 parameters */
    fprintf(output, "void %s::init(int ac, char *av[]) {\n\n", project_name);
    //  fprintf(output, "%sac_init_opts( ac, av);\n", INDENT[1]);
    fprintf(output, "%sargs_t args = ac_init_args( ac, av);\n", INDENT[1]);
    fprintf(output, "%sset_args(args.size, args.app_args);\n", INDENT[1]);
    fprintf(output, "%s%s_mport.load(args.app_filename);\n", INDENT[1], load_device->name);

    if (ACGDBIntegrationFlag) {
        fprintf(output, "%senable_gdb(args.gdb_port);\n", INDENT[1]);
    }

    for (pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next) {
        switch(pstorage->type) {
            case CACHE:
            case ICACHE:
            case DCACHE:
                fprintf(output, "%sif (ac_cache_traces.find(\"%s\") != ac_cache_traces.end()) "
                        "%s.set_trace(*ac_cache_traces[\"%s\"]);\n",
                        INDENT[1], pstorage->name, pstorage->name, pstorage->name);

            default: continue;

        }
    }

    fprintf(output, "#ifdef AC_VERIFY\n");
    fprintf(output, "%sset_queue(av[0]);\n", INDENT[1]);
    fprintf(output, "#endif\n\n");
    fprintf(output, "%sac_pc = ac_start_addr;\n", INDENT[1]);
    fprintf(output, "%sISA._behavior_begin();\n", INDENT[1]);
    fprintf(output, "%scerr << endl << \"ArchC: -------------------- Starting Simulation --------------------\" << endl;\n",
            INDENT[1]);
    fprintf(output, "%sInitStat();\n", INDENT[1]);

    fprintf( output, "%sstart_up = 0;\n", INDENT[1]);
    if( ACDecCacheFlag )
        fprintf( output, "%sinit_dec_cache();\n", INDENT[1]);

    fprintf(output, "%ssignal(SIGINT, sigint_handler);\n", INDENT[1]);
    fprintf(output, "%ssignal(SIGTERM, sigint_handler);\n", INDENT[1]);
    fprintf(output, "%ssignal(SIGSEGV, sigsegv_handler);\n", INDENT[1]);
    fprintf(output, "%ssignal(SIGUSR1, sigusr1_handler);\n", INDENT[1]);
    fprintf(output, "#ifdef USE_GDB\n");
    fprintf(output, "%ssignal(SIGUSR2, sigusr2_handler);\n", INDENT[1]);
    fprintf(output, "#endif\n");
    fprintf(output, "%sset_running();\n", INDENT[1]);
    fprintf(output, "}\n\n");

    fprintf(output, "void %s::set_prog_args(){\n", project_name);
    if (ACABIFlag)
        fprintf(output, "%sISA.syscall.set_prog_args(argc, argv);\n",
                INDENT[1]);
    fprintf(output, "}\n\n");

    /* stop() */
    fprintf(output, "//Stop simulation (may receive exit status)\n");
    fprintf(output, "void %s::stop(int status) {\n", project_name);

    fprintf(output, "%scerr << endl << \"ArchC: -------------------- Simulation Finished --------------------\" << endl;\n",
            INDENT[1]);
    fprintf(output, "%sISA._behavior_end();\n", INDENT[1]);
    fprintf(output, "%sac_stop_flag = 1;\n", INDENT[1]);
    fprintf(output, "%sac_exit_status = status;\n", INDENT[1]);
    fprintf(output, "%sset_stopped();\n", INDENT[1]);
    if (ACLongJmpStop)
        fprintf(output, "%slongjmp(ac_env, AC_ACTION_STOP);\n", INDENT[1]);
    fprintf(output, "}\n\n");

    /* Program loading functions */
    /* load() */
    fprintf(output, "void %s::load(char* program) {\n", project_name);
    fprintf(output, "%s%s_mport.load(program);\n", INDENT[1], load_device->name);
    fprintf(output, "}\n\n");

    /* delayed_load() */
    fprintf(output, "void %s::delayed_load(char* program) {\n", project_name);
    fprintf(output, "%shas_delayed_load = true;\n", INDENT[1]);
    fprintf(output, "%sdelayed_load_program = new char[strlen(program)];\n",
            INDENT[1]);
    fprintf(output, "%sstrcpy(delayed_load_program, program);\n", INDENT[1]);
    fprintf(output, "}\n\n");

    /* Some simple GDB support methods */
    if (ACGDBIntegrationFlag) {
        /* get_gdbstub() */
        fprintf(output, "// Returns pointer to gdbstub\n");
        fprintf(output, "AC_GDB<%s_parms::ac_word>* %s::get_gdbstub() {\n",
                project_name, project_name);
        fprintf(output, "%sreturn gdbstub;\n", INDENT[1]);
        fprintf(output, "}\n\n");

        /* get_ac_tgt_endian() */
        fprintf(output, "// Returns true if model endianness doesn't match with host's, false otherwise\n");
        fprintf(output, "bool %s::get_ac_tgt_endian() {\n", project_name);
        fprintf(output, "%sreturn ac_tgt_endian;\n", INDENT[1]);
        fprintf(output, "}\n\n");

        /* ac_stop() */
        fprintf(output, "// Stops the processor\n");
        fprintf(output, "void %s::ac_stop() {\n", project_name);
        fprintf(output, "%sstop();\n", INDENT[1]);
        fprintf(output, "}\n\n");
    }

    /* get_ac_pc() */
    fprintf(output, "// Returns ac_pc value\n");
    fprintf(output, "unsigned %s::get_ac_pc() {\n", project_name);
    fprintf(output, "%sreturn ac_pc;\n", INDENT[1]);
    fprintf(output, "}\n\n");

    /* set_ac_pc() */
    fprintf(output, "// Assigns value to ac_pc\n");
    fprintf(output, "void %s::set_ac_pc(unsigned int value) {\n", project_name);
    fprintf(output, "%sac_pc = value;\n", INDENT[1]);
    fprintf(output, "}\n\n");

    /* PrintStat() */
    fprintf(output, "// Wrapper function to PrintStat().\n");
    fprintf(output, "void %s::PrintStat() {\n", project_name);
    fprintf(output, "%sac_arch<%s_parms::ac_word, %s_parms::ac_Hword>::PrintStat();\n",
            INDENT[1], project_name, project_name);



    if (HaveMemHier) {
        for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next) {
            switch(pstorage->type) {
                case CACHE:
                case ICACHE:
                case DCACHE:
                    fprintf(output, "%sstd::cerr << \"cache: %s\\n\";\n", INDENT[1], pstorage->name);
                    fprintf(output, "%s%s.print_statistics(std::cerr);\n", INDENT[1], pstorage->name);
                    break;
                default:
                    continue;
            }
        }
    }



    fprintf(output, "}\n\n");

    if (ACWaitFlag) {
        /* set_proc_freq() */
        fprintf(output, "// Assigns value to processor frequency and updates cycle time values\n");
        fprintf(output, "void %s::set_proc_freq(unsigned int proc_freq) {\n", project_name);
        fprintf(output, "%sac_module::set_proc_freq(proc_freq);\n", INDENT[1]);

        for(int cycles=1; cycles<=5; cycles++) {
            for (ac_dec_instr *pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
                if (pinstr->cycles == cycles) {
                    fprintf(output, "%stime_%dcycle=sc_time(%d*module_period_ns, SC_NS);\n", INDENT[1], cycles, cycles);
                    break;
                }
            }
        }

        fprintf(output, "}\n\n");

    }
    /* GDB enable method */
    if (ACGDBIntegrationFlag) {
        fprintf(output, "// Enables GDB\n");
        fprintf(output, "void %s::enable_gdb(int port) {\n", project_name);
        fprintf(output, "%sunsigned gdbport = 5000;\n", INDENT[1]);
        fprintf(output, "%sif (port > 1024)\n", INDENT[1]);
        fprintf(output, "%sgdbport = port;\n\n", INDENT[2]);

        fprintf(output,
                "%sgdbstub = new AC_GDB<%s_parms::ac_word>(this, gdbport);\n",
                INDENT[1], project_name);
        fprintf(output, "%sgdbstub->set_port(gdbport);\n", INDENT[1]);
        fprintf(output, "%sgdbstub->enable();\n", INDENT[1]);
        fprintf(output, "%sgdbstub->connect();\n", INDENT[1]);
        fprintf(output, "}\n\n");
    }

    //!END OF FILE.
    fclose(output);
    free(filename);
}

/** Creates the _arch.cpp Implementation File. */
void CreateArchImpl() {
    extern ac_sto_list *storage_list, *fetch_device, *first_level_data_device;
    extern int HaveMemHier, HaveTLMPorts, HaveTLM2IntrPorts, HaveTLM2Ports,
        HaveTLM2NBPorts, HaveTLM2IntrPorts;
    extern ac_sto_list *load_device;
    extern char *project_name;
    ac_sto_list *pstorage;
    FILE *output;
    char filename[256];

    sprintf(filename, "%s_arch.cpp", project_name);

    load_device = storage_list;
    if (!(output = fopen(filename, "w"))) {
        perror("ArchC could not open output file");
        exit(1);
    }

    print_comment(output, "ArchC Resources Implementation file.");

    fprintf(output, "#include \"%s_arch.H\"\n\n", project_name);

    if (HaveMemHier) {
        fprintf(output, "#include \"ac_cache_if.H\"\n");
    }

    fprintf(output, "\n");

    /* Emitting Constructor */
    fprintf(output, "%s%s_arch::%s_arch() :\n", INDENT[0], project_name,
            project_name);
    fprintf(output, "%sac_arch_dec_if<%s_parms::ac_word, "
                    "%s_parms::ac_Hword>(%s_parms::AC_MAX_BUFFER),\n",
            INDENT[1], project_name, project_name, project_name);

    /* Constructing ac_pc */
    fprintf(output, "%sac_pc(\"ac_pc\", 0", INDENT[1]);
    if (ACDelayFlag) {
        fprintf(output, ", time_step");
    }
    fprintf(output, "),\n");

    /* Constructing ac_id */
    fprintf(output, "%sac_id(\"ac_id\", 0),\n", INDENT[1]);


    for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next) {
        switch (pstorage->type) {
        case REG:
            // Formatted registers have a special class.
            if (pstorage->format != NULL)
                fprintf(output, "%s%s(\"%s\"", INDENT[1], pstorage->name,
                        pstorage->name);
            else
                fprintf(output, "%s%s(\"%s\", 0", INDENT[1], pstorage->name,
                        pstorage->name);

            if (ACDelayFlag)
                fprintf(output, ", time_step");

            fprintf(output, ")");
            break;

        case REGBANK:
            // Emitting register bank. Checking is a register width was
            // declared.
            fprintf(output, "%s%s(\"%s\"", INDENT[1], pstorage->name,
                    pstorage->name);
            if (ACDelayFlag)
                fprintf(output, ", time_step");

            fprintf(output, ")");
            break;

        case CACHE:
        case ICACHE:
        case DCACHE:
            if (!pstorage->parms) { // It is a generic cache. Just emit a base
                                    // container object.
                fprintf(output, "%s%s(\"%s\", %uU),\n", INDENT[1],
                        pstorage->name, pstorage->name, pstorage->size);
                fprintf(output, "%s%s(*this, %s)", INDENT[1], pstorage->name,
                        pstorage->name);
            } else {
                // It is an ac_cache object.
                fprintf(output, "%s%s(%s_mport,globalId)", INDENT[1],
                        pstorage->name, pstorage->higher->name);
                fprintf(output, ",\n%s%s_if(%s)", INDENT[1], pstorage->name,
                        pstorage->name);
                fprintf(output, ",\n%s%s_mport(*this, %s_if)", INDENT[1],
                        pstorage->name, pstorage->name);
            }
            break;

        case MEM:
            fprintf(output, "%s%s(\"%s\", %uU),\n", INDENT[1], pstorage->name,
                    pstorage->name, pstorage->size);
            fprintf(output, "%s%s_mport(*this, %s)", INDENT[1], pstorage->name,
                    pstorage->name);
            break;

        case TLM_PORT:
            fprintf(output, "%s%s(\"%s\", %uU),\n", INDENT[1], pstorage->name,
                    pstorage->name, pstorage->size);
            fprintf(output, "%s%s_mport(*this, %s)", INDENT[1], pstorage->name,
                    pstorage->name);
            break;

        case TLM2_PORT:
            fprintf(output, "%s%s(\"%s\", %uU),\n", INDENT[1], pstorage->name,
                    pstorage->name, pstorage->size);
            fprintf(output, "%s%s_mport(*this, %s)", INDENT[1], pstorage->name,
                    pstorage->name);
            break;

        case TLM2_NB_PORT:
            fprintf(output, "%s%s(\"%s\", %uU),\n", INDENT[1], pstorage->name,
                    pstorage->name, pstorage->size);
            fprintf(output, "%s%s_mport(*this, %s)", INDENT[1], pstorage->name,
                    pstorage->name);
            break;

        default:
            fprintf(output, "%s%s(\"%s\", %uU),\n", INDENT[1], pstorage->name,
                    pstorage->name, pstorage->size);
            fprintf(output, "%s%s_mport(*this, %s)", INDENT[1], pstorage->name,
                    pstorage->name);
            break;
        }
        if (pstorage->next != NULL)
            fprintf(output, ",\n");
    }

    if (HaveTLMIntrPorts || HaveTLM2IntrPorts) {
        fprintf(output, "\n%s,intr_reg(\"instr_reg\",1)", INDENT[1]);
    }

    /* opening constructor body */
    fprintf(output, " {\n\n");

    /* setting endianness match */
    fprintf(output, "%sac_mt_endian = %s_parms::AC_MATCH_ENDIAN;\n", INDENT[1],
            project_name);

    /* setting target endianness */
    fprintf(output, "%sac_tgt_endian = %s_parms::AC_PROC_ENDIAN;\n\n",
            INDENT[1], project_name);

    /* Determining which device is gonna be used for fetching instructions */
    if (!fetch_device) {
        // The parser has not determined because there is not an ac_icache obj
        // declared.
        // In this case, look for the object with the lowest (zero) hierarchy
        // level.
        for (pstorage = storage_list; pstorage != NULL;
             pstorage = pstorage->next)
            if (pstorage->level == 0 && pstorage->type != REG &&
                pstorage->type != REGBANK && pstorage->type != TLM_INTR_PORT &&
                pstorage->type != TLM2_INTR_PORT)
                fetch_device = pstorage;

        if (!fetch_device) { // Couldn't find a fetch device. Error!
            AC_INTERNAL_ERROR("Could not determine a device for fetching.");
            exit(1);
        }
    }

    fprintf(output, "%sINST_PORT = &%s_mport;\n", INDENT[1],
            fetch_device->name);

    fprintf(output, "%sDATA_PORT = &%s_mport;\n", INDENT[1],
            first_level_data_device->name);

    fprintf(output, "}\n\n");

    fprintf(output, "int %s_arch::globalId = 0;", project_name);
}

/*!Create the template for the .cpp file where the user has
  the basic code for the main function. */
void CreateMainTmpl() {
  extern char *project_name;
  char filename[] = "main.cpp.tmpl";
  char description[256];

  FILE  *output;

  sprintf( description, "This is the main file for the %s ArchC model", project_name);
  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, description);

  fprintf( output, "const char *project_name=\"%s\";\n", project_name);
  fprintf( output, "const char *project_file=\"%s\";\n", arch_filename);
  fprintf( output, "const char *archc_version=\"%s\";\n", ACVERSION);
  fprintf( output, "const char *archc_options=\"%s\";\n", ACOptions);
  fprintf( output, "\n");

  fprintf( output, "#include  <iostream>\n");
  fprintf( output, "#include  <systemc.h>\n");
  fprintf( output, "#include  \"ac_stats_base.H\"\n");
  fprintf( output, "#include  \"%s.H\"\n\n", project_name);

  fprintf( output, "\n\n");
  fprintf( output, "int sc_main(int ac, char *av[])\n");
  fprintf( output, "{\n\n");

  COMMENT(INDENT[1],"%sISA simulator", INDENT[1]);
  fprintf( output, "%s%s %s_proc1(\"%s\");\n\n",
           INDENT[1], project_name, project_name, project_name);

  fprintf( output, "#ifdef AC_DEBUG\n");
  fprintf( output, "%sac_trace(\"%s_proc1.trace\");\n",
           INDENT[1], project_name);
  fprintf( output, "#endif \n\n");

  fprintf(output, "%s%s_proc1.init(ac, av);\n", INDENT[1], project_name);
  fprintf(output, "%s%s_proc1.set_prog_args();\n", INDENT[1], project_name);

  fprintf(output, "%scerr << endl;\n\n", INDENT[1]);

  fprintf(output, "%ssc_start();\n\n", INDENT[1]);

  fprintf(output, "%s%s_proc1.PrintStat();\n", INDENT[1], project_name);
  fprintf(output, "%scerr << endl;\n\n", INDENT[1]);

  fprintf( output, "#ifdef AC_STATS\n");
  fprintf( output, "%sac_stats_base::print_all_stats(std::cerr);\n",
           INDENT[1]);
  fprintf( output, "#endif \n\n");

  fprintf( output, "#ifdef AC_DEBUG\n");
  fprintf( output, "%sac_close_trace();\n", INDENT[1]);
  fprintf( output, "#endif \n\n");

  fprintf( output, "%sreturn %s_proc1.ac_exit_status;\n",
           INDENT[1], project_name);

  fprintf( output, "}\n");
}


/*!Create the template for the .cpp file where the user has
  to fill out the instruction and format behaviors. */
void CreateImplTmpl(){
  extern ac_dec_format *format_ins_list;
  extern ac_dec_instr *instr_list;
  extern ac_grp_list* group_list;
  extern char *project_name;
  extern int wordsize;
  extern int declist_num;

  ac_grp_list* pgroup;
  ac_instr_ref_list* pref;
  ac_dec_format *pformat;
  ac_dec_instr *pinstr;
  ac_dec_field *pdecfield;
  ac_dec_list* pdeclist;

  char filename[256];
  char description[] = "Behavior implementation file template.";
  char initfilename[256];

  //File containing ISA declaration
  FILE  *output;

  int i;
  int count_fields;

  sprintf( filename, "%s_isa.cpp.tmpl", project_name);
  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, description);
  fprintf( output, "#include  \"%s_isa.H\"\n", project_name);
  fprintf( output, "#include  \"%s_isa_init.cpp\"\n", project_name);
  fprintf(output, "#include  \"%s_bhv_macros.H\"\n", project_name);
  fprintf( output, "\n");

  COMMENT(INDENT[0],
          "'using namespace' statement to allow access to all %s-specific datatypes",
          project_name);
  fprintf( output, "using namespace %s_parms;\n\n", project_name);

  //Behavior to begin simulation.
  COMMENT(INDENT[0],"Behavior executed before simulation begins.");
  fprintf( output, "%svoid ac_behavior( begin ){};\n", INDENT[0]);
  fprintf( output, "\n");

  //Behavior to end simulation.
  COMMENT(INDENT[0],"Behavior executed after simulation ends.");
  fprintf( output, "%svoid ac_behavior( end ){};\n", INDENT[0]);
  fprintf( output, "\n");

  //Declaring ac_instruction behavior method.
  COMMENT(INDENT[0],"Generic instruction behavior method.");
  fprintf( output, "%svoid ac_behavior( instruction ){};\n", INDENT[0]);
  fprintf( output, " \n");

  //Declaring Instruction Format behavior methods.
  COMMENT(INDENT[0]," Instruction Format behavior methods.");
  for( pformat = format_ins_list; pformat!= NULL; pformat=pformat->next)
    fprintf( output, "%svoid ac_behavior( %s ){}\n",
             INDENT[0], pformat->name);

  fprintf( output, " \n");

  //Declaring each instruction behavior method.
  for( pinstr = instr_list; pinstr!= NULL; pinstr=pinstr->next){
    COMMENT(INDENT[0],"Instruction %s behavior method.",pinstr->name);
    fprintf( output, "%svoid ac_behavior( %s ){}\n\n",
             INDENT[0], pinstr->name);
  }

  //!END OF FILE.
  fclose(output);


  /* ac_isa_init creation starts here */
  /* Name for ISA initialization file. */
  sprintf( initfilename, "%s_isa_init.cpp", project_name);
  if ( !(output = fopen( initfilename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, "AC_ISA Initialization File");

  /* Creating static decoder tables */
  fprintf( output, "%s#include \"%s_isa.H\"\n", INDENT[0], project_name);
  fprintf(output, "\n");
  /* Creating group tables. */
  for (pgroup = group_list; pgroup != NULL; pgroup = pgroup->next) {
    COMMENT(INDENT[0], "Group %s table initialization.", pgroup->name);
    fprintf(output, "%sbool %s_parms::%s_isa::group_%s[%s_parms::AC_DEC_INSTR_NUMBER] =\n%s{\n",
            INDENT[0], project_name, project_name, pgroup->name,
            project_name, INDENT[1]);
    for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
      for (pref = pgroup->instrs; pref != NULL; pref = pref->next)
        if (pref->instr == pinstr)
          break;

      if (pref == NULL)
        fprintf(output, "%sfalse", INDENT[2]);
      else
        fprintf(output, "%strue", INDENT[2]);

      if (pinstr->next)
        fprintf(output, ",\n");
      else
        fprintf(output, "\n%s};\n\n", INDENT[1]);
    }
  }
  COMMENT(INDENT[0],"Fields table declaration.");

  /* Creating field table */
  fprintf(output, "ac_dec_field %s_parms::%s_isa::fields[%s_parms::AC_DEC_FIELD_NUMBER] = {\n",
          project_name, project_name, project_name);
  i = 0;
  for (pformat = format_ins_list; pformat != NULL; pformat = pformat->next) {
    for (pdecfield = pformat->fields; pdecfield != NULL; pdecfield = pdecfield->next) {
      /* fprintf {char* name, int size, int first_bit, int id, long val, int sign, next} */
      fprintf(output, "%s{\"%s\", %d, %d, %d, %ld, %d, ",
              INDENT[1],
              pdecfield->name,
              pdecfield->size,
              pdecfield->first_bit,
              pdecfield->id,
              pdecfield->val,
              pdecfield->sign);
      if (pdecfield->next)
        fprintf(output, "&(%s_parms::%s_isa::fields[%d])},\n", project_name, project_name, i + 1);
      else
        fprintf(output, "NULL}");

      i++;
    }
    if (pformat->next)
      fprintf(output, ",\n");
  }
  fprintf(output, "\n};\n\n");

  /* Creating format structure */
  fprintf(output, "ac_dec_format %s_parms::%s_isa::formats[%s_parms::AC_DEC_FORMAT_NUMBER] = {\n",
          project_name, project_name, project_name);
  i = 0;
  count_fields = 0;
  for( pformat = format_ins_list; pformat!= NULL; pformat = pformat->next){
    /* fprintf int id, char* name, int size, ac_dec_field* fields, next */
    fprintf(output, "%s{%d, \"%s\", %d, &(%s_parms::%s_isa::fields[%d]), ",
            INDENT[1],
            pformat->id,
            pformat->name,
            pformat->size,
            project_name,
            project_name,
            count_fields);
    if (pformat->next)
      fprintf(output, "&(%s_parms::%s_isa::formats[%d])},\n",
              project_name,
              project_name,
              i + 1);
    else
      fprintf(output, "NULL}");

    i++;
    for( pdecfield = pformat->fields;
         pdecfield!= NULL; pdecfield=pdecfield->next)
      count_fields++;
  }
  fprintf(output, "\n};\n\n");

  /* Creating decode list structure */
  fprintf(output, "ac_dec_list %s_parms::%s_isa::dec_list[%s_parms::AC_DEC_LIST_NUMBER] = {\n",
          project_name, project_name, project_name);
  i = 0;
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    for (pdeclist = pinstr->dec_list; pdeclist != NULL;
         pdeclist = pdeclist->next) {
      /* fprintf char* name, int id, int value, ac_dec_list* next  */
      fprintf(output, "%s{\"%s\", %d, %d, ",
              INDENT[1],
              pdeclist->name,
              pdeclist->id,
              pdeclist->value);
      if (pdeclist->next)
        fprintf(output, "&(%s_parms::%s_isa::dec_list[%d])},\n",
                project_name, project_name, i + 1);
      else
        fprintf(output, "NULL}");

      i++;
    }
    if (pinstr->next)
      fprintf(output, ",\n");
  }
  fprintf(output, "\n};\n\n");
  declist_num = i;

  /* Creating instruction structure */
  fprintf(output,
          "ac_dec_instr %s_parms::%s_isa::instructions[%s_parms::AC_DEC_INSTR_NUMBER] = {\n",
          project_name, project_name, project_name);
  i = 0;
  count_fields = 0;
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    /* fprintf char* name, int size, char* mnemonic, char* asm_str, char* format, unsigned id, unsigned cycles, unsigned min_latency, unsigned max_latency, ac_dec_list* dec_list, ac_control_flow* cflow, ac_dec_instr* next */
    fprintf(output, "%s{\"%s\", %d, \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, &(%s_parms::%s_isa::dec_list[%d]), %d, ",
            INDENT[1],
            pinstr->name,
            pinstr->size,
            pinstr->mnemonic,
            pinstr->asm_str,
            pinstr->format,
            pinstr->id,
            pinstr->cycles,
            pinstr->min_latency,
            pinstr->max_latency,
            project_name,
            project_name,
            count_fields,
            0);
    if (pinstr->next)
      fprintf(output, "&(%s_parms::%s_isa::instructions[%d])},\n", project_name, project_name, i + 1);
    else
      fprintf(output, "NULL}\n");
    for (pdeclist = pinstr->dec_list; pdeclist != NULL; pdeclist = pdeclist->next)
      count_fields++;
    i++;
  }
  fprintf(output, "};\n\n");

  /* Creating instruction table */
  fprintf(output, "const ac_instr_info\n");
  fprintf(output, "%s_parms::%s_isa::instr_table[%s_parms::AC_DEC_INSTR_NUMBER + 1] = {\n",
          project_name, project_name, project_name);
  fprintf(output, "%sac_instr_info(0, \"_ac_invalid_\", \"_ac_invalid_\", %d),\n",
          INDENT[1], wordsize / 8);
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    fprintf(output, "%sac_instr_info(%d, \"%s\", \"%s\", %d)",
            INDENT[1],
            pinstr->id,
            pinstr->name,
            pinstr->mnemonic,
            pinstr->size);
    if (pinstr->next)
      fprintf(output, ",\n");
  }
  fprintf(output, "\n};\n\n");

  /* Creating instruction format table */
  fprintf(output, "const unsigned %s_parms::%s_isa::instr_format_table[%s_parms::AC_DEC_INSTR_NUMBER + 1] = {\n",
          project_name, project_name, project_name);
  fprintf(output, "%s0,\n", INDENT[1]);
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    fprintf(output, "%s%d",
            INDENT[1], (FindFormat(format_ins_list, pinstr->format))->id);
    if (pinstr->next) fprintf(output, ",\n");
  }
  fprintf(output, "\n};\n");

  //!END OF FILE.
  fclose(output);

}


///Creates the .cpp template file for interrupt handlers.
void CreateIntrTLM2Tmpl() {
  extern ac_sto_list *tlm2_intr_port_list;
  extern char* project_name;

  ac_sto_list *pport;

  FILE *output;
  char filename[256];
  char description[] = "Interrupt Handlers implementation file.";

  sprintf(filename, "%s_intr_handlers.cpp.tmpl", project_name);

  if (!(output = fopen( filename, "w"))) {
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, description);

  fprintf(output, "#include \"ac_intr_handler.H\"\n");
  fprintf(output, "#include \"%s_intr_handlers.H\"\n\n", project_name);
  fprintf(output, "#include \"%s_ih_bhv_macros.H\"\n\n", project_name);

  COMMENT(INDENT[0], "'using namespace' statement to allow access to all %s-specific datatypes", project_name);
  fprintf( output, "using namespace %s_parms;\n\n", project_name);

  //Declaring formatted register behavior methods.
  for (pport = tlm2_intr_port_list; pport != NULL; pport = pport->next) {
    fprintf(output, "// Interrupt handler behavior for interrupt port %s.\n", pport->name);
    fprintf(output,
    "void ac_behavior(%s, value, addr) {\n"
   	"   if (value == INTR_PROC_OFF) { \n"
	"       //printf(\"\\nINSTR_HANDLER: Processor %%d (%s) is sleeping.\" , ac_id.read()); \n"
	"       intr_reg.write(value); \n"
	"   } \n"
	"   else if (value == INTR_PROC_ON) { \n"
	"       //printf(\"\\nINSTR_HANDLER: Processor %%d (%s) is waking up.\" , ac_id.read()); \n"
	"       intr_reg.write(value); \n"
	"       wake->notify(sc_core::SC_ZERO_TIME); \n\n"
	"       /* ac_release update a signal to re-start the processor simulator   */ \n"
	"       /* See %s_isa::ac_behavior (instruction) (%s_isa.cpp)               */ \n"
	"       //ac_release();	\n"
	"   }\n"
	"   else { \n"
	"       printf(\"\\nINSTR_HANDLER: Processor %%d (%s) unrecognized interuption code %%d. Ignoring.\", ac_id.read(), value); \n"
	"   }\n}", pport->name, project_name,  project_name, project_name, project_name, project_name);


  }

  //END OF FILE.
  fclose(output);
}



///Creates the .cpp template file for interrupt handlers.
void CreateIntrTmpl() {
  extern ac_sto_list *tlm_intr_port_list;

  extern char* project_name;

  ac_sto_list *pport;

  FILE *output;
  char filename[256];
  char description[] = "Interrupt Handlers implementation file.";

  sprintf(filename, "%s_intr_handlers.cpp.tmpl", project_name);

  if (!(output = fopen( filename, "w"))) {
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, description);

  fprintf(output, "#include \"ac_intr_handler.H\"\n");
  fprintf(output, "#include \"%s_intr_handlers.H\"\n\n", project_name);
  fprintf(output, "#include \"%s_ih_bhv_macros.H\"\n\n", project_name);

  COMMENT(INDENT[0],
          "'using namespace' statement to allow access to all %s-specific datatypes",
          project_name);
  fprintf( output, "using namespace %s_parms;\n\n", project_name);

  //Declaring formatted register behavior methods.
  for (pport = tlm_intr_port_list; pport != NULL; pport = pport->next) {
    fprintf(output, "// Interrupt handler behavior for interrupt port %s.\n",
            pport->name);
    fprintf(output, "void ac_behavior(%s, value, addr) {\n}\n\n", pport->name);
  }

  //END OF FILE.
  fclose(output);
}







//!Create ArchC model syscalls
void CreateArchSyscallHeader()
{
  extern char *project_name;
  FILE *output;
  char filename[50];

  snprintf(filename, 50, "%s_syscall.H.tmpl", project_name);

  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, "ArchC Architecture Dependent Syscall header file.");

  fprintf(output,
          "#ifndef %s_SYSCALL_H\n"
          "#define %s_SYSCALL_H\n"
          "\n"
          "#include \"%s_arch.H\"\n"
          "#include \"%s_arch_ref.H\"\n"
          "#include \"%s_parms.H\"\n"
          "#include \"ac_syscall.H\"\n"
          "\n"
          "//%s system calls\n"
          "class %s_syscall : public ac_syscall<%s_parms::ac_word, %s_parms::ac_Hword>, public %s_arch_ref\n"
          "{\n"
          "public:\n"
          "  %s_syscall(%s_arch& ref) : ac_syscall<%s_parms::ac_word, %s_parms::ac_Hword>(ref, %s_parms::AC_RAMSIZE), %s_arch_ref(ref) {};\n"
          "  virtual ~%s_syscall() {};\n\n"
          "  void get_buffer(int argn, unsigned char* buf, unsigned int size);\n"
          "  void set_buffer(int argn, unsigned char* buf, unsigned int size);\n"
          "  void set_buffer_noinvert(int argn, unsigned char* buf, unsigned int size);\n"
          "  int  get_int(int argn);\n"
          "  void set_int(int argn, int val);\n"
          "  void return_from_syscall();\n"
          "  void set_prog_args(int argc, char **argv);\n"
          "};\n"
          "\n"
          "#endif\n"
          , project_name, project_name, project_name, project_name,
          project_name, project_name, project_name, project_name,
          project_name, project_name, project_name, project_name,
          project_name, project_name, project_name, project_name,
          project_name);

  fclose( output);
}

void CreateArchSyscallTmpl() {
    extern char *project_name;
    FILE *output;
    char filename[50];

    char description[] = "Syscall implementation file template.";

    sprintf(filename, "%s_syscall.cpp.tmpl", project_name);
    if (!(output = fopen(filename, "w"))) {
        perror("ArchC could not open output file");
        exit(1);
    }

    print_comment(output, description);
    fprintf(output, "#include  \"%s_syscall.H\"\n", project_name);
    fprintf(output, "\n");

    COMMENT(INDENT[0], "'using namespace' statement to allow access to all "
                       "%s-specific datatypes",
            project_name);
    fprintf(output, "using namespace %s_parms;\n\n", project_name);

    fprintf(output, "void %s_syscall::get_buffer(int argn, unsigned char* buf, "
                    "unsigned int size) { }\n",
            project_name);
    fprintf(output, "void %s_syscall::set_buffer(int argn, unsigned char* buf, "
                    "unsigned int size) { }\n",
            project_name);
    fprintf(output, "void %s_syscall::set_buffer_noinvert(int argn, unsigned "
                    "char* buf, unsigned int size) { }\n",
            project_name);
    fprintf(output, "int  %s_syscall::get_int(int argn) { }\n", project_name);
    fprintf(output, "void %s_syscall::set_int(int argn, int val) { }\n",
            project_name);
    fprintf(output, "void %s_syscall::return_from_syscall() { }\n",
            project_name);
    fprintf(output,
            "void %s_syscall::set_prog_args(int argc, char **argv) { } \n",
            project_name);
}

///Creates the header file for interrupt handlers.
void CreateIntrTLM2Header() {

  extern ac_sto_list *tlm2_intr_port_list;
  extern char *project_name;
  extern char *upper_project_name;

  ac_sto_list *pport;

  char filename[256];
  char description[] = "Interrupt Handlers header file.";

  // Header File.
  FILE  *output;

  sprintf(filename, "%s_intr_handlers.H", project_name);

  if (!(output = fopen( filename, "w"))) {
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment(output, description);
  fprintf(output, "#ifndef _%s_INTR_HANDLERS_H\n", upper_project_name);
  fprintf(output, "#define _%s_INTR_HANDLERS_H\n\n", upper_project_name);
  fprintf(output, "#include \"ac_intr_handler.H\"\n");
  fprintf(output, "#include \"%s_parms.H\"\n", project_name);
  fprintf(output, "#include \"%s_arch.H\"\n", project_name);
  fprintf(output, "#include \"%s_arch_ref.H\"\n", project_name);
  fprintf(output, "\n");

  // TODO: see what we need to do for tlm2 ports below
  /* Creating interrupt handler classes for each ac_tlm_intr_port */

  for (pport = tlm2_intr_port_list; pport != NULL; pport = pport->next) {
    fprintf(output, "class %s_%s_handler :\n", project_name, pport->name);
    fprintf(output, "%spublic ac_intr_handler,\n", INDENT[1]);
    fprintf(output, "%spublic %s_arch_ref\n", INDENT[1], project_name);
    fprintf(output, "{\n");
    fprintf(output, " public:\n\n");
    //fprintf(output, "%sexplicit %s_%s_handler(%s_arch& ref) : %s_arch_ref(ref) {}\n\n",
    //        INDENT[1], project_name, pport->name, project_name, project_name);
    fprintf(output, "%sexplicit %s_%s_handler(%s_arch& ref,sc_event *event) : %s_arch_ref(ref) { wake = event; }\n\n",
      INDENT[1], project_name, pport->name, project_name, project_name);

    fprintf(output, "%svoid handle(uint32_t value, uint64_t addr);\n\n", INDENT[1]);
    fprintf(output, "%ssc_event *wake;\n", INDENT[1]);
    fprintf(output, "};\n\n\n");
  }

  fprintf(output, "#endif // _%s_INTR_HANDLERS_H\n", upper_project_name);

  //END OF FILE
  fclose(output);

}

///Creates the header file for interrupt handlers.
void CreateIntrHeader() {

  extern ac_sto_list *tlm_intr_port_list;
  extern char *project_name;
  extern char *upper_project_name;

  ac_sto_list *pport;

  char filename[256];
  char description[] = "Interrupt Handlers header file.";

  // Header File.
  FILE  *output;

  sprintf(filename, "%s_intr_handlers.H", project_name);

  if (!(output = fopen( filename, "w"))) {
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment(output, description);
  fprintf(output, "#ifndef _%s_INTR_HANDLERS_H\n", upper_project_name);
  fprintf(output, "#define _%s_INTR_HANDLERS_H\n\n", upper_project_name);
  fprintf(output, "#include \"ac_intr_handler.H\"\n");
  fprintf(output, "#include \"%s_parms.H\"\n", project_name);
  fprintf(output, "#include \"%s_arch.H\"\n", project_name);
  fprintf(output, "#include \"%s_arch_ref.H\"\n", project_name);
  fprintf(output, "\n");

  /* Creating interrupt handler classes for each ac_tlm_intr_port */
  for (pport = tlm_intr_port_list; pport != NULL; pport = pport->next) {
    fprintf(output, "class %s_%s_handler :\n", project_name, pport->name);
    fprintf(output, "%spublic ac_intr_handler,\n", INDENT[1]);
    fprintf(output, "%spublic %s_arch_ref\n", INDENT[1], project_name);
    fprintf(output, "{\n");
    fprintf(output, " public:\n\n");
    fprintf(output,
            "%sexplicit %s_%s_handler(%s_arch& ref) : %s_arch_ref(ref) {}\n\n",
            INDENT[1], project_name, pport->name, project_name, project_name);
    fprintf(output, "%svoid handle(uint32_t value, uint64_t addr);\n\n", INDENT[1]);
    fprintf(output, "};\n\n\n");
  }

  fprintf(output, "#endif // _%s_INTR_HANDLERS_H\n", upper_project_name);

  //END OF FILE
  fclose(output);

}

///Creates the header file with ac_behavior macros for interrupt handlers.
void CreateIntrTLM2MacrosHeader() {
  extern char *project_name;
  extern char *upper_project_name;

  char filename[256];
  char description[] = "TLM 2.0 Interrupt Handlers ac_behavior macros header file.";

  // Header File.
  FILE  *output;

  sprintf(filename, "%s_ih_bhv_macros.H", project_name);

  if (!(output = fopen( filename, "w"))) {
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment(output, description);
  fprintf(output, "#ifndef _%s_IH_BHV_MACROS_H\n", upper_project_name);
  fprintf(output, "#define _%s_IH_BHV_MACROS_H\n\n\n", upper_project_name);

  /* ac_behavior main macro */
  fprintf(output, "#define ac_behavior(intrp, value, addr) %s_##intrp##_handler::handle(uint32_t value, uint64_t addr)\n\n",
          project_name);

  fprintf(output, "#endif // _%s_IH_BHV_MACROS_H\n", upper_project_name);

  //END OF FILE
  fclose(output);

}




///Creates the header file with ac_behavior macros for interrupt handlers.
void CreateIntrMacrosHeader() {
  extern char *project_name;
  extern char *upper_project_name;

  char filename[256];
  char description[] = "Interrupt Handlers ac_behavior macros header file.";

  // Header File.
  FILE  *output;

  sprintf(filename, "%s_ih_bhv_macros.H", project_name);

  if (!(output = fopen( filename, "w"))) {
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment(output, description);
  fprintf(output, "#ifndef _%s_IH_BHV_MACROS_H\n", upper_project_name);
  fprintf(output, "#define _%s_IH_BHV_MACROS_H\n\n\n", upper_project_name);

  /* ac_behavior main macro */
  fprintf(output, "#define ac_behavior(intrp, value, addr) %s_##intrp##_handler::handle(uint32_t value, uint64_t addr)\n\n",
          project_name);

  fprintf(output, "#endif // _%s_IH_BHV_MACROS_H\n", upper_project_name);

  //END OF FILE
  fclose(output);

}


//!Create Makefile
void CreateMakefile(){
  extern ac_dec_format *format_ins_list;
  extern char *project_name;
  extern int HaveMemHier;
  extern int HaveFormattedRegs;
  extern int HaveTLMPorts;
  extern int HaveTLM2Ports;
  extern int HaveTLM2NBPorts;
  extern int HaveTLMIntrPorts;
  extern int HaveTLM2IntrPorts;

  FILE *output;
  char filename[] = "Makefile";

  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  COMMENT_MAKE("####################################################");
  COMMENT_MAKE("This is the Makefile for building the %s ArchC model",
               project_name);
  COMMENT_MAKE("This file is automatically generated by ArchC");
  COMMENT_MAKE("WITHOUT WARRANTY OF ANY KIND, either express");
  COMMENT_MAKE("or implied.");
  COMMENT_MAKE("For more information on ArchC, please visit:   ");
  COMMENT_MAKE("http://www.archc.org                           ");
  COMMENT_MAKE("                                               ");
  COMMENT_MAKE("The ArchC Team                                 ");
  COMMENT_MAKE("Computer Systems Laboratory (LSC)              ");
  COMMENT_MAKE("IC-UNICAMP                                     ");
  COMMENT_MAKE("http://www.lsc.ic.unicamp.br                   ");
  COMMENT_MAKE("####################################################");

  fprintf( output, "\n\n");

  fprintf( output, "INC_DIR := -I. `pkg-config --cflags systemc` `pkg-config --cflags archc` ");
  if (HaveTLMPorts || HaveTLMIntrPorts || HaveTLM2Ports || HaveTLM2NBPorts || HaveTLM2IntrPorts)
     fprintf(output, "`pkg-config --cflags tlm`");

  fprintf( output, "\n\nLIB_SYSTEMC := `pkg-config --libs systemc`\n");
  fprintf( output, "LIB_ARCHC := `pkg-config --libs archc`\n");
  fprintf( output, "LIB_POWERSC := %s\n", (ACPowerEnable) ? "`pkg-config --libs powersc`" : "");
  fprintf( output, "LIB_DWARF := %s\n", (ACHLTraceFlag) ? "-ldw -lelf" : "" );
  fprintf( output, "LIBS := $(LIB_SYSTEMC) $(LIB_ARCHC) $(LIB_POWERSC) $(LIB_DWARF) -lm $(EXTRA_LIBS)\n");
  fprintf( output, "CC :=  %s", CC_PATH);
  fprintf( output, "OPT :=  %s", OPT_FLAGS);
  fprintf( output, "DEBUG :=  %s", DEBUG_FLAGS);
  fprintf( output, "OTHER := -std=c++11 ");

  //!< The guest arch is big endian?
  if ( ac_tgt_endian )
    fprintf( output, " -DAC_GUEST_BIG_ENDIAN");

  //!< The guest and host arch is the same endianness?
  if ( ac_match_endian )
    fprintf( output, " -DAC_MATCH_ENDIANNESS");

  fprintf( output, " %s", OTHER_FLAGS);

  fprintf( output, "CFLAGS := $(DEBUG) $(OPT) $(OTHER) %s %s\n",
           (ACGDBIntegrationFlag) ? "-DUSE_GDB" : "",
           (ACPowerEnable) ? "-DPOWER_SIM=\\\"$(PWD)/powersc\\\"" : "");

  fprintf( output, "\nTARGET := %s\n\n", project_name);

  //Declaring ACSRCS variable
  COMMENT_MAKE("These are the source files automatically generated by ArchC, that must appear in the SRCS variable");
  fprintf( output, "ACSRCS := $(TARGET)_arch.cpp $(TARGET)_arch_ref.cpp ");
  fprintf( output, "$(TARGET).cpp\n\n");

  //Declaring ACINCS variable
  COMMENT_MAKE("These are the source files automatically generated  by ArchC that are included by other files in ACSRCS");
  fprintf( output, "ACINCS := $(TARGET)_isa_init.cpp\n\n");

  //Declaring ACHEAD variable
  COMMENT_MAKE("These are the header files automatically generated by ArchC");
  fprintf( output, "ACHEAD := $(TARGET)_parms.H $(TARGET)_arch.H $(TARGET)_arch_ref.H $(TARGET)_isa.H $(TARGET)_bhv_macros.H ");
  if(HaveFormattedRegs)
    fprintf( output, "$(TARGET)_fmt_regs.H ");
  if(ACStatsFlag)
    fprintf( output, "%s_stats.H ", project_name);
  if(HaveTLMIntrPorts || HaveTLM2IntrPorts)
    fprintf( output,  " $(TARGET)_intr_handlers.H $(TARGET)_ih_bhv_macros.H ");
  fprintf( output, "$(TARGET).H\n\n");

  //if(HaveTLMIntrPorts)
  //  fprintf( output, "$(TARGET)_intr_handlers.H $(TARGET)_ih_bhv_macros.H ");
  //fprintf( output, "\n\n");

  //if(HaveTLM2IntrPorts)
  //       fprintf( output, "$(TARGET)_intr_handlers.H $(TARGET)_ih_bhv_macros.H ");

  fprintf( output, "\n");

  //Declaring FILES variable
//  COMMENT_MAKE("These are the source files provided by ArchC that must be compiled together with the ACSRCS");
//  COMMENT_MAKE("They are stored in the archc/src/aclib directory");
//  fprintf( output, "ACFILES := ");
//  if( HaveMemHier )
//    fprintf( output, "ac_cache.cpp ac_mem.cpp ac_cache_if.cpp ");
//  fprintf( output, "\n\n");

//Declaring ACLIBFILES variable
  COMMENT_MAKE("These are the library files provided by ArchC");
  COMMENT_MAKE("They are stored in the archc/lib directory");

  fprintf(output, "ACLIBFILES := ac_decoder_rt.o ac_module.o ac_mem.o ac_utils.o "HLT_OBJ" ");
  if(ACABIFlag)
      fprintf(output, "ac_syscall.o ");
  if(HaveTLMPorts)
      fprintf(output, "ac_tlm_port.o ");
  if(HaveTLM2Ports)
      fprintf(output, "ac_tlm2_port.o ");
  if(HaveTLM2NBPorts)
      fprintf(output, "ac_tlm2_nb_port.o ");
  if(HaveTLMIntrPorts)
      fprintf(output, "ac_tlm_intr_port.o ");
  if(HaveTLM2IntrPorts)
      fprintf(output, "ac_tlm2_intr_port.o ");
  fprintf(output, "\n\n");

//  //Declaring FILESHEAD variable
//  COMMENT_MAKE("These are the headers files provided by ArchC");
//  COMMENT_MAKE("They are stored in the archc/include directory");
//  fprintf( output, "ACFILESHEAD := ac_decoder_rt.H ac_module.H ac_mem.H ac_utils.H ac_regbank.H ac_debug_model.H ac_sighandlers.H ac_ptr.H ac_memport.H ac_arch.H ac_arch_dec_if.H ac_arch_ref.H "HLT_HEADER" ");
//  if (ACABIFlag)
//      fprintf(output, "ac_syscall.H ");
//  if (HaveTLMPorts)
//      fprintf(output, "ac_tlm_port.H ");
//  if (HaveTLM2Ports)
//      fprintf(output, "ac_tlm2_port.H ");
//  if (HaveTLM2NBPorts)
//      fprintf(output, "ac_tlm2_nb_port.H ");
//  if (HaveTLMIntrPorts)
//      fprintf(output, "ac_tlm_intr_port.H ");
//  if (HaveTLM2IntrPorts)
//      fprintf(output, "ac_tlm2_intr_port.H ");
//  if (HaveTLMPorts || HaveTLMIntrPorts || HaveTLM2NBPorts || HaveTLM2Ports || HaveTLM2IntrPorts)
//      fprintf(output, "ac_tlm_protocol.H ");
//  if (ACStatsFlag)
//      fprintf(output, "ac_stats.H ac_stats_base.H ");
//  fprintf(output, "\n\n");

  //Declaring SRCS variable
  //Note: Removed $(TARGET)_isa.cpp, added as include inside $(TARGET).cpp
  COMMENT_MAKE("These are the source files provided by the user + ArchC sources");
  fprintf( output, "SRCS := main.cpp $(ACSRCS) %s",
          (ACGDBIntegrationFlag)?"$(TARGET)_gdb_funcs.cpp":"");
  if (ACABIFlag)
      fprintf( output, " $(TARGET)_syscall.cpp");
  if (HaveTLMIntrPorts || HaveTLM2IntrPorts )
      fprintf( output, " $(TARGET)_intr_handlers.cpp");
  if (ACStatsFlag)
      fprintf( output, " $(TARGET)_stats.cpp");
  fprintf( output, "\n\n");

  //Declaring OBJS variable
  fprintf( output, "OBJS := $(SRCS:.cpp=.o)\n\n");

  //Declaring Executable name
  fprintf( output, "EXE := $(TARGET).x\n\n");

  //Declaring dependencie rules
  fprintf( output, ".SUFFIXES: .cc .cpp .o .x\n\n");

  //fprintf( output, "all: $(addprefix %s/include/, $(ACFILESHEAD))", ARCHCDIR);
  fprintf( output, "all: " );
  if (HaveTLMPorts || HaveTLMIntrPorts || HaveTLM2Ports || HaveTLM2NBPorts || HaveTLM2IntrPorts) {
      fprintf( output, " lib \n\n");
  } else {
      if (ACABIFlag)
          fprintf( output, " $(TARGET)_syscall.H");
      fprintf( output, " $(ACHEAD) $(EXE)\n\n");
  }
  fprintf( output, "$(EXE): $(OBJS)\n");
  fprintf( output, "\t$(CC) $(CFLAGS) $(INC_DIR) $(LIB_DIR) -o $@ $(OBJS) $(LIBS) 2>&1 | c++filt\n\n");

  COMMENT_MAKE("Copy from template if main.cpp not exist");
  fprintf(output, "main.cpp:\n");
  fprintf(output, "\tcp main.cpp.tmpl main.cpp\n\n");

  if (ACABIFlag) {
      COMMENT_MAKE("Copy from template if %s_syscall.H not exist", project_name);
      fprintf(output, "%s_syscall.H:\n", project_name);
      fprintf(output, "\tcp %s_syscall.H.tmpl %s_syscall.H\n\n", project_name, project_name);
  }

  if (ACStatsFlag) {
      COMMENT_MAKE("Copy from template if %s_stats.H not exist", project_name);
      fprintf(output, "%s_stats.H:\n", project_name);
      fprintf(output, "\tcp %s_stats.H.tmpl %s_stats.H\n\n", project_name, project_name);

      COMMENT_MAKE("Copy from template if %s_stats.cpp not exist", project_name);
      fprintf(output, "%s_stats.cpp:\n", project_name);
      fprintf(output, "\tcp %s_stats.cpp.tmpl %s_stats.cpp\n\n", project_name, project_name);
  }

  if (HaveTLMIntrPorts || HaveTLM2IntrPorts) {
      COMMENT_MAKE("Copy from template if %s_intr_handlers.cpp not exist", project_name);
      fprintf( output, "%s_intr_handlers.cpp:\n", project_name);
      fprintf( output, "\tcp %s_intr_handlers.cpp.tmpl %s_intr_handlers.cpp\n\n", project_name, project_name);
  }

  if (HaveTLMPorts || HaveTLMIntrPorts || HaveTLM2Ports || HaveTLM2NBPorts || HaveTLM2IntrPorts) {
      fprintf( output, "lib: %s.cpp $(OBJS)\n", project_name);
      fprintf( output, "\tar r lib$(TARGET).a $(OBJS)\n\n");

//      fprintf( output, "%s.cpp:\n", project_name);
//      fprintf( output, "\techo --- No simulator found, using acsim to generate one.\n");
//      fprintf( output, "\t$(ARCHC_PATH)/bin/acsim %s_$(TRANSPORT).ac -abi -ndc -pw\n", project_name);
//      fprintf( output, "\t$(MAKE) lib\n\n");
  }

  fprintf( output, ".cpp.o:\n");
  fprintf( output, "\t$(CC) $(CFLAGS) $(INC_DIR) -c $<\n\n");

  fprintf( output, ".cc.o:\n");
  fprintf( output, "\t$(CC) $(CFLAGS) $(INC_DIR) -c $<\n\n");

  fprintf( output, "clean:\n");
  fprintf( output, "\trm -f $(OBJS) *~ $(EXE) core *.o *.a \n\n");

  fprintf( output, "model_clean:\n");
  //fprintf( output, "\trm -f $(ACSRCS) $(ACHEAD) $(ACINCS) $(ACFILESHEAD)  *.tmpl loader.ac \n\n");
  fprintf( output, "\trm -f $(ACSRCS) $(ACHEAD) $(ACINCS) *.tmpl loader.ac \n\n");

  fprintf( output, "sim_clean: clean model_clean\n\n");

  fprintf( output, "distclean: sim_clean\n");
  fprintf( output, "\trm -f main.cpp Makefile\n\n");

}


////////////////////////////////////////////////////////////////////////////////////
// Emit functions ...                                                             //
// These Functions are used by the Create functions declared above to write files //
////////////////////////////////////////////////////////////////////////////////////

/**************************************/
/*!  Emits a method to update pipe regs
  \brief Used by EmitProcessorBhv and EmitDispatch functions      */
/***************************************/
void EmitUpdateMethod( FILE *output, int base_indent ) {
  extern int HaveMemHier;
  extern ac_sto_list *storage_list;

  ac_sto_list *pstorage;

  //Emitting Update Method.
  if( ACDelayFlag || HaveMemHier || ACWaitFlag) {
    COMMENT(INDENT[base_indent],"Updating Regs for behavioral simulation.");
  }

  if (!ACLongJmpStop) {
    fprintf( output, "%sif (ac_stop_flag) ", INDENT[base_indent]);
    if (ACThreading)
      fprintf(output, "longjmp(ac_env, AC_ACTION_STOP);\n\n");
    else
      fprintf(output, "return;\n\n");
  }

  if( ACDelayFlag ){
    fprintf( output, "%sif(!ac_wait_sig){\n", INDENT[base_indent]);
    for( pstorage = storage_list; pstorage!= NULL; pstorage = pstorage->next ) {
        if (pstorage->has_memport)
            fprintf( output, "%s%s_mport.commit_delays( (double)ac_cycle_counter );\n", INDENT[base_indent + 1], pstorage->name);
        else
            fprintf( output, "%s%s.commit_delays( (double)ac_cycle_counter );\n", INDENT[base_indent + 1], pstorage->name);
    }

    fprintf( output, "%sac_pc.commit_delays(  (double)ac_cycle_counter );\n",
             INDENT[base_indent + 1]);
    fprintf( output, "%sif(!ac_parallel_sig)\n", INDENT[base_indent + 1]);
    fprintf( output, "%sac_cycle_counter++;\n", INDENT[base_indent + 2]);
    fprintf( output, "%selse\n", INDENT[base_indent + 1]);
    fprintf( output, "%sac_parallel_sig = 0;\n\n", INDENT[base_indent + 2]);
    fprintf( output, "%s}\n", INDENT[base_indent]);
  }

  /*if( HaveMemHier ) {
    for( pstorage = storage_list; pstorage!= NULL; pstorage = pstorage->next ) {
      if( pstorage->type == CACHE || pstorage->type == ICACHE ||
          pstorage->type == DCACHE || pstorage->type == MEM )
        fprintf( output, "%s%s.process_request( );\n",
                 INDENT[base_indent], pstorage->name);
    }
  }*/

  if (ACWaitFlag) {
    fprintf(output, "%sif (ac_qk.need_sync()) {\n", INDENT[base_indent]);
    fprintf(output, "%sac_qk.sync();\n", INDENT[base_indent + 1]);
    fprintf(output, "%s}\n", INDENT[base_indent]);
  }
}


/**************************************/
/*!  Emits the if statement that handles instruction decodification
  \brief Used by EmitProcessorBhv and EmitDispatch functions */
/***************************************/
void EmitDecodification( FILE *output, int base_indent) {
  extern int wordsize, fetchsize, HaveMemHier, largest_format_size;

  /*if( HaveMemHier ){
    if (fetchsize == wordsize)
      fprintf( output, "%s*((ac_fetch*)(fetch)) = IM->read( ac_pc );\n\n",
               INDENT[base_indent]);
    else if (fetchsize == wordsize/2)
      fprintf( output, "%s*((ac_fetch*)(fetch)) = IM->read_half( ac_pc );\n\n",
               INDENT[base_indent]);
    else if (fetchsize == 8)
      fprintf( output, "%s*((ac_fetch*)(fetch)) = IM->read_byte( ac_pc );\n\n",
               INDENT[base_indent]);
    else {
      AC_ERROR("Fetchsize differs from wordsize or (wordsize/2) or 8: not implemented.");
      exit(EXIT_FAILURE);
    }



    fprintf( output, "%sif( ac_wait_sig ) {\n", INDENT[base_indent]);

   */

    //if (ACThreading)
    //  fprintf(output, "%slongjmp(ac_env, AC_ACTION_STOP);\n",
    //          INDENT[base_indent + 1]);
    //else
    //  fprintf( output, "%sreturn;\n", INDENT[base_indent+1]);


  //}

  if( ACDecCacheFlag ){
    fprintf( output, "%sinstr_dec = (DEC_CACHE + (", INDENT[base_indent]);
    if (ACFullDecode)
      fprintf( output, "decode_pc");
    else
      fprintf( output, "ac_pc");

    if( ACIndexFix )
      fprintf( output, " / %d", largest_format_size / 8);
    fprintf( output, "));\n");

    if( !ACFullDecode ) {
      fprintf( output, "%sif ( !instr_dec->valid ){\n", INDENT[base_indent]);
      base_indent++;
    }

    fprintf( output, "%sunsigned* ins_cache;\n", INDENT[base_indent]);
  }

  if( !ACFullDecode )
    fprintf( output, "%sdecode_pc = ac_pc;\n", INDENT[base_indent]);

  fprintf( output, "%squant = 0;\n", INDENT[base_indent]);
  fprintf( output, "%sins_cache = (ISA.decoder)->Decode(reinterpret_cast<unsigned char*>(buffer), quant);\n",
           INDENT[base_indent]);

  if( ACDecCacheFlag ){
    if( ACFullDecode ) {
      fprintf( output, "%sif( ins_cache ) {\n",
               INDENT[base_indent]);
      base_indent++;
    }
    else
      fprintf( output, "%sinstr_dec->valid = true;\n",
               INDENT[base_indent]);

    fprintf( output, "%sinstr_dec->id = ins_cache ? ins_cache[IDENT]: 0;\n",
             INDENT[base_indent]);

    if (ACThreading)
      fprintf( output, "%sinstr_dec->end_rot = IntRoutine[instr_dec->id];\n",
               INDENT[base_indent]);

    EmitDecCacheAt( output, base_indent);

    base_indent--;
    fprintf( output, "%s}\n", INDENT[base_indent]);

    if( !ACFullDecode )
      fprintf( output, "%sins_id = instr_dec->id;\n\n", INDENT[base_indent]);
  }
  else {
    fprintf( output, "%sins_id = ins_cache ? ins_cache[IDENT]: 0;\n\n", INDENT[base_indent]);

    //Checking if it is a valid instruction
    fprintf( output, "%sif( ins_id == 0 ) {\n", INDENT[base_indent]);
    fprintf( output, "%scerr << \"ArchC Error: Unidentified instruction. \" << endl;\n",
            INDENT[base_indent+1]);
    fprintf( output, "%scerr << \"PC = \" << hex << ac_pc << dec << endl;\n",
            INDENT[base_indent+1]);
    fprintf( output, "%sstop();\n", INDENT[base_indent+1]);
    if (ACThreading)
      fprintf(output, "%slongjmp(ac_env, AC_ACTION_STOP);\n",
              INDENT[base_indent + 1]);
    else
      fprintf( output, "%sreturn;\n", INDENT[base_indent+1]);
    fprintf( output, "%s}\n\n", INDENT[base_indent]);
  }
}


/**************************************/
/*!  Emit initial code for executing instructions
  \brief Used by EmitInstrExec and EmitDispatch functions */
/***************************************/
void EmitInstrExecIni( FILE *output, int base_indent) {
  extern ac_dec_field *common_instr_field_list;
  extern ac_dec_format *format_ins_list;

  ac_dec_format *pformat;
  ac_dec_field *pfield;

  if( ACGDBIntegrationFlag )
    fprintf( output, "%sif (gdbstub && gdbstub->stop(ac_pc)) gdbstub->process_bp();\n\n",
             INDENT[base_indent]);

  if ( ACCurInstrID )
    fprintf(output, "%sISA.cur_instr_id = ins_id;\n", INDENT[base_indent]);

  if (!(ACThreading && ACABIFlag)) {
    fprintf(output, "%sISA._behavior_instruction(", INDENT[base_indent]);
    /* common_instr_field_list has the list of fields for the generic instruction. */
    if( ACDecCacheFlag ){
      for( pfield = common_instr_field_list, pformat = format_ins_list;
          pfield != NULL; pfield = pfield->next) {
        fprintf(output, "instr_dec->F_%s.%s", pformat->name, pfield->name);
        if (pfield->next != NULL)
          fprintf(output, ", ");
      }
    }
    else {
      for( pfield = common_instr_field_list;
          pfield != NULL; pfield = pfield->next){
        fprintf(output, "ins_cache[%d]", pfield->id);
        if (pfield->next != NULL)
          fprintf(output, ", ");
      }
    }
    fprintf(output, ");\n");
  }
}


/**************************************/
/*!  Emit code for executing instructions
  \brief Used by EmitProcessorBhv function */
/***************************************/
void EmitInstrExec( FILE *output, int base_indent){
    extern ac_dec_instr *instr_list;
    extern ac_dec_format *format_ins_list;
    extern char* project_name;

    ac_dec_format *pformat;
    ac_dec_instr *pinstr;
    ac_dec_field *pfield;

    if( ACThreading ) {
        fprintf(output, "%sI_Init:\n", INDENT[base_indent]);
        fprintf(output, "%sgoto *dispatch();\n\n", INDENT[base_indent + 1]);

        if ( ACABIFlag && ACDecCacheFlag ) {
            fprintf( output, "%s#define AC_SYSC(NAME,LOCATION) \\\n",
                    INDENT[base_indent]);
            fprintf( output, "%sSys_##LOCATION: \\\n", INDENT[base_indent]);
            base_indent++;

            if( ACStatsFlag ){
                fprintf( output, "%sISA.stats[%s_stat_ids::SYSCALLS]++; \\\n",
                        INDENT[base_indent], project_name);
            }

            fprintf( output, "%sISA.syscall.NAME(); \\\n", INDENT[base_indent]);
            fprintf( output, "%sgoto *dispatch();\n\n", INDENT[base_indent]);
            base_indent--;

            fprintf( output, "%s#include <ac_syscall.def>\n", INDENT[base_indent]);
            fprintf( output, "%s#undef AC_SYSC\n\n", INDENT[base_indent]);
        }
    }
    else {

        if( ACDebugFlag ){
            fprintf( output, "%sif( ac_do_trace != 0 ) \n", INDENT[base_indent]);
            fprintf( output, PRINT_TRACE, INDENT[base_indent+1]);
            fprintf( output, "\n");
        }
        if( ACHLTraceFlag)
        {
          fprintf( output, "%sgenerate_trace_for_address(ac_pc);\\\n", INDENT[base_indent]);
        }

        EmitInstrExecIni( output, base_indent );

        /* Switch statement for instruction selection */
        fprintf(output, "%sswitch (ins_id) {\n", INDENT[base_indent]);
    }

    for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
        if( ACThreading ) {
            fprintf(output, "%sI_%s: // Instruction %s\n",
                    INDENT[base_indent], pinstr->name, pinstr->name);
            if ( ACABIFlag ) {
                fprintf(output, "%sISA._behavior_instruction(", INDENT[base_indent + 1]);
                /* common_instr_field_list has the list of fields for the generic instruction. */
                if( ACDecCacheFlag ){
                    for (pformat = format_ins_list;
                            (pformat != NULL) && strcmp(pinstr->format, pformat->name);
                            pformat = pformat->next);
                    for( pfield = common_instr_field_list;
                            pfield != NULL; pfield = pfield->next) {
                        fprintf(output, "instr_dec->F_%s.%s", pformat->name, pfield->name);
                        if (pfield->next != NULL)
                            fprintf(output, ", ");
                    }
                }
                else {
                    for( pfield = common_instr_field_list;
                            pfield != NULL; pfield = pfield->next){
                        fprintf(output, "ins_cache[%d]", pfield->id);
                        if (pfield->next != NULL)
                            fprintf(output, ", ");
                    }
                }
                fprintf(output, ");\n");
            }
        }
        else
            /* opens case statement */
            fprintf(output, "%scase %d: // Instruction %s\n",
                    INDENT[base_indent], pinstr->id, pinstr->name);

        /* emits format behavior method call */
        for (pformat = format_ins_list;
                (pformat != NULL) && strcmp(pinstr->format, pformat->name);
                pformat = pformat->next);
        fprintf(output, "%sISA._behavior_%s_%s(", INDENT[base_indent + 1],
                project_name, pformat->name);
        for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
            if( ACDecCacheFlag )
                fprintf(output, "instr_dec->F_%s.%s", pformat->name, pfield->name);
            else
                fprintf(output, "ins_cache[%d]", pfield->id);
            if (pfield->next != NULL)
                fprintf(output, ", ");
        }
        fprintf(output, ");\n");

        /* emits instruction behavior method call */
        fprintf(output, "%sISA.behavior_%s(", INDENT[base_indent + 1],
                pinstr->name);
        for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
            if( ACDecCacheFlag )
                fprintf(output, "instr_dec->F_%s.%s", pformat->name, pfield->name);
            else
                fprintf(output, "ins_cache[%d]", pfield->id);
            if (pfield->next != NULL)
                fprintf(output, ", ");
        }
        fprintf(output, ");\n");

        if( ACWaitFlag ) {
          if (pinstr->cycles <= 5)
            fprintf(output, "%sac_qk.inc(time_%dcycle);\n", INDENT[base_indent + 1], pinstr->cycles);
          else
            fprintf(output, "%sac_qk.inc(sc_time(module_period_ns*%d, SC_NS));\n", INDENT[base_indent + 1], pinstr->cycles);
        }

        if( ACThreading )
            fprintf(output, "%sgoto *dispatch();\n\n", INDENT[base_indent + 1]);
        else
            fprintf(output, "%sbreak;\n", INDENT[base_indent]);
    }

    if( !ACThreading ) {
        fprintf(output, "%s} // switch (ins_id)\n", INDENT[base_indent]);

        if( ACStatsFlag ){
            fprintf( output, "%sif(!ac_wait_sig) {\n", INDENT[base_indent]);
            fprintf( output, "%sISA.stats[%s_stat_ids::INSTRUCTIONS]++;\n",
                    INDENT[base_indent+1], project_name);
            fprintf( output, "%s(*(ISA.instr_stats[ins_id]))[%s_instr_stat_ids::COUNT]++;\n",
                    INDENT[base_indent+1], project_name);
            fprintf( output, "%s}\n", INDENT[base_indent]);
        }

    }
}


/**************************************/
/*!  Emits the if statement executed before
  fetches are performed.
  \brief Used by EmitProcessorBhv and EmitDispatch functions */
/***************************************/
void EmitFetchInit( FILE *output, int base_indent){

  if (ACPCAddress) {
    if (!ACDecCacheFlag)
      fprintf( output, "%sif( ac_pc >= %s.get_size()){\n",
              INDENT[base_indent], load_device->name);
    else
      fprintf( output, "%sif( ac_pc >= dec_cache_size){\n",
              INDENT[base_indent]);
    fprintf( output, "%scerr << \"ArchC: Address out of bounds (pc=0x\" << hex << ac_pc << \").\" << endl;\n",
            INDENT[base_indent+1]);
    fprintf( output, "%sstop();\n", INDENT[base_indent+1]);
    if (ACThreading)
      fprintf(output, "%slongjmp(ac_env, AC_ACTION_STOP);\n",
              INDENT[base_indent + 1]);
    else
      fprintf( output, "%sreturn;\n", INDENT[base_indent+1]);
    fprintf( output, "%s}\n", INDENT[base_indent]);
  }
}


/**************************************/
/*!  Emits the body of a processor implementation for
  a processor without pipeline and with single cycle instruction.
  \brief Used by CreateProcessorImpl function      */
/***************************************/
void EmitProcessorBhv( FILE *output, int base_indent ) {
  extern char* project_name;

  fprintf(output, "%sfor (;;) {\n\n", INDENT[base_indent]);
  base_indent++;

  EmitFetchInit(output, base_indent);

  if( ACABIFlag ) {
    if (ACSyscallJump) {
      fprintf( output, "%sbool exec = true;\n\n", INDENT[base_indent]);
      fprintf( output, "%sif (ac_pc < 0x100) {\n", INDENT[base_indent]);
      base_indent++;
    }

    //Emitting system calls handler.
    COMMENT(INDENT[base_indent],"Handling System calls.")
    fprintf( output, "%sswitch( ac_pc ){\n", INDENT[base_indent]);
    base_indent++;
    fprintf( output, "%s#define AC_SYSC(NAME,LOCATION) \\\n",
             INDENT[base_indent]);
    fprintf( output, "%scase LOCATION: \\\n", INDENT[base_indent]);
    base_indent++;

    if( ACStatsFlag ){
      fprintf( output, "%sISA.stats[%s_stat_ids::SYSCALLS]++; \\\n",
              INDENT[base_indent], project_name);
    }

    if( ACDebugFlag ){
      fprintf( output, "%sif( ac_do_trace != 0 )\\\n", INDENT[base_indent]);
      fprintf( output, "%strace_file << hex << ac_pc << dec << endl; \\\n",
              INDENT[base_indent + 1]);
    }

    if( ACHLTraceFlag)
    {
      fprintf( output, "%sgenerate_trace_for_address(ac_pc);\\\n", INDENT[base_indent]);
    }

    fprintf( output, "%sISA.syscall.NAME(); \\\n", INDENT[base_indent]);

    if (ACSyscallJump)
      fprintf( output, "%sexec = false; \\\n", INDENT[base_indent]);

    base_indent--;
    fprintf( output, "%sbreak;\n", INDENT[base_indent]);
    fprintf( output, "%s#include <ac_syscall.def>\n", INDENT[base_indent]);
    fprintf( output, "%s#undef AC_SYSC\n\n", INDENT[base_indent]);


    if (ACSyscallJump) {
      base_indent--;
      fprintf( output, "%s} // switch( ac_pc )\n", INDENT[base_indent]);
      base_indent--;
      fprintf( output, "%s} // if( ac_pc < 0x100 )\n\n", INDENT[base_indent]);
      fprintf( output, "%sif (exec) {\n", INDENT[base_indent]);
      base_indent++;
    }
    else {
      fprintf( output, "%sdefault:\n", INDENT[base_indent]);
      base_indent++;
    }
  }

  if( ACFullDecode ) {
    fprintf( output, "%sinstr_dec = (DEC_CACHE + (ac_pc",
             INDENT[base_indent]);
    if( ACIndexFix )
      fprintf( output, " / %d", largest_format_size / 8);
    fprintf( output, "));\n");
    fprintf( output, "%sins_id = instr_dec->id;\n\n", INDENT[base_indent]);
  }
  else EmitDecodification(output, base_indent);

  EmitInstrExec(output, base_indent);

  if( ACABIFlag ) {
      base_indent--;
    if( ACSyscallJump ) {
      fprintf( output, "%s} // if (exec)\n", INDENT[base_indent]);
    }
    else{
      fprintf( output, "%sbreak;\n", INDENT[base_indent]);
      base_indent--;
      fprintf( output, "%s} // switch( ac_pc )\n", INDENT[base_indent]);
    }
  }

  fprintf( output, "%sac_instr_counter++;\n", INDENT[base_indent]);

  if (ACVerboseFlag) {
    if( ACABIFlag )
      fprintf( output, "%sdone.write(1);\n", INDENT[base_indent]);
    else
      fprintf( output, "%sbhv_done.write(1);\n", INDENT[base_indent]);
  }

  //!Emit update method.
  EmitUpdateMethod( output, base_indent);

  base_indent--;
  fprintf( output, "%s} // for (;;)\n", INDENT[base_indent]);
}


/**************************************/
/*!  Emits a ac_cache instantiation.
  \brief Used by CreateResourcesImpl function      */
/***************************************/
void EmitCacheDeclaration( FILE *output, ac_sto_list* pstorage, int base_indent){

  //Parameter 1 will be the pstorage->name string
  //Parameters passed to the ac_cache constructor.
  //They are not exactly in the same
  //order used for ArchC declarations.
  //TODO: Include write policy
  char parm2[128];  //Block size
  char parm3[128];  //# if blocks
  char parm4[128];  //set  size
  char parm5[128];  //replacement strategy

  //Integer indicating the write-policy
  int wp=0;   /* 0x11 (Write Through, Write Allocate)  */
  /* 0x12 (Write Back, Write Allocate)  */
  /* 0x21 (Write Through, Write Around)  */
  /* 0x22 (Write Back, Write Around)  */

  char *aux;
  int is_dm=0, is_fully=0;
  ac_cache_parms* pparms;
  int i=1;

  for( pparms = pstorage->parms; pparms != NULL; pparms = pparms->next ) {
    switch( i ) {
      case 1: /* First parameter must be a valid associativity */
        if( !pparms->str ){
          AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
          printf("The first parameter must be a valid associativity: \"dm\", \"2w\", \"4w\", ... \n");
          exit(1);
        }
#ifdef DEBUG_STORAGE
      printf("CacheDeclaration: Processing parameter: %d, which is: %s\n", i, pparms->str);
#endif
        if( !strcmp(pparms->str, "dm") || !strcmp(pparms->str, "DM") ){
          //It is a direct-mapped cache
          is_dm = 1;
          //Set size will be the 4th parameter for ac_cache constructor.
          sprintf( parm4, "1");
          //DM caches do not need a replacement strategy,
          //use a default value in this parameter.
          sprintf( parm5, "DEFAULT");
        }
        else if( !strcmp(pparms->str, "fully") ||
                 !strcmp(pparms->str, "FULLY") )
          //It is a fully associative cache
          is_fully =1;
        else {  //It is a n-way cache
          aux = strchr( pparms->str,'w');
          if(  !aux ){   // Checking if the string has a 'w'
            AC_ERROR("Invalid parameter in cache declaration: %s\n",
                     pstorage->name);
            printf("The first parameter must be a valid associativity: \"dm\", \"2w\", \"4w\", ..., \"fully\" \n");
            exit(1);
          }
          aux = (char*) malloc( strlen(pparms->str) );
          strncpy(aux, pparms->str,  strlen(pparms->str)-1);
          aux[ strlen(pparms->str)-1]='\0';
          //Set size will be the 4th parameter for ac_cache constructor.
          sprintf(parm4, "%s", aux);
          free(aux);
        }
        break;

      case 2: /* Second parameter is the number of blocks (lines) */
        if( !(pparms->value > 0 ) ){
          AC_ERROR("Invalid parameter in cache declaration: %s\n",
                   pstorage->name);
          printf("The second parameter must be a valid (>0) number of blocks (lines).\n");
          exit(1);
        }
        if( is_fully )
          //Set size will be the number of blocks (lines) for fully associative caches
          sprintf( parm4, "%d", pparms->value);

        sprintf(parm3, "%d", pparms->value);
        break;

      case 3: /* Third parameter is the block (line) size */
        if( !(pparms->value > 0 ) ){
          AC_ERROR("Invalid parameter in cache declaration: %s\n",
                   pstorage->name);
          printf("The third parameter must be a valid (>0) block (line) size.\n");
          exit(1);
        }
        sprintf(parm2, "%d", pparms->value);
        break;

      case 4: /* The fourth  parameter may be the write policy or the
                replacement strategy. If it is a direct-mapped cache,
                then we don't have a replacement strategy, so this
                parameter must be the write policy, which is "wt"
                (write-through) or "wb" (write-back). Otherwise,
                it must be a replacement strategy, which is "lru"
                or "random", and the fifth parameter will be the
                write policy. */

        if( is_dm ) {
          //This value is set when the first parameter is being processed.
          /* So this is a write-policy */
          if( !strcmp( pparms->str, "wt") || !strcmp( pparms->str, "WT") )
            //One will tell that a wt cache was declared
            wp = WRITE_THROUGH;
          else if( !strcmp( pparms->str, "wb") || !strcmp( pparms->str, "WB") )
            //Zero will tell that a wb cache was declared
            wp = WRITE_BACK;
          else {
            AC_ERROR("Invalid parameter in cache declaration: %s\n",
                     pstorage->name);
            printf("For direct-mapped caches, the fourth parameter must be a valid write policy: \"wt\" or \"wb\".\n");
            exit(1);
          }
        }
        else {
          /* So, this is a replacement strategy */
          if( !strcmp( pparms->str, "lru") || !strcmp( pparms->str, "LRU") ){
            sprintf( parm5, "LRU");  //Including parameter
          }
          else if( !strcmp( pparms->str, "random") ||
                   !strcmp( pparms->str, "RANDOM") )
            sprintf( parm5, "RANDOM");  //Including parameter
          else {
            AC_ERROR("Invalid parameter in cache declaration: %s\n",
                     pstorage->name);
            printf("For non-direct-mapped caches, the fourth parameter must be a valid replacement strategy: \"lru\" or \"random\".\n");
            exit(1);
          }
        }
        break;

      case 5: /* The fifth parameter is a write policy. */
        if( !is_dm ){
          //This value is set when the first parameter is being processed.
          if( !strcmp( pparms->str, "wt") || !strcmp( pparms->str, "WT") )
            wp = WRITE_THROUGH;
          else if( !strcmp( pparms->str, "wb") || !strcmp( pparms->str, "WB") )
            wp = WRITE_BACK;
          else {
            AC_ERROR("Invalid parameter in cache declaration: %s\n",
                     pstorage->name);
            printf("For non-direct-mapped caches, the fourth parameter must be a valid replacement strategy: \"lru\" or \"random\".\n");
            exit(1);
          }
        }
        else {
          //This value is "war" for write-around or "wal" for "write-allocate"
          if( !strcmp( pparms->str, "war") || !strcmp( pparms->str, "WAR") )
            wp = wp | WRITE_AROUND;
          else if( !strcmp( pparms->str, "wal") ||
                   !strcmp( pparms->str, "WAL") )
            wp = wp | WRITE_ALLOCATE;
          else {
            AC_ERROR("Invalid parameter in cache declaration: %s\n",
                     pstorage->name);
            printf("For non-direct-mapped caches, the fourth parameter must be a valid replacement strategy: \"lru\" or \"random\".\n");
            exit(1);
          }
        }
        break;

      case 6: /* The sixth parameter, if it is present, is a write policy.
                It must not be present for direct-mapped caches.*/
        if( !is_dm ) {
          //This value is set when the first parameter is being processed.
          if( !strcmp( pparms->str, "war") || !strcmp( pparms->str, "WAR") )
            wp = wp | WRITE_AROUND;
          else if( !strcmp( pparms->str, "wal") ||
                   !strcmp( pparms->str, "WAL") )
            wp = wp | WRITE_ALLOCATE;
          else {
            AC_ERROR("Invalid parameter in cache declaration: %s\n",
                     pstorage->name);
            printf("For non-direct-mapped caches, the fifth parameter must be  \"war\" or \"wal\".\n");
            exit(1);
          }
        }
        else{
          AC_ERROR("Invalid parameter in cache declaration: %s\n",
                   pstorage->name);
          printf("For direct-mapped caches there must be only five parameters (do not need a replacement strategy).\n");
          exit(1);
        }
        break;

      default:
        break;
    }
    i++;
  }
  //Printing cache declaration.
  fprintf( output, "%sac_cache ac_resources::%s(\"%s\", %s, %s, %s, %s, 0x%x);\n",
           INDENT[base_indent], pstorage->name, pstorage->name,
           parm2, parm3, parm4, parm5, wp);
}


/**************************************/
/*!  Emits a Decoder Cache Structure.
  \brief Used by CreateProcessorHeader function */
/***************************************/
void EmitDecCache(FILE *output, int base_indent) {
  extern ac_dec_format *format_ins_list;

  ac_dec_format *pformat;
  ac_dec_field *pfield;

  for (pformat = format_ins_list; pformat != NULL ; pformat = pformat->next) {
    fprintf(output, "%stypedef struct {\n", INDENT[base_indent]);
    for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
      fprintf(output, "%s", INDENT[base_indent + 1]);
      if (!pfield->sign) fprintf(output, "u");

      if (pfield->size < 9) fprintf(output, "int8_t");
      else if (pfield->size < 17) fprintf(output, "int16_t");
      else if (pfield->size < 33) fprintf(output, "int32_t");
      else fprintf(output, "int64_t");

      fprintf(output, " %s;\n", pfield->name);
    }
    fprintf(output, "%s} T_%s;\n\n", INDENT[base_indent], pformat->name);
  }

  fprintf(output, "%stypedef struct {\n", INDENT[base_indent]);
  if( !ACFullDecode )
    fprintf(output, "%sbool valid;\n", INDENT[base_indent + 1]);
  if (ACThreading)
    fprintf(output, "%svoid* end_rot;\n", INDENT[base_indent + 1]);
  fprintf(output, "%sunsigned id;\n", INDENT[base_indent + 1]);

  fprintf(output, "%sunion {\n", INDENT[base_indent + 1]);
  for (pformat = format_ins_list; pformat != NULL ; pformat = pformat->next) {
    fprintf(output, "%sT_%s F_%s;\n", INDENT[base_indent + 2],
            pformat->name, pformat->name);
  }
  fprintf(output, "%s};\n", INDENT[base_indent + 1]);
  fprintf(output, "%s} DecCacheItem ;\n\n", INDENT[base_indent]);
}


/**************************************/
/*!  Emits a Decoder Cache Attribution.
  \brief Used by EmitDecodification function */
/***************************************/
void EmitDecCacheAt(FILE *output, int base_indent) {
  extern ac_dec_format *format_ins_list;
  ac_dec_format *pformat;
  ac_dec_field *pfield;

  fprintf(output, "%sswitch (ISA.instr_format_table[instr_dec->id]) {\n",
          INDENT[base_indent]);
  for (pformat = format_ins_list; pformat != NULL ; pformat = pformat->next) {
    fprintf(output, "%scase %d:\n", INDENT[base_indent + 1], pformat->id);
    for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
      fprintf(output, "%sinstr_dec->F_%s.%s = ins_cache[%d];\n",
              INDENT[base_indent + 2], pformat->name,
              pfield->name, pfield->id);
    fprintf(output, "%sbreak;\n", INDENT[base_indent + 1]);
  }

  if( !ACFullDecode ) {
    fprintf(output, "%sdefault:\n", INDENT[base_indent + 1]);
    fprintf(output, "%scerr << \"ArchC Error: Unidentified instruction. \" << endl;\n",
            INDENT[base_indent + 2]);
    fprintf(output, "%scerr << \"PC = \" << hex << ac_pc << dec << endl;\n",
            INDENT[base_indent + 2]);
    fprintf(output, "%sstop();\n", INDENT[base_indent + 2]);

    if (ACThreading)
      fprintf(output, "%slongjmp(ac_env, AC_ACTION_STOP);\n",
              INDENT[base_indent + 2]);
    else
      fprintf(output, "%sreturn;\n", INDENT[base_indent + 2]);
  }

  fprintf(output, "%s}\n", INDENT[base_indent]);
}

/**************************************/
/*!  Emits the Dispatch Function used by Threading
  \brief Used by CreateProcessorImpl function */
/***************************************/
void EmitDispatch(FILE *output, int base_indent) {

  fprintf( output, "%svoid* %s::dispatch() {\n",
           INDENT[base_indent], project_name);

  base_indent++;


 if (HaveTLMIntrPorts || HaveTLM2IntrPorts)
  {
    fprintf(output, "%s/*************************************************************************************/\n",INDENT[base_indent]);
    fprintf(output, "%s/* SLEEP / AWAKE mode control                                                        */\n",INDENT[base_indent]);
    fprintf(output, "%s/* intr_reg may store 1 (AWAKE MODE) or 0 (SLEEP MODE) - default is AWAKE            */\n",INDENT[base_indent]);
    fprintf(output, "%s/* if intr_reg == 0, the simulator will be suspended until it happens the wake event */\n",INDENT[base_indent]);
    fprintf(output, "%s/* wake - this event will happen in the moment the processor receives and            */\n",INDENT[base_indent]);
    fprintf(output, "%s/* interrupt with code AWAKE (1)                                                     */\n",INDENT[base_indent]);
    fprintf(output, "%s/*************************************************************************************/\n",INDENT[base_indent]);
    fprintf(output, "%sif (intr_reg.read() == 0)  wait(wake);\n",INDENT[base_indent]);
  }


  if( ACDebugFlag ){
    fprintf( output, "%sextern bool ac_do_trace;\n", INDENT[base_indent]);
    fprintf( output, "%sextern ofstream trace_file;\n", INDENT[base_indent]);
  }

  //!Emit update method.
  EmitUpdateMethod( output, base_indent);

  EmitFetchInit(output, base_indent);

  fprintf( output, "%sac_instr_counter++;\n", INDENT[base_indent]);
  fprintf( output, "%sunsigned ins_id;\n", INDENT[base_indent]);





  if( ACABIFlag && !ACDecCacheFlag) {
    if (ACSyscallJump) {
      fprintf( output, "%sbool exec = true;\n\n", INDENT[base_indent]);
      fprintf( output, "%sif (ac_pc < 0x100) {\n", INDENT[base_indent]);
      base_indent++;
    }

    //Emitting system calls handler.
    COMMENT(INDENT[base_indent],"Handling System calls.")
    fprintf( output, "%sswitch( ac_pc ){\n", INDENT[base_indent]);
    base_indent++;
    fprintf( output, "%s#define AC_SYSC(NAME,LOCATION) \\\n",
             INDENT[base_indent]);
    fprintf( output, "%scase LOCATION: \\\n", INDENT[base_indent]);
    base_indent++;

    if( ACStatsFlag ){
      fprintf( output, "%sISA.stats[%s_stat_ids::SYSCALLS]++; \\\n",
              INDENT[base_indent], project_name);
    }

    if( ACDebugFlag ){
      fprintf( output, "%sif( ac_do_trace != 0 )\\\n", INDENT[base_indent]);
      fprintf( output, "%strace_file << hex << ac_pc << dec << endl; \\\n",
              INDENT[base_indent + 1]);
    }
    if( ACHLTraceFlag)
    {
      fprintf( output, "%sgenerate_trace_for_address(ac_pc);\\\n", INDENT[base_indent]);
    }

    fprintf( output, "%sISA.syscall.NAME(); \\\n", INDENT[base_indent]);

    if (ACSyscallJump)
      fprintf( output, "%sexec = false; \\\n", INDENT[base_indent]);

    if (ACThreading)
      fprintf( output, "%sreturn IntRoutine[0]; \\\n", INDENT[base_indent]);

    base_indent--;
    fprintf( output, "%sbreak;\n", INDENT[base_indent]);
    fprintf( output, "%s#include <ac_syscall.def>\n", INDENT[base_indent]);
    fprintf( output, "%s#undef AC_SYSC\n\n", INDENT[base_indent]);

    if (ACSyscallJump) {
      base_indent--;
      fprintf( output, "%s} // switch( ac_pc )\n",
               INDENT[base_indent]);
      base_indent--;
      fprintf( output, "%s} // if( ac_pc < 0x100 )\n\n",
               INDENT[base_indent]);
      fprintf( output, "%sif (exec) {\n", INDENT[base_indent]);
      base_indent++;
    }
    else {
      fprintf( output, "%sdefault:\n", INDENT[base_indent]);
      base_indent++;
    }
  }

  if( ACFullDecode ) {
    fprintf( output, "%sinstr_dec = (DEC_CACHE + (ac_pc",
             INDENT[base_indent]);
    if( ACIndexFix )
      fprintf( output, " / %d", largest_format_size / 8);
    fprintf( output, "));\n");
    fprintf( output, "%sins_id = instr_dec->id;\n\n", INDENT[base_indent]);
  }
  else EmitDecodification(output, base_indent);

  EmitInstrExecIni(output, base_indent);

  if( ACStatsFlag ){
    fprintf( output, "%sif(!ac_wait_sig && ins_id) {\n", INDENT[base_indent]);
    fprintf( output, "%sISA.stats[%s_stat_ids::INSTRUCTIONS]++;\n",
            INDENT[base_indent + 1], project_name);
    fprintf( output, "%s(*(ISA.instr_stats[ins_id]))[%s_instr_stat_ids::COUNT]++;\n",
            INDENT[base_indent + 1], project_name);
    fprintf( output, "%s}\n", INDENT[base_indent]);
  }

  if( ACDebugFlag ){
    fprintf( output, "%sif( ac_do_trace != 0 ) \n", INDENT[base_indent]);
    fprintf( output, PRINT_TRACE, INDENT[base_indent + 1]);
  }
  if( ACHLTraceFlag)
  {
    fprintf( output, "%sgenerate_trace_for_address(ac_pc);\\\n", INDENT[base_indent]);
  }

  if( ACABIFlag && !ACDecCacheFlag ) {
      base_indent--;
    if( ACSyscallJump ) {
      fprintf( output, "%s} // if (exec)\n", INDENT[base_indent]);
    }
    else{
      fprintf( output, "%sbreak;\n", INDENT[base_indent]);
      base_indent--;
      fprintf( output, "%s} // switch( ac_pc )\n", INDENT[base_indent]);
    }
  }

  if (ACVerboseFlag) {
    if( ACABIFlag )
      fprintf( output, "%sdone.write(1);\n", INDENT[base_indent]);
    else
      fprintf( output, "%sbhv_done.write(1);\n", INDENT[base_indent]);
  }

// POWER ESTIMATION

  if (ACPowerEnable) {
    fprintf(output, "\n\n#ifdef POWER_SIM\n");
    fprintf(output, "ps.update_stat_power(ins_id);\n");
    fprintf(output, "#endif\n\n");
  }


  if(ACDecCacheFlag)
    fprintf( output, "%sreturn instr_dec->end_rot;\n", INDENT[base_indent]);
  else
    fprintf( output, "%sreturn IntRoutine[ins_id];\n", INDENT[base_indent]);

  base_indent--;
  fprintf( output, "%s}\n\n", INDENT[base_indent]);
}


/**************************************/
/*!  Emits the Vector with Address of the
 * Interpretation Routines used by Threading
  \brief Used by CreateProcessorImpl function */
/***************************************/
void EmitVetLabelAt(FILE *output, int base_indent) {
  extern ac_dec_instr *instr_list;
  ac_dec_instr *pinstr;
  unsigned cont = 0;

  fprintf( output, "%svoid* vet[] = {&&I_Init", INDENT[base_indent]);
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    fprintf(output, ", ");
    if (cont++ >= 4) {
      fprintf(output, "\n%s", INDENT[base_indent + 6]);
      cont = 0;
    }
    fprintf(output, "&&I_%s", pinstr->name);
  }
  fprintf(output, "};\n\n");

  fprintf(output, "%sIntRoutine = vet;\n\n", INDENT[base_indent]);
}


////////////////////////////////////
// Utility Functions              //
////////////////////////////////////

#include <unistd.h>

//!Read the archc.conf configuration file
void ReadConfFile(){

  char *user_homedir;
  char *conf_filename_local;
  char *conf_filename_global;

  extern char *CC_PATH;
  extern char *OPT_FLAGS;
  extern char *DEBUG_FLAGS;
  extern char *OTHER_FLAGS;

  FILE *conf_file;
  char line[CONF_MAX_LINE];
  char var[CONF_MAX_LINE];
  char value[CONF_MAX_LINE];

  user_homedir = getenv("HOME");
  conf_filename_local = malloc(strlen(user_homedir) + 19);
  strcpy(conf_filename_local, user_homedir);
  strcat(conf_filename_local, "/.archc/archc.conf");

  conf_filename_global = malloc(strlen(SYSCONFDIR) + 12);
  strcpy(conf_filename_global, SYSCONFDIR);
  strcat(conf_filename_global, "/archc.conf");

  // Try to get a local file in $HOME/.archc
  conf_file = fopen(conf_filename_local, "r");

  // Try to get a global file, based on PREFIX instalation path
  if (!conf_file)
    conf_file = fopen(conf_filename_global, "r");

  // Try get a relative conf based on "acsim" executable path
  if (!conf_file) {
      char buf[512];
      int len = readlink("/proc/self/exe", buf, 512-1);
      while (buf[--len] != '/');  // find the path without exec name
      while (buf[--len] != '/');  // find the path without /bin
      buf[len] = '\0';
      strcat(buf,"/etc/archc.conf");
      conf_file = fopen(buf,"r");
  }

  if( !conf_file ){
    AC_ERROR("Could not open archc.conf configuration file.\n");
    exit(1);
  }
  else{
    while( fgets( line, CONF_MAX_LINE, conf_file) ){

      var[0]='\0';
      value[0]='\0';

      if(line[0] == '#' || line[0] == '\n'){  //Comments or blank lines
        continue;
      }
      else{

        sscanf(line, "%s",var);
        strcpy( value, strchr(line, '=')+1);

        if( !strcmp(var, "CC") ){
          CC_PATH =  (char*) malloc(strlen(value)+1);
          CC_PATH = strcpy(CC_PATH, value);
        }
        else if( !strcmp(var, "OPT") ){
          OPT_FLAGS =(char*) malloc(strlen(value)+1);
          OPT_FLAGS = strcpy(OPT_FLAGS, value);
        }
        else if( !strcmp(var, "DEBUG") ){
          DEBUG_FLAGS = (char*) malloc(strlen(value)+1);
          DEBUG_FLAGS = strcpy(DEBUG_FLAGS, value);
        }
        else if( !strcmp(var, "OTHER") ){
          OTHER_FLAGS = (char*) malloc(strlen(value)+1);
          OTHER_FLAGS = strcpy(OTHER_FLAGS, value);
        }
      }
    }
  }

  fclose(conf_file);

  free(conf_filename_local);
  free(conf_filename_global);
}

void ParseCache(ac_sto_list *cache_in) {
    ac_cache_parms *p;
    bool fully_associative = false;
    extern char *project_name;
    struct CacheObject *cache_out;
    if (cache_in->cache_object != NULL)
        free(cache_in->cache_object);

    // Let the OS free this in the end
    cache_in->cache_object = malloc(sizeof(struct CacheObject));
    cache_out = cache_in->cache_object;

    // 1st parameter
    p = cache_in->parms;

    if (!strcmp(p->str, "dm") || !strcmp(p->str, "DM")) {
        cache_out->associativity = 1;
    } else if (!strcmp(p->str, "fully") || !strcmp(p->str, "FULLY")) {
        fully_associative = true;
    } else if (sscanf(p->str, " %d", &cache_out->associativity) <= 0 ||
               cache_out->associativity < 1) {
        AC_ERROR("Invalid parameter in cache declaration: %s\n",
                 cache_in->name);
        printf("The first parameter must be a valid associativity: \"dm\","
               " \"2w\", \"4w\", ..., \"fully\" \n");
        exit(EXIT_FAILURE);
    }

    // 2nd parameter
    p = p->next;
    if (fully_associative) {
        cache_out->associativity = p->value;
    }

    cache_out->block_count = p->value;

    // 3rd parameter
    p = p->next;
    cache_out->block_size = p->value;

    // 4th parameter
    p = p->next;
    if (!strcmp(p->str, "wt") || !strcmp(p->str, "WT")) {
        cache_out->type = WriteThrough;
    } else if (!strcmp(p->str, "wb") || !strcmp(p->str, "WB")) {
        cache_out->type = WriteBack;
    } else {
        AC_ERROR("Invalid parameter in cache declaration: %s\n",
                 cache_in->name);
        printf("The fourth parameter must be a valid write policy: \"wt\" or "
               "\"wb\".\n");
    }

    // 5th parameter
    p = p->next;

    // if (p == NULL)
    if (cache_out->associativity == 1) {
        if (strcmp(p->str, "none") && strcmp(p->str, "NONE")) {
            printf("The replacement policy will be ignored because it is a "
                   "direct-mapped cache\n");
        }
        cache_out->replacement_policy = None;

    } else {
        if (!strcmp(p->str, "plrum") || !strcmp(p->str, "PLRUM")) {

            cache_out->replacement_policy = PLRUM;
        } else if (!strcmp(p->str, "random") || !strcmp(p->str, "RANDOM")) {
            cache_out->replacement_policy = Random;
        } else if (!strcmp(p->str, "fifo") || !strcmp(p->str, "FIFO")) {
            cache_out->replacement_policy = FIFO;
        } else if (!strcmp(p->str, "lru") || !strcmp(p->str, "LRU")) {
            cache_out->replacement_policy = LRU;
        } else {
            AC_ERROR("Invalid parameter in cache declaration: %s\n",
                     cache_in->name);
            printf("The fifth parameter must be a valid replacement strategy:"
                   "\"plrum\", \"random\", \"fifo\" or \"lru\" (or \"none\" "
                   "for direct-mapped caches.\")\n");
            exit(EXIT_FAILURE);
        }
    }
}

void TLMMemoryClassDeclaration(ac_sto_list * memory)
{
    extern char *project_name;
    const unsigned s = 70;
    //unsigned i = memory->size * 8 / wordsize;
    memory->class_declaration = malloc(s);
    if (snprintf(memory->class_declaration, s, "ac_memport<%s_parms::ac_word, %s_parms::ac_Hword>", project_name, project_name) >= s)
  abort();
}

/*void TLMCacheMemoryClassDeclaration(ac_sto_list * memory)
{
    extern char *project_name;
    const unsigned s = 70;
    //unsigned i = memory->size * 8 / wordsize;
    memory->class_declaration = malloc(s);


    if (snprintf(memory->class_declaration, s, "ac_cache_memport<%s_parms::ac_word, %s_parms::ac_Hword>", project_name, project_name) >= s)
  abort();
}*/


void CacheClassDeclaration(ac_sto_list * storage)
{
    const unsigned s = 800;
    extern char *project_name;
    struct CacheObject *cache = storage->cache_object;
    if (storage->higher->class_declaration == NULL) {
        if (storage->higher->type == MEM || storage->higher->type == TLM_PORT ||
            storage->higher->type == TLM2_PORT ||
            storage->higher->type == TLM2_NB_PORT) {
            /*****/
            TLMMemoryClassDeclaration(storage->higher);
        } else {
            ParseCache(storage->higher);
            CacheClassDeclaration(storage->higher);
        }
    }
    storage->class_declaration = malloc(s);

    // FIXME: Can I set "ac_memport" directly here instead of
    // cache->higher->class_declaration?
    int r = snprintf(
        storage->class_declaration, s, "%s<%d, %d, %d, %s_parms::ac_word, "
                                       "ac_memport<%s_parms::ac_word, "
                                       "%s_parms::ac_Hword>, %s>",
        CacheName[cache->type], cache->block_count / cache->associativity,
        cache->block_size, cache->associativity, project_name, project_name,
        project_name, ReplacementPolicyName[cache->replacement_policy]);
    if (r >= s)
        abort();
}

void EnumerateCaches() {
    extern ac_sto_list *storage_list;
    ac_sto_list *i;
    for (i = storage_list; i != NULL; i = i->next) {
        // printf("\nprinting i->type = %d", i->type);
        if (i->type == ICACHE || i->type == DCACHE || i->type == CACHE) {
            if (i->parms != NULL) {
                ParseCache(i);
                CacheClassDeclaration(i);
            }
        }
    }
}

void GetFetchDevice()
{
    extern ac_sto_list *fetch_device, *storage_list;
    ac_sto_list *pstorage;
    int lowest_level = 10;

    /* Determining which device is gonna be used for fetching instructions */
    if (!fetch_device) {
        //The parser has not determined because there is not an ac_icache obj declared.
        //In this case, look for the object with the lowest hierarchy level.
        for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
            if ( pstorage->type == MEM || pstorage->type == ICACHE || pstorage->is_tlm ) {
                if ( pstorage->level < lowest_level ) {
                   fetch_device = pstorage;
                   lowest_level = pstorage->level;
                }
            }

        if (!fetch_device) {  //Couldn't find a fetch device. Error!
            AC_INTERNAL_ERROR("Could not determine a device for fetching.");
            exit(EXIT_FAILURE);
        }
    }
}

void GetLoadDevice()
{
    extern ac_sto_list *storage_list, *fetch_device;
    ac_sto_list *pstorage;
    load_device = storage_list;

    /* Determining which device is going to be used for loading applications */
    /* The device used for loading applications must be the one in the highest
       level of a memory hierarchy. */
    for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next) {
        if (pstorage->level > load_device->level)
            load_device = pstorage;
    }

    /* If there is only one level, which is gonna be zero, then it is the same
       object used for fetching. */
    if (load_device->level == 0)
        load_device = fetch_device;
}


void GetFirstLevelDataDevice()
{
    extern ac_sto_list *first_level_data_device, *storage_list;
    ac_sto_list *pstorage;
    int lowest_level = 10;

    /* Determining which device is gonna be used for data access */
    if (!first_level_data_device) {
        //The parser has not determined because there is not an ac_dcache obj declared.
        //In this case, look for the object with the lowest hierarchy level.
        for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next) {
            if ( pstorage->type == MEM || pstorage->type == DCACHE || pstorage->is_tlm ) {
                if ( pstorage->level < lowest_level ) {
                   first_level_data_device = pstorage;
                   lowest_level = pstorage->level;
                }
            }
        }

        if (!first_level_data_device) {  //Couldn't find a data device. Error!
            AC_INTERNAL_ERROR("Could not determine a device for data access.");
            exit(EXIT_FAILURE);
        }
    }
}
