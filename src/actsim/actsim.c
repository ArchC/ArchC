/* ex: set tabstop=2 expandtab:
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      actsim.c
 * @author    Sandro Rigo
 *            Marcus Bartholomeu
 *            Alexandro Baldassin
 *            Thiago Sigrist
 *            Marilia Chiozo
 *
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Tue, 06 Jun 2006 18:05:35 -0300
 *
 * @brief     The ArchC timed simulator generator
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

//////////////////////////////////////////////////////////
/*!\file actsim.c
  \brief The ArchC pre-processor.

  This file contains functions to control the ArchC
  to emit the source files that compose the timed
  simulator.
*/
//////////////////////////////////////////////////////////

#include "actsim.h"
#include "stdlib.h"
#include "ctype.h"
#include "string.h"

//#define DEBUG_STORAGE

// Consts; these values can be changed to suit users' needs.
const char* isa_header_description = "Instruction Set Architecture header file.";
const char* isa_template_description = "Behavior implementation file template.";
const char* fregs_template_description = "Formatted registers behavior implementation file.";
const char* main_filename = "main.cpp.tmpl";
const char* make_filename = "Makefile.archc";
const char* proc_module_header_description = "Processor module header file.";
const char* proc_module_impl_description = "Processor module implementation file.";

// Const strings; these might be replaced by variables in the future.
const char* field_type = "int"; //!< A data type which can hold instruction fields
// DO NOT set pc_type to ac_word. BAD THINGS will happen. --Marilia
const char* ptr_type = "unsigned long"; //!< A data type which can hold pointers
const char* pc_type = "unsigned"; //!< The data type representing the value of ac_pc
// This is used to initialize flushes_left in the stage implementation.
// Think of a better way to find out this value. --Marilia
const unsigned pipe_maximum_path_length = 7;

// Defining traces and dasm strings
#define PRINT_TRACE "%strace_file << hex << ap.decode_pc << dec << \"\\n\";\n"
#define PRINT_DASM "%sdasmfile << hex << ap.decode_pc << dec << \": \" << \"(\" << isa.instructions[1 + ins_id].name << \",\" << %s_parms::%s_isa::instructions[1 + ins_id].format << \")\" << endl;\n"

// Command-line options flags
int ACABIFlag = 0;                              //!< Indicates whether an ABI was provided or not
int ACDasmFlag = 0;                             //!< Indicates whether disassembler option is turned on or not
int ACDebugFlag = 0;                            //!< Indicates whether debugger option is turned on or not
int ACDecCacheFlag = 1;                         //!< Indicates whether the simulator will cache decoded instructions or not
int ACDelayFlag = 0;                            //!< Indicates whether delay option is turned on or not
int ACDDecoderFlag = 0;                         //!< Indicates whether decoder structures are dumped or not
//int ACQuietFlag = 0;                            //!< Indicates whether storage update logs are displayed during simulation or not
int ACStatsFlag = 0;                            //!< Indicates whether statistics collection is enable or not
int ACVerboseFlag = 0;                          //!< Indicates whether verbose option is turned on or not
int ACVerifyFlag = 0;                           //!< Indicates whether verification option is turned on or not
int ACVerifyTimedFlag = 0;                      //!< Indicates whether verification option is turned on for a timed behavioral model
int ACEncoderFlag = 0;                          //!< Indicates whether encoder tools will be included in the simulator
int ACGDBIntegrationFlag = 0;                   //!< Indicates whether gdb support will be included in the simulator

char* ACVersion = ACVERSION;                    //!< Stores ArchC version number.
char ACOptions[500];                            //!< Stores ArchC recognized command line options
char* ACOptions_p = ACOptions;                  //!< Pointer used to append options in ACOptions
char* arch_filename;                            //!< Stores ArchC arquitecture file
char* caps_project_name;                        //!< Holds the all-caps version of the project name, for header files.
ac_stg_list* fetch_stage = NULL;                //!< Pointer to the fetch stage.

int ac_host_endian;                             //!< Indicates the endianness of the host machine
extern int ac_tgt_endian;                       //!< Indicates the endianness of the host machine
int ac_match_endian;                            //!< Indicates whether host and target endianness match on or not

//! This structure describes one command-line option mapping.
/*!  It is used to manage command line options, following gcc style. */
struct option_map
{
 const char* name;                          //!< The long option's name.
 const char* equivalent;                    //!< The equivalent short-name for options.
 const char* arg_desc;                      //!< The option description.
 /* Argument info.  A string of flag chars; NULL equals no options.
    r => argument required.
    o => argument optional.
    * => require other text after NAME as an argument.  */
 const char* arg_info;                      //!< Argument info.  A string of flag chars; NULL equals no options.
};

/*! Decoder object pointer */
ac_decoder_full* decoder;

/*! Storage device used for loading applications */
ac_sto_list* load_device = 0;

/*! This is the table of mappings.  Mappings are tried sequentially
  for each option encountered; the first one that matches, wins.  */
struct option_map option_map[] =
{
 {"--abi-included"  , "-abi"        , "Indicate that an ABI for system call emulation was provided." ,"o"},
 {"--disassembler"  , "-dasm"       , "Dump executing instructions in assembly format (Not completely implemented)." ,"o"},
 {"--debug"         , "-g"          , "Enable simulation debug features: traces, update logs." ,"o"},
#if 0
 {"--delay"         , "-dy"         , "Enable delayed assignments to storage elements. No longer supported." ,"o"},
#endif
 {"--dumpdecoder"   , "-dd"         , "Dump the decoder data structure." ,"o"},
 {"--help"          , "-h"          , "Display this help message."       , 0},
#if 0
 {"--quiet"         , "-q"          , ".", "o"},
#endif
 {"--no-dec-cache"  , "-ndc"        , "Disable cache of decoded instructions." ,"o"},
 {"--stats"         , "-s"          , "Enable statistics collection during simulation." ,"o"},
 {"--verbose"       , "-vb"         , "Display update logs for storage devices during simulation.", "o"},
#if 0 // Co-verification is currently unmaintained. --Marilia
 {"--verify"        , "-v"          , "Enable co-verification mechanism." ,"o"},
 {"--verify-timed"  , "-vt"         , "Enable co-verification mechanism. Timed model." ,"o"},
#endif
 {"--version"       , "-vrs"        , "Display ACTSIM version.", 0},
#if 0 // I don't know whatever happened to the encoder tools. --Marilia
 {"--encoder"       , "-enc"        , "Use encoder tools with the simulator (beta version).", 0},
#endif
 {"--gdb-integration", "-gdb"       , "Enable support for debbuging programs running on the simulator.", 0},
 0
};

/*! Display the command line options accepted by ArchC. */
static void DisplayHelp()
{
 int i;
 char line[] = "====================";

 line[strlen(ACVersion) + 1] = '\0';
 printf ("=====================================================%s\n", line);
 printf (" This is the ArchC Timed Simulator Generator version %s\n", ACVersion);
 printf ("=====================================================%s\n\n", line);
 printf ("Usage: actsim input_file [options]\n");
 printf ("       Where input_file stands for your AC_ARCH description file.\n\n");
 printf ("Options:\n");
 for (i = 0; i < ACNumberOfOptions; i++)
  printf("    %-17s, %-11s %s\n", option_map[i].name, option_map[i].equivalent,
         option_map[i].arg_desc);
 printf("\nFor more information please visit www.archc.org\n\n");
 return;
}

/*! Function for decoder that updates the instruction buffer pointer,
  the bytes quantity available and the old relative index. */
/* PARSER-TIME VERSION: Should not be called. */
int ExpandInstrBuffer(int index)
{
 AC_ERROR("ExpandInstrBuffer(index=%d): This function should not be called in parser-time for interpreted simulator.\n", index);
 return (index + 1);
}

/*! Function for decoder to get bits from the instruction. */
/*    PARSER-TIME VERSION: Should not be called. */
unsigned long long GetBits(void* buffer, int* quant, int last, int quantity, int sign)
{
 AC_ERROR("GetBits(): This function should not be called in parser-time for interpreted simulator.\n");
 return 1;
}

/**********************************************************/
/*!Writes a standard comment at the begining of each file
   generated by ArchC.
   OBS: Description must have 50 characteres at most!!
  \param output The output file pointer.
  \param description A brief description of the file being emited.*/
/**********************************************************/
void print_comment(FILE* output, const char* description)
{
 fprintf(output, "/******************************************************\n");
 fprintf(output, " * %-50s *\n", description);
 fprintf(output, " * This file is automatically generated by ArchC      *\n");
 fprintf(output, " * WITHOUT WARRANTY OF ANY KIND, either express       *\n");
 fprintf(output, " * or implied.                                        *\n");
 fprintf(output, " * For more information on ArchC, please visit:       *\n");
 fprintf(output, " * http://www.archc.org                               *\n");
 fprintf(output, " *                                                    *\n");
 fprintf(output, " * The ArchC Team                                     *\n");
 fprintf(output, " * Computer Systems Laboratory (LSC)                  *\n");
 fprintf(output, " * IC-UNICAMP                                         *\n");
 fprintf(output, " * http://www.lsc.ic.unicamp.br                       *\n");
 fprintf(output, " ******************************************************/\n");
 fprintf(output, "\n");
 return;
}

///////////////////////////////////////////
/*! Main routine of ArchC pre-processor. */
///////////////////////////////////////////
int main(int argc, char** argv)
{
 extern char* project_name;
 extern char* isa_filename;
 extern int wordsize;
 extern int fetchsize;
 // Structures to be passed to the decoder generator.
 extern ac_dec_format* format_ins_list;
 extern ac_dec_instr* instr_list;
 // Structures to be passed to the simulator generator.
 extern ac_pipe_list* pipe_list;
 extern ac_stg_list* stage_list; // Just for warning purposes.
 extern int HaveFormattedRegs, HaveMultiCycleIns, HaveTLMIntrPorts;
 extern ac_decoder_full* decoder;
 // Uncomment the line bellow if you want to debug the parser.
 //extern int yydebug;
 ac_pipe_list* ppipe;
 ac_stg_list* pstage;
 int argn, i, j;
 endian a, b;
 int error_flag = 0;
 char* dummy;

 // Uncomment the line bellow if you want to debug the parser.
 //yydebug = 1;
 // Initializes the pre-processor.
 acppInit(0);
 ++argv, --argc; // Skip over program name.
 // First argument must be the file or --help option.
 if (argc > 0)
 {
  if(!strcmp(argv[0], "--help") || !strcmp(argv[0], "-h"))
  {
   DisplayHelp();
   return 0;
  }
  if (!strcmp(argv[0], "--version") || !strcmp(argv[0], "-vrs"))
  {
   printf("This is ArchC version %s\n", ACVersion);
   return 0;
  }
  else
  {
   if (!acppLoad(argv[0]))
   {
    AC_ERROR("Invalid input file: %s\n", argv[0]);
    printf("   Try actsim --help for more information.\n");
    return EXIT_FAILURE;
   }
   arch_filename = argv[0];
  }
 }
 else
 {
  AC_ERROR("No input file provided.\n");
  printf("   Try actsim --help for more information.\n");
  return EXIT_FAILURE;
 }
 ++argv, --argc; // Skip over arch file name.
 if (argc > 0)
 {
  argn = argc;
  // Handling command line options.
  for (j = 0; j < argn; j++)
  {
   // Searching option map.
   for (i = 0; i < ACNumberOfOptions; i++)
   {
    if (!strcmp(argv[0], option_map[i].name) ||
        !strcmp(argv[0], option_map[i].equivalent))
    {
     switch (i)
     {
      case OPABI:
       ACABIFlag = 1;
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
       break;
      case OPDasm:
       ACDasmFlag = 1;
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
       break;
      case OPDebug:
       ACDebugFlag = 1;
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
       break;
      case OPDDecoder:
       ACDDecoderFlag = 1;
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
       break;
#if 0 // From actsim.h, this isn't available, though I have no idea why. --Marilia
      case OPQuiet:
       ACQuietFlag = 1;
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
       break;
#endif
      case OPDecCache:
       ACDecCacheFlag = 0;
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
       break;
      case OPStats:
       ACStatsFlag = 1;
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
       break;
      case OPVerbose:
       ACVerboseFlag = 1;
       AC_MSG("Simulator running on verbose mode.\n");
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
       break;
#if 0 // Co-verification is currently unmaintained. --Marilia
      case OPVerify:
       ACVerifyFlag = 1;
       AC_MSG("Simulator running on co-verification mode. Use it ONLY along with the ac_verifier tool.\n");
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
       break;
      case OPVerifyTimed:
       ACVerifyFlag = ACVerifyTimedFlag = 1;
       AC_MSG("Co-verification is turned on, running on timed mode.\n");
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
       break;
#endif
#if 0 // I don't know whatever happened to the encoder tools. --Marilia
      case OPEncoder:
       ACEncoderFlag = 1;
       AC_MSG("Simulator will have encoder extra tools (beta version).\n");
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
       break;
#endif
#if 0 // GDB integration is currently not supported for the timed simulator. --Marilia
      case OPGDBIntegration:
       ACGDBIntegrationFlag = 1;
       ACOptions_p += sprintf(ACOptions_p, "%s ", argv[0]);
#endif
      default:
       break;
     }
     ++argv, --argc; // Skip over found argument.
     break;
    }
   }
  }
 }
 if (argc > 0)
 {
  AC_ERROR("Invalid argument %s.\n", argv[0]);
  return EXIT_FAILURE;
 }
 // Loading configuration variables.
 ReadConfFile();
 // Parsing Architecture declaration file.
 AC_MSG("Parsing AC_ARCH declaration file: %s\n", arch_filename);
 if (acppRun())
 {
  AC_ERROR("Parser terminated unsuccessfully.\n");
  error_flag = 1;
  return EXIT_FAILURE;
 }
 acppUnload();
 // Opening ISA file.
 if (isa_filename == NULL)
  AC_ERROR("No ISA file defined");
 if (!acppLoad(isa_filename))
 {
  AC_ERROR("Could not open ISA input file: %s\n", isa_filename);
  AC_ERROR("Parser terminated unsuccessfully.\n");
  return EXIT_FAILURE;
 }
 // Parsing ArchC ISA declaration file.
 AC_MSG("Parsing AC_ISA declaration file: %s\n", isa_filename);
 if (acppRun())
 {
  AC_ERROR("Parser terminated unsuccessfully.\n");
  error_flag = 1;
 }
 acppUnload();
 if (error_flag)
  return EXIT_FAILURE;
 else
 {
  if (pipe_list)
   if (pipe_list->next)
   {
    AC_ERROR("actsim does not yet handle multiple pipelines.\n");
    return EXIT_FAILURE;
   }
  if (!pipe_list && !HaveMultiCycleIns)
  {
   AC_ERROR("No pipeline and no multicycle instructions: please use acsim.\n");
   return EXIT_FAILURE;
  }
  if (wordsize == 0)
  {
   AC_MSG("Warning: No wordsize defined. Default value is 32 bits.\n");
   wordsize = 32;
  }
  if (fetchsize == 0)
  {
   if (pipe_list)
    if (pipe_list->next)
    {
     AC_ERROR("Multiple pipelines require explicit definition of fetchsize.\n");
     return EXIT_FAILURE;
    }
   //AC_MSG("Warning: No fetchsize defined. Default is to be equal to wordsize (%d).\n", wordsize);
   fetchsize = wordsize;
  }
  // Testing host endianness.
  a.i = 255;
  b.i = 0;
  b.c[(sizeof(int) / sizeof(char)) - 1] = 255;
  if (a.i == b.i)
   ac_host_endian = 1; // Host machine is big endian.
    else
   ac_host_endian = 0; // Host machine is little endian.
  ac_match_endian = (ac_host_endian == ac_tgt_endian);
  // Creating decoder to get field number info and write its static version.
  // This object will be used by some Create functions.
  if (ACDDecoderFlag)
  {
   AC_MSG("Dumping decoder structures:\n");
   ShowDecInstr(instr_list);
   ShowDecFormat(format_ins_list);
   printf("\n\n");
  }
  decoder = CreateDecoder(format_ins_list, instr_list);
  if (ACDDecoderFlag)
   ShowDecoder(decoder->decoder, 0);
  // For practicity, an upper-case version of the project name is prepared here. --Marilia
  caps_project_name = malloc(sizeof(char) * (1 + strlen(project_name)));
  i = 0;
  while (project_name[i])
  {
   caps_project_name[i] = toupper(project_name[i]);
   i++;
  }
  caps_project_name[i] = '\0';
  // Internally, ac_stage is already converted to ac_pipe by the parser. --Marilia
  if (stage_list)
   AC_MSG("The ac_stage keyword is deprecated. Using ac_pipe is preferred.\n");
  // Creating arch-specific resources header file.
  CreateArchHeader();
  // Creating arch-specific references header file.
  CreateArchRefHeader();
  // Creating arch-specific references impl file.
  CreateArchRefImpl();
  // Creating ISA header file.
  CreateISAHeader();
  // Creating ISA initialization file.
  CreateISAInitImpl();
  // Creating parameters header file.
  CreateParmHeader();
  // Creating processor header file.
  CreateProcessorHeader();
  // Creating processor impl file.
  CreateProcessorImpl();
  // Now, declare stages if a pipeline was declared.
  // Otherwise, declare one sc_module to simulate the processor datapath.
  // OBS: For pipelined architectures there must be a stage_list or a pipe_list,
  //      but never both of them.
  if (pipe_list)
  {
   //Pipeline list exists. Used for ac_pipe declarations.
   for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   {
    // Creating stage module header files.
    CreateStgHeader(ppipe->stages, ppipe->name);
    // Creating stage module implementation files.
    CreateStgImpl(ppipe->stages, ppipe->name);
   }
   // Creating stages references header.
   CreateStagesRefHeader();
  }
  if (HaveFormattedRegs) // Creating formatted registers header and implementation (???) files.
  {
   CreateRegsHeader();
   //CreateRegsImpl();
  }
  if (HaveTLMIntrPorts)
  {
   CreateIntrHeader();
   CreateIntrMacrosHeader();
   CreateIntrTmpl();
  }
  // Creating co-verification class header file.
#if 0 // Co-verification is currently unmaintained. --Marilia
  if (ACVerifyFlag)
   CreateCoverifHeader();
#endif
  // Creating simulation statistics class header and implementation file.
  if (ACStatsFlag)
  {
   CreateStatsHeaderTmpl();
   CreateStatsImplTmpl();
  }
  // Creating model syscall header file.
  if (ACABIFlag)
  {
   CreateArchSyscallHeader();
   CreateArchABIImplTmpl();
  }
  // Create the template for the .cpp instruction and format behavior file.
  CreateImplTmpl();
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
  // Create the template for the .cpp GDB integration file.
  if (ACGDBIntegrationFlag)
   CreateArchGDBImplTmpl();
#endif
  // Create the template for the main.cpp file.
  CreateMainTmpl();
  // Create the makefile.
  CreateMakefile();
  // Issuing final messages to the user.
  AC_MSG("%s model files generated.\n", project_name);
  if (ACDasmFlag)
   AC_MSG("Disassembler file is: %s.dasm\n", project_name);
 }
 free(caps_project_name);
 return 0;
}

////////////////////////////////////////////////////////////////////////////////////
// Create functions ...                                                           //
// These functions are used to create the behavioral simulation files             //
// All of them use structures built by the parser.                                //
////////////////////////////////////////////////////////////////////////////////////

/*! Create architecture-specific resources header file. */
void CreateArchHeader(void)
{
 extern char* project_name;
 extern ac_pipe_list* pipe_list;
 extern ac_sto_list* storage_list;
 extern ac_sto_list* fetch_device;
 extern ac_sto_list* load_device;
 extern ac_dec_format* format_reg_list;
 extern int HaveFormattedRegs, HaveMultiCycleIns, HaveMemHier, HaveTLMPorts,
            HaveTLMIntrPorts, reg_width, stage_num;
 ac_pipe_list* ppipe;
 ac_stg_list* pstage;
 ac_sto_list* pstorage;
 ac_dec_format* pformat;
 ac_dec_field* pfield;
 char* cache_str;
 char* prev_stage_name;
 FILE* output;
 char* filename;

 load_device = storage_list;
 filename = malloc(sizeof(char) * (8 + strlen(project_name)));
 sprintf(filename, "%s_arch.H", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, "ArchC arch-specific resources header file.");
 fprintf(output, "#ifndef _%s_ARCH_H_\n", caps_project_name);
 fprintf(output, "#define _%s_ARCH_H_\n", caps_project_name);
 fprintf(output, "\n");
 fprintf(output, "#include \"%s_parms.H\"\n", project_name);
 fprintf(output, "#include \"ac_arch_dec_if.H\"\n");
 fprintf(output, "#include \"ac_storage.H\"\n");
 fprintf(output, "#include \"ac_memport.H\"\n");
 fprintf(output, "#include \"ac_sync_reg.H\"\n");
 fprintf(output, "#include \"ac_regbank.H\"\n");
 if (HaveTLMPorts)
  fprintf(output, "#include \"ac_tlm_port.H\"\n");
 if (HaveTLMIntrPorts)
  fprintf(output, "#include \"ac_tlm_intr_port.H\"\n");
 fprintf(output, "\n");
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
 {
  fprintf(output, "#include \"ac_msgbuf.H\"\n");
  fprintf(output, "#include <sys/ipc.h>\n");
  fprintf(output, "#include <unistd.h>\n");
  fprintf(output, "#include <sys/msg.h>\n");
  fprintf(output, "#include <sys/types.h>\n");
 }
#endif
#if 0 // More unmantained stuff. --Marilia
 if (HaveMemHier)
 {
  fprintf(output, "#include \"ac_mem.H\"\n");
  fprintf(output, "#include \"ac_cache.H\"\n");
 }
#endif
 if (HaveFormattedRegs)
  fprintf(output, "#include \"%s_fmt_regs.H\"\n", project_name);
 fprintf(output, "\n");
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
 if (ACGDBIntegrationFlag)
 {
  fprintf(output, "#ifndef PORT_NUM\n" );
  fprintf(output, "#define PORT_NUM 5000 // Port to which GDB binds for remote debugging.\n");
  fprintf(output, "#endif /* PORT_NUM */\n" );
 }
#endif
 // Declaring Architecture Resources class.
 COMMENT(INDENT[0], "ArchC class for architecture-specific resources.\n");
 fprintf(output, "%sclass %s_arch: public ac_arch_dec_if<%s_parms::ac_word, %s_parms::ac_Hword>\n%s{\n",
         INDENT[0], project_name, project_name, project_name, INDENT[0]);
 fprintf(output, "%spublic:\n", INDENT[1]);
 // Disassembler file.
 if (ACDasmFlag)
  fprintf(output, "%sofstream dasmfile;\n", INDENT[1]);
 COMMENT(INDENT[2], "Program Counter register.");
 fprintf(output, "%sac_sync_reg<%s> ac_pc;\n",
         INDENT[2], pc_type);
 /* Declaring storage devices */
 COMMENT(INDENT[2], "Storage devices.");
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
 {
  switch (pstorage->type)
  {
   case REG:
    // Formatted registers have a special class.
    if (pstorage->format != NULL)
     fprintf(output, "%s%s_fmt_%s %s;\n", INDENT[2], project_name, pstorage->format,
             pstorage->name);
    else
    {
     switch ((unsigned) (pstorage->width))
     {
      case 0:
       fprintf(output, "%sac_sync_reg<%s_parms::ac_word> %s;\n", INDENT[2],
               project_name, pstorage->name);
       break;
      case 1:
       fprintf(output, "%sac_sync_reg<bool> %s;\n", INDENT[1], pstorage->name);
       break;
      case 8:
       fprintf(output, "%sac_sync_reg<unsigned char> %s;\n", INDENT[1],
               pstorage->name);
       break;
      case 16:
       fprintf(output, "%sac_sync_reg<unsigned short> %s;\n", INDENT[1],
               pstorage->name);
       break;
      case 32:
       fprintf(output, "%sac_sync_reg<unsigned long> %s;\n", INDENT[1],
               pstorage->name);
       break;
      case 64:
       fprintf(output, "%sac_sync_reg<unsigned long long> %s;\n", INDENT[1],
               pstorage->name);
       break;
      default:
       AC_ERROR("Register width not supported: %d\n", pstorage->width);
     }
    }
    break;
   case REGBANK:
    // Emiting register bank. Checking whether a register width was declared.
    switch ((unsigned) pstorage->width)
    {
     case 0:
      fprintf(output, "%sac_regbank<%d, %s_parms::ac_word, %s_parms::ac_Dword> %s;\n",
              INDENT[2], pstorage->size, project_name, project_name, pstorage->name);
      break;
     case 8:
      fprintf(output, "%sac_regbank<%d, unsigned char, unsigned short> %s;\n",
              INDENT[2], pstorage->size, pstorage->name);
      break;
     case 16:
      fprintf(output, "%sac_regbank<%d, unsigned short, unsigned long> %s;\n",
              INDENT[2], pstorage->size, pstorage->name);
      break;
     case 32:
      fprintf(output, "%sac_regbank<%d, unsigned long, unsigned long long> %s;\n",
              INDENT[2], pstorage->size, pstorage->name);
      break;
     case 64:
      fprintf(output, "%sac_regbank<%d, unsigned long long, unsigned long long> %s;\n",
              INDENT[2], pstorage->size, pstorage->name);
      break;
     default:
      AC_ERROR("Register width not supported: %d\n", pstorage->width);
    }
    break;
   case CACHE:
   case ICACHE:
   case DCACHE:
    if (!pstorage->parms)
    { // It is a generic cache. Just emit a base container object.
     fprintf(output, "%sac_storage %s_stg;\n", INDENT[2], pstorage->name);
     fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword> %s;\n",
             INDENT[2], project_name, project_name, pstorage->name);
    }
    else // It is an ac_cache object.
     fprintf(output, "%sac_cache %s;\n", INDENT[2], pstorage->name);
    break;
   case MEM:
    if (!HaveMemHier)
    { // It is a generic mem. Just emit a base container object.
     fprintf(output, "%sac_storage %s_stg;\n", INDENT[2], pstorage->name);
     fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword> %s;\n",
             INDENT[2], project_name, project_name, pstorage->name);
    }
    else // It is an ac_mem object.
     fprintf(output, "%sac_mem %s;\n", INDENT[2], pstorage->name);
    break;
   /* IMPORTANT TODO: TLM_PORT and TLM_INTR_PORT */
   case TLM_PORT:
    fprintf(output, "%sac_tlm_port %s_port;\n", INDENT[2], pstorage->name);
    fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword> %s;\n",
            INDENT[2], project_name, project_name, pstorage->name);
    break;
   case TLM_INTR_PORT:
    fprintf(output, "%sac_tlm_intr_port %s;\n", INDENT[2], pstorage->name);
    break;
   default:
    fprintf(output, "%sac_storage %s_stg;\n", INDENT[2], pstorage->name);
    fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword> %s;\n",
            INDENT[2], project_name, project_name, pstorage->name);
  }
 }
 // The cycle control variable. Only available for multi-cycle archs.
 if (HaveMultiCycleIns)
  fprintf(output, "%sunsigned ac_cycle;\n", INDENT[2]);
 fprintf(output, "\n");
  // Constructor.
 COMMENT(INDENT[2], "Constructor.");
 fprintf(output, "%sexplicit %s_arch(): ac_arch_dec_if<%s_parms::ac_word, %s_parms::ac_Hword>(%s_parms::AC_MAX_BUFFER)",
         INDENT[2], project_name, project_name, project_name, project_name);
//// Initialization begin.
 fprintf(output, ", ac_pc(\"ac_pc\", 0)");
 /* Initializing storage devices */
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
 {
  switch (pstorage->type)
  {
   case REG:
    // Formatted registers have a special class.
    if (pstorage->format != NULL)
     fprintf(output, ", %s(\"%s\")", pstorage->name, pstorage->name);
    else
     fprintf(output, ", %s(\"%s\", 0)", pstorage->name, pstorage->name);
    break;
   case REGBANK:
    fprintf(output, ", %s(\"%s\")", pstorage->name, pstorage->name);
    break;
   case CACHE:
   case ICACHE:
   case DCACHE:
    if (!pstorage->parms) // It is a generic cache. Just emit a base container object.
     fprintf(output, ", %s_stg(\"%s_stg\", %uU), %s(*this, %s_stg)",
             pstorage->name, pstorage->name, pstorage->size,
             pstorage->name, pstorage->name);
    else
    { // It is an ac_cache object.
     cache_str = (char*) malloc(sizeof(char) * (534 + (2 * strlen(pstorage->name))));
     EmitCacheInitialization(cache_str, pstorage);
     fprintf(output, ", %s", cache_str);
     free(cache_str);
    }
    break;
   case MEM:
    if (!HaveMemHier) // It is a generic mem. Just emit a base container object.
     fprintf(output, ", %s_stg(\"%s_stg\", %uU), %s(*this, %s_stg)",
             pstorage->name, pstorage->name, pstorage->size,
             pstorage->name, pstorage->name);
#if 0 // Unmaintained. --Marilia
    else // It is an ac_mem object.
     // Some initialization stuff is supposed to go here, but this is unmaintained code. --Marilia
     fprintf(output, ", %s_stg(\"%s_stg\", %uU), %s(*this, %s_stg)",
             pstorage->name, pstorage->name, pstorage->size,
             pstorage->name, pstorage->name);
#endif
    break;
   /* IMPORTANT TODO: TLM_PORT and TLM_INTR_PORT */
   case TLM_PORT:
    fprintf(output, ", %s_port(\"%s_port\", %uU), %s(*this, %s_port)",
            pstorage->name, pstorage->name, pstorage->size,
            pstorage->name, pstorage->name);
    break;
   case TLM_INTR_PORT:
    fprintf(output, ", %s(\"%s\")", pstorage->name, pstorage->name);
    break;
   default:
    fprintf(output, ", %s_stg(\"%s_stg\", %uU), %s(*this, %s_stg)",
            pstorage->name, pstorage->name, pstorage->size,
            pstorage->name, pstorage->name);
  }
 }
 // The cycle control variable. Only available for multi-cycle archs.
 if (HaveMultiCycleIns)
  fprintf(output, ", ac_cycle(1)");
//// Initialization end.
 fprintf(output, "\n%s{\n", INDENT[2]);
 COMMENT(INDENT[3], "Initializing.");
 /* Setting endianness match, */
 fprintf(output, "%sac_mt_endian = %s_parms::AC_MATCH_ENDIAN;\n", INDENT[3],
         project_name);
 /* Determining which device is gonna be used for fetching instructions. */
 if (!fetch_device)
 {
  /* The parser has not determined because there is not an ac_icache obj declared.
     In this case, look for the object with the lowest (zero) hierarchy level. */
  for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
   if ((pstorage->level == 0) && (pstorage->type != REG) &&
       (pstorage->type != REGBANK))
    fetch_device = pstorage;
  if (!fetch_device)
  { // Couldn't find a fetch device. Error!
   AC_INTERNAL_ERROR("Could not determine a device for fetching.");
   exit(1);
  }
 }
 fprintf(output, "%sIM = &%s;\n", INDENT[3], fetch_device->name);
 /* Determining which device is going to be used for loading applications. */
 /* The device used for loading applications must be the one in the highest
    level of a memory hierachy. */
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  if (pstorage->level > load_device->level)
   load_device = pstorage;
/* If there is only one level, which is gonna be zero, then it is the same
    object used for fetching. */
 if (load_device->level == 0)
  load_device = fetch_device;
 fprintf(output, "%sAPP_MEM = &%s;\n", INDENT[3], load_device->name);
 /* Connecting memory hierarchy. */
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  if (pstorage->higher)
   fprintf(output, "%s%s.bindToNext(%s);\n", INDENT[3], pstorage->name,
           pstorage->higher->name);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[3], INDENT[2]);
 // End of constructor.
 // Destructor.
 COMMENT(INDENT[2], "Destructor.");
 fprintf(output, "%svirtual ~%s_arch()\n%s{\n%sreturn;\n%s}\n\n", INDENT[2],
         project_name, INDENT[2], INDENT[3], INDENT[2]);
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
 {
  int ndevice = 0;
  // Set co-verification msg queue method.
  fprintf(output, "%svoid set_queue(char* exec_name)\n%s{\n", INDENT[2],
          INDENT[2]);
  fprintf(output, "%sstruct start_msgbuf sbuf;\n", INDENT[3]);
  fprintf(output, "%sstruct dev_msgbuf dbuf;\n", INDENT[3]);
  fprintf(output, "%sextern key_t key;\n", INDENT[3]);
  fprintf(output, "%sextern int msqid;\n\n", INDENT[3]);
  fprintf(output, "%sif ((key = ftok(exec_name, 'A')) == -1)\n%s{\n", INDENT[3],
          INDENT[3]);
  fprintf(output, "%sAC_ERROR(\"Could not attach to the co-verification msg queue. Process: \" << getpid());\n",
          INDENT[4]);
  fprintf(output, "%sperror(\"ftok\");\n", INDENT[4]);
  fprintf(output, "%sexit(1);\n", INDENT[4]);
  fprintf(output, "%s}\n", INDENT[3]);
  fprintf(output, "%sif ((msqid = msgget(key, 0644)) == -1)\n%s{\n", INDENT[3],
          INDENT[3]);
  fprintf(output, "%sAC_ERROR(\"Could not attach to the co-verification msg queue. Process: \" << getpid());\n",
          INDENT[4]);
  fprintf(output, "%sperror(\"msgget\");\n", INDENT[4]);
  fprintf(output, "%sexit(1);\n", INDENT[4]);
  fprintf(output, "%s}\n", INDENT[3]);
  for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  {
   if ((pstorage->type == MEM) || (pstorage->type == ICACHE) ||
       (pstorage->type == DCACHE) || (pstorage->type == CACHE) ||
       (pstorage->type == REGBANK))
    ndevice++;
  }
  fprintf(output, "%ssbuf.mtype = 1;\n", INDENT[3]);
  fprintf(output, "%ssbuf.ndevice = %d;\n", INDENT[3], ndevice);
  fprintf(output, "%sif (msgsnd(msqid, reinterpret_cast<void*>(&sbuf), sizeof(sbuf), 0) == -1)\n",
          INDENT[3]);
  fprintf(output, "%sperror(\"msgsnd\");\n", INDENT[4]);
  fprintf(output, "%sdbuf.mtype = 2;\n", INDENT[3]);
  for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  {
   if ((pstorage->type == MEM) || (pstorage->type == ICACHE) ||
       (pstorage->type == DCACHE) || (pstorage->type == CACHE) ||
       (pstorage->type == REGBANK))
   {
    fprintf(output, "%sstrcpy(dbuf.name,%s.get_name());\n", INDENT[3],
            pstorage->name);
    fprintf(output, "%sif (msgsnd(msqid, reinterpret_cast<void*>(&dbuf), sizeof(dbuf), 0) == -1)\n",
            INDENT[3]);
    fprintf(output, "%sperror(\"msgsnd\");\n", INDENT[4]);
   }
  }
  fprintf(output, "%sreturn;\n%s}\n\n", INDENT[3], INDENT[2]); // End of set_queue.
 }
#endif
 // Delegated ac_pc read access.
 COMMENT(INDENT[2], "Delegated read access to ac_pc.");
 fprintf(output, "%sunsigned get_ac_pc()\n%s{\n%sreturn static_cast<unsigned>(ac_pc.read());\n%s}\n",
         INDENT[2], INDENT[2], INDENT[3], INDENT[2]);
 fprintf(output, "%s};\n\n", INDENT[0]); // End of class.
 fprintf(output, "#endif // _%s_ARCH_H_\n", caps_project_name);
 fclose(output);
 return;
}

/*! Create architecture-specific resources references header file. */
void CreateArchRefHeader(void)
{
 extern char* project_name;
 extern ac_pipe_list* pipe_list;
 extern ac_sto_list* storage_list;
 extern ac_dec_format* format_reg_list;
 extern int HaveFormattedRegs, HaveMultiCycleIns, HaveMemHier, HaveTLMPorts,
            HaveTLMIntrPorts, reg_width;
 ac_pipe_list* ppipe;
 ac_stg_list* pstage;
 ac_sto_list* pstorage;
 ac_dec_format* pformat;
 ac_dec_field* pfield;
 char* cache_str;
 FILE* output;
 char* filename;

 filename = malloc(sizeof(char) * (12 + strlen(project_name)));
 sprintf(filename, "%s_arch_ref.H", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, "ArchC arch-specific references header file.");
 fprintf(output, "#ifndef _%s_ARCH_REF_H_\n", caps_project_name);
 fprintf(output, "#define _%s_ARCH_REF_H_\n", caps_project_name);
 fprintf(output, "\n");
 fprintf(output, "#include \"%s_parms.H\"\n", project_name);
 fprintf(output, "#include \"ac_arch_ref.H\"\n");
 fprintf(output, "#include \"ac_memport.H\"\n");
 fprintf(output, "#include \"ac_regbank.H\"\n");
 fprintf(output, "#include \"ac_sync_reg.H\"\n");
 if (HaveTLMIntrPorts)
  fprintf(output, "#include \"ac_tlm_intr_port.H\"\n");
 if (HaveMemHier)
 {
  fprintf(output, "#include \"ac_mem.H\"\n");
  fprintf(output, "#include \"ac_cache.H\"\n");
 }
 if (HaveFormattedRegs)
  fprintf(output, "#include \"%s_fmt_regs.H\"\n", project_name);
 fprintf(output, "\n");
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
 if (ACGDBIntegrationFlag)
 {
  fprintf(output, "#ifndef PORT_NUM\n" );
  fprintf(output, "#define PORT_NUM 5000 // Port to which GDB binds for remote debugging.\n");
  fprintf(output, "#endif /* PORT_NUM */\n" );
 }
#endif
 // Forward declarations.
 COMMENT(INDENT[0], "Forward class declarations, needed to compile.");
 fprintf(output, "class %s_arch;\n\n", project_name);
 // Declaring Architecture Resources class.
 COMMENT(INDENT[0], "ArchC class for architecture-specific resources references.");
 fprintf(output, "%sclass %s_arch_ref: public ac_arch_ref<%s_parms::ac_word, %s_parms::ac_Hword>\n%s{\n",
         INDENT[0], project_name, project_name, project_name, INDENT[0]);
 fprintf(output, "%sprivate:\n", INDENT[1]);
 fprintf(output, "%s%s_arch& p;\n", INDENT[2], project_name); // Do NOT replicate this in the "arch" file. --Marilia
 fprintf(output, "\n");
 fprintf(output, "%sprotected:\n", INDENT[1]);
 // A statistics declaration used to be here, but it's in ac_arch (which is library) now. --Marilia
 if (ACDasmFlag)
  fprintf(output, "%sofstream& dasmfile;\n", INDENT[1]);
 COMMENT(INDENT[2], "Program Counter register.");
 fprintf(output, "%sac_sync_reg<%s>& ac_pc;\n", INDENT[2], pc_type);
 /* Declaring storage devices */
 COMMENT(INDENT[2], "Storage devices.");
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
 {
  switch (pstorage->type)
  {
   case REG:
    // Formatted registers have a special class.
    if (pstorage->format != NULL)
     fprintf(output, "%s%s_fmt_%s& %s;\n", INDENT[2], project_name, pstorage->format,
             pstorage->name);
    else
     // This is correct, isn't it? That registers default to ac_word? --Marilia
     fprintf(output, "%sac_sync_reg<%s_parms::ac_word>& %s;\n",
             INDENT[2], project_name, pstorage->name);
    break;
   case REGBANK:
    // Emiting register bank. Checking is a register width was declared.
    switch ((unsigned) reg_width)
    {
     case 0:
      fprintf(output, "%sac_regbank<%d, %s_parms::ac_word, %s_parms::ac_Dword>& %s;\n",
              INDENT[2], pstorage->size, project_name, project_name, pstorage->name);
      break;
     case 8:
      fprintf(output, "%sac_regbank<%d, unsigned char, unsigned char>& %s;\n",
              INDENT[2], pstorage->size, pstorage->name);
      break;
     case 16:
      fprintf(output, "%sac_regbank<%d, unsigned short, unsigned char>& %s;\n",
              INDENT[2], pstorage->size, pstorage->name);
      break;
     case 32:
      fprintf(output, "%sac_regbank<%d, unsigned long, unsigned short>& %s;\n",
              INDENT[2], pstorage->size, pstorage->name);
      break;
     case 64:
      fprintf(output, "%sac_regbank<%d, unsigned long long, unsigned long>& %s;\n",
              INDENT[2], pstorage->size, pstorage->name);
      break;
     default:
      AC_ERROR("Register width not supported: %d\n", reg_width);
    }
    break;
   case CACHE:
   case ICACHE:
   case DCACHE:
    if (!HaveMemHier)
     // It is a generic cache. Just emit a base container object.
     fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword>& %s;\n",
             INDENT[2], project_name, project_name, pstorage->name);
    else
     // It is an ac_cache object.
     fprintf(output, "%sac_cache& %s;\n", INDENT[2], pstorage->name);
    break;
   case MEM:
    if (!HaveMemHier)
     // It is a generic mem. Just emit a base container object.
     fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword>& %s;\n",
             INDENT[2], project_name, project_name, pstorage->name);
    else // It is an ac_mem object.
     fprintf(output, "%sac_mem& %s;\n", INDENT[2], pstorage->name);
    break;
   case TLM_INTR_PORT:
    fprintf(output, "%sac_tlm_intr_port& %s;\n", INDENT[2], pstorage->name);
    break;
   default:
    fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword>& %s;\n",
            INDENT[2], project_name, project_name, pstorage->name);
  }
 }
 // The cycle control variable. Only available for multi-cycle archs
 if (HaveMultiCycleIns)
  fprintf(output, "%sunsigned& ac_cycle;\n", INDENT[2]);
 // Constructor.
 COMMENT(INDENT[2], "Default constructor.");
 fprintf(output, "%s%s_arch_ref(%s_arch& arch);\n", INDENT[2], project_name,
         project_name);
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
  // Set co-verification msg queue method.
  fprintf(output, "%svoid set_queue(char* exec_name);\n", INDENT[2]);
#endif
 // Delegated ac_pc read access.
 COMMENT(INDENT[2], "Delegated read access to ac_pc.");
 fprintf(output, "%sunsigned get_ac_pc();\n", INDENT[2]);
 fprintf(output, "};\n\n"); // End of class.
 fprintf(output, "#endif // _%s_ARCH_REF_H_\n", caps_project_name);
 fclose(output);
 return;
}

/*! Create parameters header file. */
void CreateParmHeader(void)
{
 extern ac_pipe_list* pipe_list;
 extern ac_dec_format* format_ins_list;
 extern int instr_num;
 extern int declist_num;
 extern int format_num, largest_format_size;
 extern int wordsize, fetchsize, HaveMemHier, HaveCycleRange, HaveTLMPorts,
            HaveTLMIntrPorts;
 extern ac_sto_list* load_device;
 extern ac_decoder_full* decoder;
 extern char* project_name;
 ac_stg_list* pstage;
 ac_pipe_list* ppipe;
 //! File containing decoding structures
 FILE* output;
 char* filename;

 filename = (char*) malloc(sizeof(char) * (9 + strlen(project_name)));
 sprintf(filename, "%s_parms.H", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, "ArchC parameters header file.");
 fprintf(output, "#ifndef _%s_PARMS_H_\n", caps_project_name);
 fprintf(output, "#define _%s_PARMS_H_\n", caps_project_name);
 fprintf(output, "\n");
 // Options defines.
 if (ACVerboseFlag
#if 0 // Co-verification is currently unmaintained. --Marilia
  || ACVerifyFlag
#endif
    )
  fprintf(output, "#define AC_UPDATE_LOG\t//!< Update log generation turned on.\n");
 if (ACVerboseFlag)
  fprintf(output, "#define AC_VERBOSE\t//!< Indicates Verbose mode. Where update logs are dumped on screen.\n");
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
  fprintf(output, "#define AC_VERIFY\t//!< Indicates that co-verification is turned on.\n");
#endif
 if (ACStatsFlag)
  fprintf(output, "#define AC_STATS\t//!< Indicates that statistics collection is turned on.\n");
 if (ACDasmFlag)
  fprintf(output, "#define AC_DISASSEMBLER\t//!< Indicates that disassembler option is turned on.\n");
 if (ACDebugFlag)
  fprintf(output, "#define AC_DEBUG\t//!< Indicates that debug option is turned on.\n");
 if (HaveMemHier)
  fprintf(output, "#define AC_MEM_HIERARCHY\t//!< Indicates that a memory hierarchy was declared.\n");
 if (HaveCycleRange)
  fprintf(output, "#define AC_CYCLE_RANGE\t//!< Indicates that cycle range for instructions were declared.\n");
 fprintf(output, "\n");
 // Parms class definition.
 fprintf(output, "namespace %s_parms\n{\n", project_name);
 fprintf(output,
         "%sstatic const unsigned int AC_DEC_FIELD_NUMBER = %d; \t //!< Number of Fields used by decoder.\n",
         INDENT[2], decoder->nFields);
 fprintf(output,
         "%sstatic const unsigned int AC_DEC_INSTR_NUMBER = %d; \t //!< Number of Instructions declared.\n",
         INDENT[2], instr_num);
 fprintf(output,
         "%sstatic const unsigned int AC_DEC_FORMAT_NUMBER = %d; \t //!< Number of Formats declared.\n",
         INDENT[2], format_num);
 fprintf(output,
         "%sstatic const unsigned int AC_DEC_LIST_NUMBER = %d; \t //!< Number of decodification lists used by decoder.\n",
         INDENT[2], declist_num);
 fprintf(output,
         "%sstatic const unsigned int AC_MAX_BUFFER = %d; \t //!< This is the size needed by decoder buffer. It is equal to the biggest instruction size.\n",
         INDENT[2], (largest_format_size / 8));
 fprintf(output,
         "%sstatic const unsigned int AC_WORDSIZE = %d; \t //!< Architecture wordsize in bits.\n",
         INDENT[2], wordsize);
 fprintf(output,
         "%sstatic const unsigned int AC_FETCHSIZE = %d; \t //!< Architecture fetchsize in bits.\n",
         INDENT[2], fetchsize);
 fprintf(output,
         "%sstatic const unsigned int AC_MATCH_ENDIAN = %d; \t //!< If the simulated arch match the endian with host.\n",
         INDENT[2], ac_match_endian);
 fprintf(output,
         "%sstatic const unsigned int AC_PROC_ENDIAN = %d; \t //!< The simulated arch is big endian?\n",
         INDENT[2], ac_tgt_endian);
 fprintf(output,
         "%sstatic const unsigned int AC_RAMSIZE = %uU; \t //!< Architecture RAM size in bytes (storage %s).\n",
         INDENT[2], load_device->size, load_device->name);
 fprintf(output,
         "%sstatic const unsigned int AC_RAM_END = %uU; \t //!< Architecture end of RAM (storage %s).\n",
         INDENT[2], load_device->size, load_device->name);
 fprintf(output, "\n\n");
 COMMENT(INDENT[0],"Word type definitions.");
 // Emitting ArchC word types.
 switch (wordsize)
 {
  case 8:
   fprintf(output, "%stypedef unsigned char ac_word;\t//!< Unsigned word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned char ac_Uword;\t//!< Unsigned word.\n", INDENT[2]);
   fprintf(output, "%stypedef char ac_Sword;\t//!< Signed word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned char ac_Hword;\t//!< Signed half word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned char ac_UHword;\t//!< Unsigned half word.\n", INDENT[2]);
   fprintf(output, "%stypedef char ac_SHword;\t//!< Signed half word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned short ac_Dword;\t//!< Signed double word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned short ac_UDword;\t//!< Unsigned double word.\n", INDENT[2]);
   fprintf(output, "%stypedef short ac_SDword;\t//!< Signed double word.\n", INDENT[2]);
   break;
  case 16:
   fprintf(output, "%stypedef unsigned short int ac_word;\t//!< Unsigned word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned short int ac_Uword;\t//!< Unsigned word.\n", INDENT[2]);
   fprintf(output, "%stypedef short int ac_Sword;\t//!< Signed word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned char ac_Hword;\t//!< Signed half word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned char ac_UHword;\t//!< Unsigned half word.\n", INDENT[2]);
   fprintf(output, "%stypedef char ac_SHword;\t//!< Signed half word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned int ac_Dword;\t//!< Signed double word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned int ac_UDword;\t//!< Unsigned double word.\n", INDENT[2]);
   fprintf(output, "%stypedef int ac_SDword;\t//!< Signed double word.\n", INDENT[2]);
   break;
  case 32:
   fprintf(output, "%stypedef unsigned int ac_word;\t//!< Unsigned word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned int ac_Uword;\t//!< Unsigned word.\n", INDENT[2]);
   fprintf(output, "%stypedef int ac_Sword;\t//!< Signed word.\n", INDENT[2]);
   fprintf(output, "%stypedef short int ac_SHword;\t//!< Signed half word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned short int ac_UHword;\t//!< Unsigned half word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned short int ac_Hword;\t//!< Unsigned half word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned long long ac_Dword;\t//!< Signed double word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned long long ac_UDword;\t//!< Unsigned double word.\n", INDENT[2]);
   fprintf(output, "%stypedef long long ac_SDword;\t//!< Signed double word.\n", INDENT[2]);
   break;
  case 64:
   fprintf(output, "%stypedef unsigned long long ac_word;\t//!< Unsigned word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned long long ac_Uword;\t//!< Unsigned word.\n", INDENT[2]);
   fprintf(output, "%stypedef long long ac_Sword;\t//!< Signed word.\n", INDENT[2]);
   fprintf(output, "%stypedef int ac_SHword;\t//!< Signed half word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned int ac_UHword;\t//!< Unsigned half word.\n", INDENT[2]);
   fprintf(output, "%stypedef unsigned int ac_UHword;\t//!< Unsigned half word.\n", INDENT[2]);
   break;
  default:
   AC_ERROR("Wordsize not supported: %d\n", wordsize);
   break;
 }
 fprintf(output, "%stypedef char ac_byte;\t//!< Signed byte word.\n", INDENT[2]);
 fprintf(output, "%stypedef unsigned char ac_Ubyte;\t//!< Unsigned byte word.\n\n", INDENT[2]);
 COMMENT(INDENT[2], "Fetch type definition.");
 switch (fetchsize)
 {
  case 8:
   fprintf(output, "%stypedef unsigned char ac_fetch;\t//!< Unsigned word.\n", INDENT[2]);
   break;
  case 16:
   fprintf(output, "%stypedef unsigned short int ac_fetch;\t//!< Unsigned word.\n", INDENT[2]);
   break;
  case 32:
   fprintf(output, "%stypedef unsigned int ac_fetch;\t//!< Unsigned word.\n", INDENT[2]);
   break;
  case 64:
   fprintf(output, "%stypedef unsigned long long ac_fetch;\t//!< Unsigned word.\n", INDENT[2]);
   break;
  default:
   AC_ERROR("Fetchsize not supported: %d\n", fetchsize);
   break;
 }
 fprintf(output, "\n");
 // This enum type is used for case identification inside the ac_behavior methods.
 if (pipe_list) // Enum type for pipes declared through ac_pipe keyword.
 {
  fprintf(output, "%senum ac_stage_list {", INDENT[2]);
  fprintf(output, "id_%s_%s = %d", pipe_list->name, pipe_list->stages->name,
          pipe_list->stages->id);
  pstage = pipe_list->stages->next;
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
  {
   for (; pstage != NULL; pstage = pstage->next)
    fprintf(output, ", id_%s_%s = %d", ppipe->name, pstage->name, pstage->id);
   pstage = ppipe->stages;
  }
 }
 else
  fprintf(output, "enum ac_stage_list {ST0");
 // Closing enum declaration.
 fprintf(output, "};\n");
 // Closing namespace declaration.
 fprintf(output, "}\n\n");
 fprintf(output, "\n#endif // _%s_PARMS_H_\n", caps_project_name);
 fclose(output);
 return;
}

/*! Create ISA header file. */
void CreateISAHeader(void)
{
 extern ac_pipe_list* pipe_list;
 extern ac_grp_list* group_list;
 extern int stage_num;
 extern ac_dec_format* format_ins_list;
 extern ac_dec_field* common_instr_field_list;
 extern char* project_name;
 extern char* helper_contents;
 extern ac_dec_instr* instr_list;
 extern int HaveFormattedRegs, HaveMultiCycleIns;
 ac_stg_list* pstage;
 ac_pipe_list* ppipe;
 ac_grp_list* pgroup;
 ac_dec_field* pfield;
 ac_dec_format* pformat;
 ac_dec_instr* pinstr;
 char* filename;
 // File containing ISA declaration.
 FILE* output;

 filename = (char*) malloc(sizeof(char) * (7 + strlen(project_name)));
 sprintf(filename, "%s_isa.H", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, isa_header_description);
 fprintf(output, "#ifndef _%s_ISA_H_\n", caps_project_name);
 fprintf(output, "#define _%s_ISA_H_\n", caps_project_name);
 fprintf(output, "\n");
 fprintf(output, "#include \"ac_arch_dec_if.H\"\n");
 fprintf(output, "#include \"ac_decoder_rt.H\"\n");
 fprintf(output, "#include \"ac_instr.H\"\n");
 fprintf(output, "#include \"ac_instr_info.H\"\n");
 fprintf(output, "\n");
 fprintf(output, "#include \"%s_parms.H\"\n", project_name);
 fprintf(output, "#include \"%s_arch.H\"\n", project_name);
 fprintf(output, "#include \"%s_arch_ref.H\"\n", project_name);
 if (pipe_list)
  fprintf(output, "#include \"%s_stages_ref.H\"\n", project_name);
 if (ACStatsFlag)
  fprintf(output, "#include \"%s_stats.H\"\n", project_name);
 fprintf(output, "\n");
 fprintf(output, "%snamespace %s_parms\n%s{\n", INDENT[0], project_name, INDENT[0]);
 fprintf(output, "%sclass %s_isa: public %s_arch_ref", INDENT[1],
         project_name, project_name);
 if (pipe_list)
  fprintf(output, ", public %s_stages_ref", project_name);
 if (ACStatsFlag)
  fprintf(output, ", public %s_all_stats", project_name);
 fprintf(output, "\n%s{\n", INDENT[1]);
 fprintf(output, "%sprivate:\n", INDENT[2]);
 fprintf(output, "%stypedef ac_instr<AC_DEC_FIELD_NUMBER> ac_instr_t;\n",
         INDENT[3]);
 for (pgroup = group_list; pgroup != NULL; pgroup = pgroup->next)
  fprintf(output, "%sstatic bool group_%s[AC_DEC_INSTR_NUMBER];\n",
          INDENT[3], pgroup->name);
 fprintf(output, "\n");
 fprintf(output, "%spublic:\n", INDENT[2]);
 fprintf(output, "%sunsigned current_instruction_id;\n", INDENT[3]);
 COMMENT(INDENT[3], "Decoding structures.");
 fprintf(output, "%sstatic ac_dec_field fields[AC_DEC_FIELD_NUMBER];\n",
         INDENT[3]);
 fprintf(output, "%sstatic ac_dec_format formats[AC_DEC_FORMAT_NUMBER];\n",
         INDENT[3]);
 fprintf(output, "%sstatic ac_dec_list dec_list[AC_DEC_LIST_NUMBER];\n",
         INDENT[3]);
 fprintf(output, "%sstatic ac_dec_instr instructions[AC_DEC_INSTR_NUMBER];\n",
         INDENT[3]);
 fprintf(output, "%sstatic const ac_instr_info instr_table[AC_DEC_INSTR_NUMBER + 1];\n",
         INDENT[3]);
 fprintf(output, "%sac_decoder_full* decoder;\n", INDENT[3]);
 if (helper_contents)
 {
  fprintf(output, "%s", helper_contents);
  fprintf(output, "\n");
 }
 fprintf(output, "\n");
 // Constructor.
 COMMENT(INDENT[3], "Standard constructor.");
 if (pipe_list)
 {
  fprintf(output, "%s%s_isa(%s_arch& ref", INDENT[3], project_name, project_name);
  // References to stages and automatic pipeline registers.
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
   {
    fprintf(output, ", %s_%s_%s::%s_%s_%s& %s_%s_ref", project_name,
            ppipe->name, pstage->name, project_name, ppipe->name, pstage->name,
            ppipe->name, pstage->name);
    if ((pstage->id != stage_num) && (pstage->next != NULL))
     fprintf(output, ", ac_sync_reg<ac_instr_t>& pr_%s_%s_%s_%s_ref",
             ppipe->name, pstage->name, ppipe->name, pstage->next->name);
   }
  fprintf(output, "): %s_arch_ref(ref), %s_stages_ref(", project_name,
          project_name);
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
   {
    fprintf(output, "%s_%s_ref", ppipe->name, pstage->name, ppipe->name,
            pstage->name);
    if ((pstage->id != stage_num) && (pstage->next != NULL))
     fprintf(output, ", pr_%s_%s_%s_%s_ref",
             ppipe->name, pstage->name, ppipe->name, pstage->next->name,
             ppipe->name, pstage->name, ppipe->name, pstage->next->name);
    if (ppipe->next || pstage->next)
     fprintf(output, ", ");
  }
  fprintf(output, ")");
 }
 else
  fprintf(output, "%s%s_isa(%s_arch& ref): %s_arch_ref(ref)", INDENT[3],
          project_name, project_name, project_name);
 fprintf(output, "\n%s{\n", INDENT[3]);
 COMMENT(INDENT[4], "Building decoder.");
 fprintf(output, "%sdecoder = ac_decoder_full::CreateDecoder(%s_isa::formats, %s_isa::instructions, &ref);\n",
         INDENT[4], project_name, project_name);
#ifdef ADD_DEBUG
 fprintf(output,"#ifdef AC_DEBUG_DECODER\n");
 fprintf(output,"%sShowDecFormat(formats);\n", INDENT[4]);
 fprintf(output,"%sShowDecoder(decoder->decoder, 0);\n", INDENT[4]);
 fprintf(output,"#endif\n");
#endif
 // End of constructor.
 fprintf(output, "%sreturn;\n%s}\n", INDENT[4], INDENT[3]);
 fprintf(output, "\n");
 // Instruction metas.
 fprintf(output, "%sinline const char* get_name()\n%s{\n", INDENT[3], INDENT[3]);
 fprintf(output, "%sreturn instr_table[current_instruction_id].ac_instr_name;\n%s}\n",
         INDENT[4], INDENT[3]);
 fprintf(output, "\n");
 fprintf(output, "%sinline const char* get_name(unsigned id)\n%s{\n", INDENT[3], INDENT[3]);
 fprintf(output, "%sreturn instr_table[id].ac_instr_name;\n%s}\n", INDENT[4], INDENT[3]);
 fprintf(output, "\n");
 fprintf(output, "%sinline const char* get_mnemonic()\n%s{\n", INDENT[3], INDENT[3]);
 fprintf(output, "%sreturn instr_table[current_instruction_id].ac_instr_mnemonic;\n%s}\n",
         INDENT[4], INDENT[3]);
 fprintf(output, "\n");
 fprintf(output, "%sinline const char* get_mnemonic(unsigned id)\n%s{\n", INDENT[3], INDENT[3]);
 fprintf(output, "%sreturn instr_table[id].ac_instr_mnemonic;\n%s}\n", INDENT[4], INDENT[3]);
 fprintf(output, "\n");
 fprintf(output, "%sinline const unsigned get_size()\n%s{\n", INDENT[3], INDENT[3]);
 fprintf(output, "%sreturn instr_table[current_instruction_id].ac_instr_size;\n%s}\n",
         INDENT[4], INDENT[3]);
 fprintf(output, "\n");
 fprintf(output, "%sinline const unsigned get_size(unsigned id)\n%s{\n", INDENT[3], INDENT[3]);
 fprintf(output, "%sreturn instr_table[id].ac_instr_size;\n%s}\n", INDENT[4], INDENT[3]);
 fprintf(output, "\n");
 if (HaveMultiCycleIns)
 {
  fprintf(output, "%sinline const unsigned get_cycles()\n%s{\n", INDENT[3], INDENT[3]);
  fprintf(output, "%sreturn instr_table[current_instruction_id].ac_instr_cycles;\n%s}\n",
          INDENT[4], INDENT[3]);
  fprintf(output, "\n");
  fprintf(output, "%sinline const unsigned get_cycles(unsigned id)\n%s{\n", INDENT[3], INDENT[3]);
  fprintf(output, "%sreturn instr_table[id].ac_instr_cycles;\n%s}\n", INDENT[4], INDENT[3]);
  fprintf(output, "\n");
 }
 fprintf(output, "%sinline const unsigned get_min_latency()\n%s{\n", INDENT[3], INDENT[3]);
 fprintf(output, "%sreturn instr_table[current_instruction_id].ac_instr_size;\n%s}\n",
         INDENT[4], INDENT[3]);
 fprintf(output, "\n");
 fprintf(output, "%sinline const unsigned get_min_latency(unsigned id)\n%s{\n", INDENT[3], INDENT[3]);
 fprintf(output, "%sreturn instr_table[id].ac_instr_size;\n%s}\n", INDENT[4], INDENT[3]);
 fprintf(output, "\n");
 fprintf(output, "%sinline const unsigned get_max_latency()\n%s{\n", INDENT[3], INDENT[3]);
 fprintf(output, "%sreturn instr_table[current_instruction_id].ac_instr_size;\n%s}\n",
         INDENT[4], INDENT[3]);
 fprintf(output, "\n");
 fprintf(output, "%sinline const unsigned get_max_latency(unsigned id)\n%s{\n", INDENT[3], INDENT[3]);
 fprintf(output, "%sreturn instr_table[id].ac_instr_size;\n%s}\n", INDENT[4], INDENT[3]);
 fprintf(output, "\n");
 // Group query methods.
 for (pgroup = group_list; pgroup != NULL; pgroup = pgroup->next)
 {
  fprintf(output, "%sinline const bool belongs_to_%s()\n%s{\n",
          INDENT[3], pgroup->name, INDENT[3]);
  fprintf(output, "%sreturn group_%s[current_instruction_id];\n%s}\n",
          INDENT[4], pgroup->name, INDENT[3]);
  fprintf(output, "\n");
  fprintf(output, "%sinline const bool belongs_to_%s(unsigned id)\n%s{\n",
          INDENT[3], pgroup->name, INDENT[3]);
  fprintf(output, "%sif (id >= AC_DEC_INSTR_NUMBER)\n", INDENT[4]);
  fprintf(output, "%sreturn false; // Instruction does not exist.\n", INDENT[5]);
  fprintf(output, "%sreturn group_%s[id];\n%s}\n", INDENT[4], pgroup->name, INDENT[3]);
  fprintf(output, "\n");
 }
 // Instruction Behavior Method declarations.
 // Instruction (generic -- really, someone should rename this --Marilia).
 fprintf(output, "%svoid _behavior_instruction(ac_stage_list stage, unsigned cycle",
         INDENT[3]);
 /* common_instr_field_list has the list of fields for the generic instruction. */
 for (pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next)
  if (pfield->sign)
   fprintf(output, ", int %s", pfield->name);
  else
   fprintf(output, ", unsigned %s", pfield->name);
 fprintf(output, ");\n");
 fprintf(output, "\n");
 // Begin & end.
 fprintf(output, "%svoid _behavior_begin();\n", INDENT[3]);
 fprintf(output, "%svoid _behavior_end();\n", INDENT[3]);
 fprintf(output, "\n");
 // Types/formats.
 for (pformat = format_ins_list; pformat != NULL; pformat = pformat->next)
 {
  fprintf(output, "%svoid _behavior_%s_%s(ac_stage_list stage, unsigned cycle",
          INDENT[3], project_name, pformat->name);
  for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
   if (pfield->sign)
    fprintf(output, ", int %s", pfield->name);
   else
    fprintf(output, ", unsigned int %s", pfield->name);
  fprintf(output, ");\n");
 }
 fprintf(output, "\n");
 // Instructions.
 for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
 {
  for (pformat = format_ins_list;
       (pformat != NULL) && strcmp(pinstr->format, pformat->name);
       pformat = pformat->next);
  fprintf(output, "%svoid behavior_%s(ac_stage_list stage, unsigned cycle",
          INDENT[3], pinstr->name);
  for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
   if (pfield->sign)
    fprintf(output, ", int %s", pfield->name);
   else
    fprintf(output, ", unsigned int %s", pfield->name);
  fprintf(output, ");\n");
 }
 // End of class.
 fprintf(output, "%s};\n", INDENT[1]);
 // End of namespace.
 fprintf(output, "%s};\n", INDENT[0]);
 fprintf(output, "\n#endif // _%s_ISA_H_\n", caps_project_name);
 // End of file.
 fclose(output);
 // Behavior macros go in a different file.
 filename = (char*) malloc(sizeof(char) * (14 + strlen(project_name)));
 sprintf(filename, "%s_bhv_macros.H", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 fprintf(output, "#ifndef _%s_BHV_MACROS_H_\n", caps_project_name);
 fprintf(output, "#define _%s_BHV_MACROS_H_\n\n", caps_project_name);
 // ac_memory typedef. Copied&pasted from acsim. --Marilia
 fprintf(output, "typedef ac_memport<%s_parms::ac_word, %s_parms::ac_Hword> ac_memory;\n\n",
         project_name, project_name);
 COMMENT(INDENT[0], "Macros for instruction behaviors.\n");
 // ac_behavior main macro.
 fprintf(output, "#ifndef ac_behavior\n");
 fprintf(output, "#define ac_behavior(instr) AC_BEHAVIOR_##instr()\n");
 fprintf(output, "#endif\n\n");
 // ac_behavior 2nd level macros - generic instruction.
 fprintf(output, "#define AC_BEHAVIOR_instruction() %s_parms::%s_isa::_behavior_instruction(%s_parms::ac_stage_list stage, unsigned cycle",
         project_name, project_name, project_name);
 /* common_instr_field_list has the list of fields for the generic instruction. */
 for (pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next)
  if (pfield->sign)
   fprintf(output, ", int %s", pfield->name);
  else
   fprintf(output, ", unsigned %s", pfield->name);
 fprintf(output, ")\n");
 fprintf(output, "\n");
 // ac_behavior 2nd level macros - pseudo-instructions begin, end.
 fprintf(output, "#define AC_BEHAVIOR_begin() %s_parms::%s_isa::_behavior_begin()\n", project_name, project_name);
 fprintf(output, "#define AC_BEHAVIOR_end() %s_parms::%s_isa::_behavior_end()\n", project_name, project_name);
 fprintf(output, "\n");
 // ac_behavior 2nd level macros - instruction types.
 for (pformat = format_ins_list; pformat!= NULL; pformat=pformat->next)
 {
  fprintf(output,
          "#define AC_BEHAVIOR_%s() %s_parms::%s_isa::_behavior_%s_%s(%s_parms::ac_stage_list stage, unsigned cycle",
          pformat->name, project_name, project_name, project_name, pformat->name, project_name);
  for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
   if (pfield->sign)
    fprintf(output, ", int %s", pfield->name);
   else
    fprintf(output, ", unsigned int %s", pfield->name);
  fprintf(output, ")\n");
 }
 fprintf(output, "\n");
 // ac_behavior 2nd level macros - instructions.
 for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
 {
  fprintf(output,
          "#define AC_BEHAVIOR_%s() %s_parms::%s_isa::behavior_%s(%s_parms::ac_stage_list stage, unsigned cycle",
          pinstr->name, project_name, project_name, pinstr->name, project_name);
  for (pformat = format_ins_list;
       (pformat != NULL) && strcmp(pinstr->format, pformat->name);
       pformat = pformat->next);
  for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
  {
   if (pfield->sign)
    fprintf(output, ", int %s", pfield->name);
   else
    fprintf(output, ", unsigned int %s", pfield->name);
  }
  fprintf(output, ")\n");
 }
 // End of file.
 fprintf(output, "\n#endif // _%s_BHV_MACROS_H_\n", caps_project_name);
 fclose(output);
 return;
}

/*! Create processor header file. */
void CreateProcessorHeader(void)
{
 extern ac_pipe_list* pipe_list;
 extern char* project_name;
 extern int stage_num;
 extern int HaveMultiCycleIns;
 ac_stg_list* pstage;
 ac_pipe_list* ppipe;
 char* prev_stage_name;
 char* filename;
 FILE* output;

 filename = (char*) malloc(sizeof(char) * (3 + strlen(project_name)));
 sprintf(filename, "%s.H", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, "Processor header file.");
 fprintf(output, "#ifndef _%s_H_\n", caps_project_name);
 fprintf(output, "#define _%s_H_\n", caps_project_name);
 fprintf(output, "\n");
 fprintf(output, "#include <systemc.h>\n");
 fprintf(output, "#include \"ac_module.H\"\n");
 fprintf(output, "#include \"ac_utils.H\"\n");
 fprintf(output, "#include \"%s_parms.H\"\n", project_name);
 fprintf(output, "#include \"%s_arch.H\"\n", project_name);
 if (pipe_list) // Stage includes.
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
    fprintf(output, "#include \"%s_%s_%s.H\"\n", project_name, ppipe->name,
            pstage->name);
 else
 {
  fprintf(output, "#include \"ac_instr.H\"\n");
  fprintf(output, "#include \"%s_isa.H\"\n", project_name);
 }
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
 else if (ACGDBIntegrationFlag)
  fprintf(output, "#include \"ac_gdb_interface.H\"\n");
#endif
 fprintf(output, "\n");
 if (pipe_list || !ACGDBIntegrationFlag)
  fprintf(output, "%sclass %s: public ac_module, public %s_arch\n%s{\n",
          INDENT[0], project_name, project_name, INDENT[0]);
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
 else
 {
  fprintf(output, "%sextern ac_GDB* gdbstub;\n\n", INDENT[0]);
  fprintf(output, "%sclass %s: public ac_module, public %s_arch, public ac_GDB_interface\n%s{\n",
          INDENT[0], project_name, project_name, INDENT[0]);
 }
#endif
 fprintf(output, "%sprivate:\n", INDENT[1]);
 fprintf(output, "%stypedef cache_item<%s_parms::AC_DEC_FIELD_NUMBER> cache_item_t;\n",
         INDENT[2], project_name);
 fprintf(output, "%stypedef ac_instr<%s_parms::AC_DEC_FIELD_NUMBER> ac_instr_t;\n",
         INDENT[2], project_name);
 fprintf(output, "\n");
 fprintf(output, "%spublic:\n", INDENT[1]);
 fprintf(output, "%ssc_in<bool> clock;\n", INDENT[2]);
 fprintf(output, "%sdouble period;\n", INDENT[2]);
 if (!pipe_list)
 {
  if (ACDecCacheFlag)
   fprintf(output, "%scache_item_t* DEC_CACHE;\n", INDENT[2]);
  fprintf(output, "%sdouble last_clock;\n", INDENT[2]);
  fprintf(output, "%sbool start_up;\n", INDENT[2]);
  fprintf(output, "%sunsigned id;\n", INDENT[2]);
  fprintf(output, "%sunsigned* instr_dec;\n", INDENT[2]);
  fprintf(output, "%sac_instr_t* instr_vec;\n", INDENT[2]);
  fprintf(output, "%s%s_arch* ap;\n", INDENT[2], project_name);
 }
 fprintf(output, "%sbool has_delayed_load;\n", INDENT[2]);
 fprintf(output, "%schar* delayed_load_program;\n", INDENT[2]);
 fprintf(output, "%s%s_parms::%s_isa isa;\n", INDENT[2], project_name, project_name);
 //! Declaring stages modules.
 if (pipe_list)
 { // Pipeline list exists. Used for ac_pipe declarations.
  COMMENT(INDENT[2], "Stage declarations.");
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
   { // Emit stage module and its in/out signal declarations.
    // Stages used to be pointers, but they need to be actual objects now.
    fprintf(output, "%s%s_%s_%s::%s_%s_%s %s_%s;\n", INDENT[2],
            project_name, ppipe->name, pstage->name, project_name, ppipe->name,
            pstage->name, ppipe->name, pstage->name);
    if ((pstage->id != stage_num) && (pstage->next != NULL))
     fprintf(output,
             "%sac_sync_reg<ac_instr_t> pr_%s_%s_%s_%s;\n",
             INDENT[2], ppipe->name, pstage->name, ppipe->name, pstage->next->name);
   }
 }
 else // Since this thing is multicycle, ins_id must be -here-. --Marilia
  fprintf(output, "%sunsigned ins_id;\n", INDENT[2]);
 COMMENT(INDENT[2], "Start simulation.");
 fprintf(output, "%svoid init(int ac, char** av);\n", INDENT[2]);
 fprintf(output, "%svoid init();\n", INDENT[2]);
 COMMENT(INDENT[2], "Program loading.");
 fprintf(output, "%svoid load(char* program);\n", INDENT[2]);
 fprintf(output, "%svoid delayed_load(char* program);\n", INDENT[2]);
 COMMENT(INDENT[2], "Stop simulation (may receive exit status).");
 fprintf(output, "%svoid stop(int status = 0);\n", INDENT[2]);
 COMMENT(INDENT[2], "PrintStat wrapper method.");
 fprintf(output, "%svoid PrintStat();\n", INDENT[2]);
 COMMENT(INDENT[2], "Verification method.");
 fprintf(output, "%svoid ac_verify();\n", INDENT[2]);
 COMMENT(INDENT[2], "Behavior method.");
 if (!pipe_list) // Pipelined architectures do not need a processor behavior.
  fprintf(output, "%svoid behavior();\n", INDENT[2]);
 if (!pipe_list && ACDecCacheFlag) // No pipeline, but with decode cache.
 {
  COMMENT(INDENT[2], "Decoder cache initialization method.");
  fprintf(output, "%svoid init_dec_cache();\n", INDENT[2]);
 }
 fprintf(output, "%sSC_HAS_PROCESS(%s);\n", INDENT[2], project_name);
 fprintf(output, "\n");
 COMMENT(INDENT[2], "Constructor.");
 fprintf(output, "%s%s(sc_module_name mname, double p): ac_module(mname), period(p), isa(*this",
         INDENT[2], project_name);
 if (pipe_list)
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
   {
    fprintf(output, ", %s_%s", ppipe->name, pstage->name);
    if ((pstage->id != stage_num) && (pstage->next != NULL))
     fprintf(output, ", pr_%s_%s_%s_%s", ppipe->name, pstage->name,
             ppipe->name, pstage->next->name);
   }
 fprintf(output, ")");
 if (pipe_list)
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
   {
    fprintf(output,", %s_%s(\"%s_%s_%s\", *this, isa", ppipe->name,
            pstage->name, project_name, ppipe->name, pstage->name);
    if (pstage->id == 1)
    {
     prev_stage_name = pstage->name;
     fprintf(output, ", 0"); // No previous stage.
     if (pstage->next)
      fprintf(output, ", 0, &pr_%s_%s_%s_%s", ppipe->name, pstage->name,
              ppipe->name, pstage->next->name); // Only out register.
     else
      fprintf(output, ", 0, 0"); // No in/out registers.
    }
    else if (pstage->id != stage_num)
    {
     fprintf(output, ", &%s_%s, &pr_%s_%s_%s_%s, &pr_%s_%s_%s_%s",
             ppipe->name, prev_stage_name, ppipe->name, prev_stage_name,
             ppipe->name, pstage->name, ppipe->name, pstage->name, ppipe->name,
             pstage->next->name);
     prev_stage_name = pstage->name;
    }
    else
     fprintf(output, ", &%s_%s, &pr_%s_%s_%s_%s, 0", ppipe->name,
             prev_stage_name, ppipe->name, prev_stage_name, ppipe->name,
             pstage->name);
    fprintf(output, ")");
   }
 if (pipe_list)
 { // Pipeline list exists. Used for ac_pipe declarations.
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
    if ((pstage->id != stage_num) && (pstage->next != NULL))
     fprintf(output, ", pr_%s_%s_%s_%s(\"pr_%s_%s_%s_%s\")",
             ppipe->name, pstage->name, ppipe->name, pstage->next->name,
             ppipe->name, pstage->name, ppipe->name, pstage->next->name);
 }
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
 else if (ACGDBIntegrationFlag)
  fprintf(output, ", ac_GDB_interface(*this)");
#endif
 fprintf(output, "\n%s{\n", INDENT[2]);
 if (pipe_list)
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
    fprintf(output, "%s%s_%s.clock(clock);\n", INDENT[3], ppipe->name, pstage->name);
 if (!pipe_list)
 {
  fprintf(output, "%sSC_METHOD(behavior);\n", INDENT[3]);
  fprintf(output, "%ssensitive_pos << clock;\n", INDENT[3]);
  fprintf(output, "%sdont_initialize();\n", INDENT[3]);
  fprintf(output, "%sap = this;\n", INDENT[3]);
  fprintf(output, "%sstart_up = 1;\n", INDENT[3]);
  fprintf(output, "%sid = 1;\n", INDENT[3]);
  fprintf(output, "%sins_id = 0;\n", INDENT[3]);
  if (ACDasmFlag)
   fprintf(output, "%sap->dasmfile.open(\"%s.dasm\");\n", INDENT[3],
           project_name);
 }
 else
 {
  fprintf(output, "%sSC_METHOD(ac_verify);\n", INDENT[3]);
  fprintf(output, "%ssensitive", INDENT[3]);
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
    fprintf(output, " << %s_%s.done", ppipe->name, pstage->name);
  fprintf(output, ";\n%sdont_initialize();\n", INDENT[3]);
 }
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[3], INDENT[2]);
 COMMENT(INDENT[2], "Destructor.");
 fprintf(output, "%s~%s()\n%s{\n", INDENT[2], project_name, INDENT[2]);
 if (!pipe_list && ACDecCacheFlag)
  fprintf(output, "%sif (DEC_CACHE)\n%sfree(DEC_CACHE);\n", INDENT[3], INDENT[4]);
 fprintf(output, "%sreturn;\n%s}\n", INDENT[3], INDENT[2]);
 // Closing class declaration.
 fprintf(output, "%s};\n\n", INDENT[0]);
 fprintf(output, "#endif // _%s_H_\n", caps_project_name);
 fclose(output);
 return;
}

/*! Create stages references header file. */
void CreateStagesRefHeader(void)
{
 extern ac_pipe_list* pipe_list;
 extern char* project_name;
 extern int stage_num;
 ac_stg_list* pstage;
 ac_pipe_list* ppipe;
 char* filename;
 FILE* output;

 filename = (char*) malloc(sizeof(char) * (14 + strlen(project_name)));
 sprintf(filename, "%s_stages_ref.H", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, "Stages references header file.");
 fprintf(output, "#ifndef _%s_STAGES_REF_H_\n", caps_project_name);
 fprintf(output, "#define _%s_STAGES_REF_H_\n", caps_project_name);
 fprintf(output, "\n");
 fprintf(output, "#include \"ac_instr.H\"\n");
 fprintf(output, "#include \"ac_sync_reg.H\"\n");
 fprintf(output, "#include \"%s_parms.H\"\n", project_name);
 fprintf(output, "\n");
 COMMENT(INDENT[0], "Forward declarations, needed to compile.")
 for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
  for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
   fprintf(output, "namespace %s_%s_%s\n{\n%sclass %s_%s_%s;\n}\n\n",
           project_name, ppipe->name, pstage->name, INDENT[1], project_name,
           ppipe->name, pstage->name);
 fprintf(output, "%sclass %s_stages_ref\n%s{\n",
         INDENT[0], project_name, INDENT[0]);
 fprintf(output, "%sprivate:\n", INDENT[1]);
 fprintf(output, "%stypedef ac_instr<%s_parms::AC_DEC_FIELD_NUMBER> ac_instr_t;\n",
         INDENT[2], project_name);
 fprintf(output, "\n");
 fprintf(output, "%spublic:\n", INDENT[1]);
 // Declaring references to stages and automatic pipeline registers.
 COMMENT(INDENT[2], "References to stages and automatic pipeline registers.");
 for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
  for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
  {
   fprintf(output, "%s%s_%s_%s::%s_%s_%s& %s_%s;\n", INDENT[2],
           project_name, ppipe->name, pstage->name, project_name, ppipe->name,
           pstage->name, ppipe->name, pstage->name);
   if ((pstage->id != stage_num) && (pstage->next != NULL))
    fprintf(output,
            "%sac_sync_reg<ac_instr_t>& pr_%s_%s_%s_%s;\n",
            INDENT[2], ppipe->name, pstage->name, ppipe->name, pstage->next->name);
  }
 fprintf(output, "\n");
 COMMENT(INDENT[2], "Constructor.");
 fprintf(output, "%s%s_stages_ref(", INDENT[2], project_name);
 // References to stages and automatic pipeline registers.
 for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
  for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
  {
   fprintf(output, "%s_%s_%s::%s_%s_%s& %s_%s_ref", project_name,
           ppipe->name, pstage->name, project_name, ppipe->name, pstage->name,
           ppipe->name, pstage->name);
   if ((pstage->id != stage_num) && (pstage->next != NULL))
    fprintf(output, ", ac_sync_reg<ac_instr_t>& pr_%s_%s_%s_%s_ref",
            ppipe->name, pstage->name, ppipe->name, pstage->next->name);
   if (ppipe->next || pstage->next)
    fprintf(output, ", ");
  }
 fprintf(output, "): ");
 for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
  for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
  {
   fprintf(output, "%s_%s(%s_%s_ref)", ppipe->name, pstage->name, ppipe->name,
           pstage->name);
   if ((pstage->id != stage_num) && (pstage->next != NULL))
    fprintf(output, ", pr_%s_%s_%s_%s(pr_%s_%s_%s_%s_ref)",
            ppipe->name, pstage->name, ppipe->name, pstage->next->name,
            ppipe->name, pstage->name, ppipe->name, pstage->next->name);
   if (ppipe->next || pstage->next)
    fprintf(output, ", ");
  }
 fprintf(output, "\n%s{\n", INDENT[2]);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[3], INDENT[2]);
 COMMENT(INDENT[2], "Destructor.");
 fprintf(output, "%s~%s_stages_ref()\n%s{\n", INDENT[2], project_name, INDENT[2]);
 fprintf(output, "%sreturn;\n%s}\n", INDENT[3], INDENT[2]);
 // Closing class declaration.
 fprintf(output, "%s};\n\n", INDENT[0]);
 fprintf(output, "#endif // _%s_STAGES_REF_H_\n", caps_project_name);
 fclose(output);
 return;
}

/*! Create stage module header file for single pipelined architectures. */
void CreateStgHeader(ac_stg_list* stage_list, char* pipe_name)
{
 extern char* project_name;
 extern int stage_num;
 ac_stg_list* pstage;
 char* caps_stage_name;
 char* stage_filename;
 char* stage_name;
 FILE* output;
 int i;

 for (pstage = stage_list; pstage != NULL; pstage = pstage->next)
 {
  // Stage module names are always PIPENAME_STAGENAME. The pipe created by an
  // ac_stage declaration is defined to be named "". --Marilia
  if (pipe_name)
  {
   caps_stage_name = (char*) malloc(sizeof(char) * (2 + strlen(pstage->name) + strlen(pipe_name)));
   stage_name = (char*) malloc(sizeof(char) * (2 + strlen(pstage->name) + strlen(pipe_name)));
   sprintf(stage_name, "%s_%s", pipe_name, pstage->name);
  }
  else
  {
   caps_stage_name = (char*) malloc(sizeof(char) * (2 + strlen(pstage->name)));
   stage_name = (char*) malloc(sizeof(char) * (2 + strlen(pstage->name)));
   sprintf(stage_name, "_%s", pstage->name);
  }
  i = 0;
  while (stage_name[i])
  {
   caps_stage_name[i] = toupper(stage_name[i]);
   i++;
  }
  caps_stage_name[i] = '\0';
  stage_filename = (char*) malloc(sizeof(char) * (4 + strlen(project_name) + strlen(stage_name)));
  sprintf(stage_filename, "%s_%s.H", project_name, stage_name);
  if (!(output = fopen(stage_filename, "w")))
  {
   perror("ArchC could not open output file");
   exit(1);
  }
  free(stage_filename);
  print_comment(output, "Stage module header file.");
  fprintf(output, "#ifndef _%s_%s_STAGE_H_\n", caps_project_name, caps_stage_name);
  fprintf(output, "#define _%s_%s_STAGE_H_\n", caps_project_name, caps_stage_name);
  fprintf(output, "\n");
  fprintf(output, "#include \"ac_stage.H\"\n");
  fprintf(output, "#include \"ac_instr.H\"\n");
  fprintf(output, "#include \"%s_isa.H\"\n", project_name);
  fprintf(output, "#include \"%s_arch.H\"\n", project_name);
  fprintf(output, "#include \"%s_arch_ref.H\"\n", project_name);
  if ((pstage->id == 1) && ACABIFlag)
   fprintf (output, "#include \"%s_syscall.H\"\n", project_name);
  fprintf(output, "\n");
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
  if ((pstage->id == 1) && ACGDBIntegrationFlag)
  {
   fprintf(output, "#include \"ac_gdb_interface.H\"\n");
   fprintf(output, "\n");
   fprintf(output, "%sextern ac_GDB* gdbstub;\n", INDENT[0]);
   fprintf(output, "\n");
  }
#endif
  // Declaring stage namespace.
  fprintf(output, "%snamespace %s_%s\n%s{\n", INDENT[0], project_name,
          stage_name, INDENT[0]);
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
  if ((pstage->id == 1) && ACGDBIntegrationFlag)
   fprintf(output, "%sclass %s_%s: public ac_stage<ac_instr<%s_parms::AC_DEC_FIELD_NUMBER>, %s_parms::ac_word, %s_parms::ac_Hword>, public %s_arch_ref, public ac_GDB_interface\n",
           INDENT[1], project_name, stage_name, project_name, project_name, project_name,
           project_name);
  else
#endif
   fprintf(output, "%sclass %s_%s: public sc_module, public ac_stage< ac_instr<%s_parms::AC_DEC_FIELD_NUMBER> >, public %s_arch_ref\n",
           INDENT[1], project_name, stage_name, project_name, project_name, project_name,
           project_name);
  // Declaring stage module.
  // It already includes the behavior method.
  // Regins and regouts are now pointers and belong in the parent class. --Marilia
  fprintf(output, "%s{\n", INDENT[1]);
  fprintf(output, "%sprivate:\n", INDENT[2]);
  fprintf(output, "%stypedef cache_item<%s_parms::AC_DEC_FIELD_NUMBER> cache_item_t;\n",
          INDENT[3], project_name);
  fprintf(output, "%stypedef ac_instr<%s_parms::AC_DEC_FIELD_NUMBER> ac_instr_t;\n",
          INDENT[3], project_name);
  fprintf(output, "%s%s_arch& ap;\n", INDENT[3], project_name);
  if ((pstage->id == 1) && ACABIFlag)
  {
   fprintf(output, "%sint flushes_left;\n", INDENT[3]);
   fprintf(output, "%s%s_syscall syscall;\n", INDENT[3], project_name);
  }
  fprintf(output, "\n");
  fprintf(output, "%spublic:\n", INDENT[2]);
  if (pstage->id == 1)
   if (ACDecCacheFlag)
    fprintf(output, "%scache_item_t* DEC_CACHE;\n", INDENT[3]);
  fprintf(output, "%s%s_parms::%s_isa& isa;\n", INDENT[3], project_name, project_name);
  if (pstage->id == 1)
   fprintf(output, "%sbool start_up;\n", INDENT[3]);
  fprintf(output, "%sunsigned id;\n", INDENT[3]);
  fprintf(output, "%ssc_in<bool> clock;\n", INDENT[3]);
  fprintf(output, "%ssc_event done;\n", INDENT[3]);
  fprintf(output, "%svoid behavior();\n", INDENT[3]);
  fprintf(output, "%sSC_HAS_PROCESS(%s_%s);\n", INDENT[3], project_name, stage_name);
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
  if ((pstage->id == 1) && ACGDBIntegrationFlag) // This stage might inherit from ac_GDB_interface, so it needs prototypes. --Marilia
   fprintf(output, "%svirtual int nRegs();\n%svirtual ac_word reg_read(int reg);\n%svirtual void reg_write(int reg, ac_word value);\n%svirtual unsigned char mem_read(unsigned int address);\n%svirtual void mem_write(unsigned int address, unsigned char byte);\n",
           INDENT[3], INDENT[3], INDENT[3], INDENT[3], INDENT[3]);
#endif
 fprintf(output, "\n");
  // Constructor. All stages have the same constructor parameter list, it's up to the caller to provide correct parameters. --Marilia
  fprintf(output, "%s%s_%s(sc_module_name _name, %s_arch& ra, %s_parms::%s_isa& i, ac_stage<ac_instr_t>* p, ac_sync_reg<ac_instr_t>* ri, ac_sync_reg<ac_instr_t>* ro): sc_module(_name), ac_stage<ac_instr_t>(_name, p, ri, ro), %s_arch_ref(ra), ap(ra)",
          INDENT[3], project_name, stage_name, project_name, project_name,
          project_name, project_name);
  if ((pstage->id == 1) && ACABIFlag)
   fprintf(output, ", syscall(ra)");
  fprintf(output, ", isa(i)\n%s{\n", INDENT[3]);
  fprintf(output, "%sid = %d;\n", INDENT[4], pstage->id);
  if (pstage->id == 1)
  {
   fprintf(output, "%sstart_up = 1;\n", INDENT[4]);
   if (ACABIFlag)
    fprintf(output, "%sflushes_left = %d;\n", INDENT[4], pipe_maximum_path_length);
   if (ACDecCacheFlag)
    fprintf(output, "%sDEC_CACHE = 0;\n", INDENT[4]);
   if (ACDasmFlag)
    fprintf(output, "%sap.dasmfile.open(\"%s.dasm\");\n", INDENT[4],
            project_name);
  }
  fprintf(output, "%sSC_METHOD(behavior);\n", INDENT[4]);
  fprintf(output, "%ssensitive_pos << clock;\n", INDENT[4]);
  fprintf(output, "%sdont_initialize();\n", INDENT[4]);
  fprintf(output, "%sreturn;\n", INDENT[4]); // End of constructor.
  fprintf(output, "%s}\n", INDENT[3]);
  if ((pstage->id == 1) && ACDecCacheFlag)
  {
   fprintf(output, "\n");
   fprintf(output, "%s~%s_%s()\n%s{\n", INDENT[3], project_name, stage_name,
           INDENT[3]);
   fprintf(output, "%sif (DEC_CACHE)\n%sfree(DEC_CACHE);\n%sreturn;\n%s}\n", INDENT[4], INDENT[5],
           INDENT[4], INDENT[3]);
   // End of destructor.
   fprintf(output, "\n");
   fprintf(output, "%svoid init_dec_cache()\n%s{\n%sDEC_CACHE = reinterpret_cast<cache_item_t*>(calloc(sizeof(cache_item_t), dec_cache_size));\n%sreturn;\n%s}\n",
           INDENT[3], INDENT[3], INDENT[4], INDENT[4], INDENT[3]);
  }
  fprintf(output, "%s};\n", INDENT[1]);
  // End of namespace.
  fprintf(output, "%s}\n\n", INDENT[0]);
  fprintf(output, "#endif // _%s_%s_STAGE_H_\n", caps_project_name, caps_stage_name);
  free(caps_stage_name);
  free(stage_name);
  fclose(output);
 }
 return;
}

/*! Create formatted registers header file. */
void CreateRegsHeader(void)
{
 extern ac_dec_format* format_reg_list;
 extern char* project_name;
 ac_dec_format* pformat;
 ac_dec_field* pfield;
 int flag = 1;
 FILE* output;
 char* filename;

 filename = (char*) malloc(sizeof(char) * (12 + strlen(project_name)));
 sprintf(filename, "%s_fmt_regs.H", project_name);
 for (pformat = format_reg_list; pformat != NULL; pformat = pformat->next)
 {
  if (flag)
  { // Print this just once.
   if (!(output = fopen(filename, "w")))
   {
    perror("ArchC could not open output file");
    exit(1);
   }
   print_comment(output, "ArchC formatted registers header file.");
   fprintf(output, "#ifndef _%s_FMT_REGS_H_\n", caps_project_name);
   fprintf(output, "#define _%s_FMT_REGS_H_\n", caps_project_name);
   fprintf(output, "\n");
   fprintf(output, "#include \"ac_sync_reg.H\"\n");
   fprintf(output, "#include <string>\n\n");
   fprintf(output, "#include \"%s_parms.H\"\n", project_name);
   COMMENT(INDENT[0], "ArchC classes for formatted registers.\n");
   flag = 0;
  }
  // Declaring formatted register class.
  fprintf(output, "%sclass %s_fmt_%s\n%s{\n", INDENT[0], project_name,
          pformat->name, INDENT[0]);
  fprintf(output, "%schar* name;\n", INDENT[2]);
  fprintf(output, "\n");
  fprintf(output, "%spublic:\n", INDENT[1]);
  // TO DO: Registers with parameterized size. The templated class ac_reg is still not
  //        working with sc_unit<x> types.
  for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
   fprintf(output, "%sac_sync_reg<unsigned long> %s;\n", INDENT[2], pfield->name);
  fprintf(output, "\n");
  // Declaring class constructor. Friggin' ugly, I know, let me know if you have a better way. --Marilia
  fprintf(output, "%s%s_fmt_%s(char* n): name(n)", INDENT[2], project_name,
          pformat->name);
  for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
   fprintf(output, ", %s(std::string(std::string(n) + std::string(\"_%s\")).c_str(), %d)",
           pfield->name, pfield->name, 0);
  // Constructor body.
  fprintf(output, "\n%s{\n", INDENT[2]);
  fprintf(output, "%sreturn;\n%s}\n\n", INDENT[3], INDENT[2]);
  // End of constructor.
  fprintf(output, "%svoid change_dump(ostream& output)\n%s{\n%sreturn;\n%s}\n\n",
          INDENT[2], INDENT[2], INDENT[3], INDENT[2]);
  fprintf(output, "%svoid reset_log()\n%s{\n%sreturn;\n%s}\n\n", INDENT[2],
          INDENT[2], INDENT[3], INDENT[2]);
  fprintf(output, "%svoid suspend()\n%s{\n", INDENT[2], INDENT[2]);
  for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
   fprintf(output, "%s%s.suspend();\n", INDENT[3], pfield->name);
  fprintf(output, "%sreturn;\n%s}\n\n", INDENT[3], INDENT[2]);
  fprintf(output, "%svoid behavior(%s_parms::ac_stage_list stage = static_cast<%s_parms::ac_stage_list>(0), int cycle = 0);\n",
          INDENT[2], project_name, project_name);
  fprintf(output, "%s};\n\n", INDENT[0]);
 }
 free(filename);
 if (!flag)
 { // We had at least one formatted reg declared.
#if 0 // This is intended to be used if and when behaviors for formatted registers are integrated into the simulator. --Marilia
  fprintf(output, "#ifndef ac_behavior\n");
  fprintf(output, "#define ac_behavior(instr) AC_BEHAVIOR_##instr()\n");
  for (pformat = format_reg_list; pformat != NULL; pformat = pformat->next)
   fprintf(output, "#define AC_BEHAVIOR_%s() %s_fmt_%s::behavior(%s_parms::ac_stage_list stage = static_cast<%s_parms::ac_stage_list>(0), int cycle = 0)\n",
           pformat->name, project_name, pformat->name, project_name, project_name);
  fprintf(output, "#endif // !ac_behavior\n");
#endif
  fprintf(output, "#endif // _%s_FMT_REGS_H_\n", caps_project_name);
  fclose(output);
 }
 return;
}

/*! Create the header file for ArchC co-verification class. */
void CreateCoverifHeader(void) // TODO -- maybe.
{
 extern ac_sto_list* storage_list;
 ac_sto_list* pstorage;
 FILE* output;
 char filename[] = "ac_verify.H";

 if (!(output = fopen( filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 print_comment(output, "ArchC Co-verification Class header file.");
 fprintf(output, "#ifndef _AC_VERIFY_H_\n");
 fprintf(output, "#define _AC_VERIFY_H_\n");
 fprintf(output, "\n");
 fprintf(output, "#include <fstream>\n");
 fprintf(output, "#include <list>\n");
 fprintf(output, "#include \"archc.H\"\n");
 fprintf(output, "#include \"ac_parms.H\"\n");
 fprintf(output, "#include \"ac_resources.H\"\n");
 fprintf(output, "#include \"ac_storage.H\"\n");
 fprintf(output, "\n");
 COMMENT(INDENT[0], "ArchC Co-verification class.\n");
 fprintf(output, "%sclass ac_verify:public ac_resources\n%s{\n", INDENT[0],
         INDENT[0]);
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  fprintf(output, "%slog_list %s_changes;\n", INDENT[2], pstorage->name);
 fprintf(output, "\n");
 fprintf(output, "%spublic:\n", INDENT[1]);
 fprintf(output, "%sofstream output;\n", INDENT[2]);
 // Printing log method.
 COMMENT(INDENT[2],"Logging structural model changes.");
 fprintf(output, "%svoid log(char* name, unsigned address, ac_word datum, double time)\n%s{\n",
         INDENT[2], INDENT[2]);
 fprintf(output, "%slog_list* pdevchg;\n", INDENT[3]);
 fprintf(output, "%s", INDENT[3]);
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
 {
  fprintf(output, "if (!strcmp(name, \"%s\"))\n", pstorage->name);
  fprintf(output, "%spdevchg = &%s_changes;\n", INDENT[4], pstorage->name);
  fprintf(output, "%selse\n", INDENT[3]);
 }
 fprintf(output, "%s{\n", INDENT[3]);
 fprintf(output, "%sAC_ERROR(\"Logging aborted! Unknown storage device used for verification: \" << name);\n",
         INDENT[4]);
 fprintf(output, "%sreturn;\n", INDENT[4]);
 fprintf(output, "%s}\n", INDENT[3]);
 fprintf(output, "%sadd_log(pdevchg, address, datum, time);\n", INDENT[3]);
 fprintf(output, "%sreturn;\n%s}\n", INDENT[3], INDENT[2]);
 fprintf(output, "\n");
 //Printing check_clock method.
 COMMENT(INDENT[2],"Checking device logs at a given simulation time");
 fprintf(output, "%svoid check_clock(double time)\n%s{\n", INDENT[2], INDENT[2]);
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  fprintf(output, "%smatch_logs(&%s, &%s_changes, time);\n", INDENT[3],
          pstorage->name, pstorage->name);
 fprintf(output, "%sreturn;\n%s}\n", INDENT[3], INDENT[2]);
 fprintf(output, "\n");
 //Printing checker_timed method.
 COMMENT(INDENT[2],"Finalize co-verification for timed model.");
 fprintf(output, "%svoid checker_timed(double time)\n%s{\n", INDENT[2],
         INDENT[2]);
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
 {
  fprintf(output, "%smatch_logs(&%s, &%s_changes, time);\n", INDENT[3],
          pstorage->name, pstorage->name);
  fprintf(output, "%scheck_final(&%s, &%s_changes);\n", INDENT[3],
          pstorage->name, pstorage->name);
 }
 fprintf(output, "%sreturn;\n%s}\n", INDENT[3], INDENT[2]);
 fprintf(output, "\n");
 // Printing checker method.
 COMMENT(INDENT[2],"Finalize co-verification for untimed model.");
 fprintf(output, "%svoid checker()\n%s{\n", INDENT[2], INDENT[2]);
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  fprintf(output, "%scheck_final(&%s, &%s_changes);\n", INDENT[3],
          pstorage->name, pstorage->name);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[3], INDENT[2]);
 // Printing class constructor.
 COMMENT(INDENT[2], "Constructor.");
 fprintf(output, "%sac_verify()\n%s{\n", INDENT[2], INDENT[2]);
 fprintf(output, "%soutput.open(\"ac_verification.log\");\n", INDENT[3]);
 fprintf(output, "%sreturn;\n%s}\n", INDENT[3], INDENT[2]);
 fprintf(output, "\n");
 // Printing add_log method.
 COMMENT(INDENT[2], "Logging structural model changes for a given device.");
 fprintf(output, "%svoid add_log(log_list* pdevchg, unsigned address, ac_word datum, double time);\n",
         INDENT[2]);
 // Printing match_logs method.
 COMMENT(INDENT[2], "Match device's behavioral and structural logs at a given simulation time.");
 fprintf(output, "%svoid match_logs(ac_storage* pdevice, log_list* pdevchange, double time);\n",
         INDENT[2]);
 // Printing check_final method.
 COMMENT(INDENT[2], "Check behavioral and structural logs for a given device in the end of simulation.");
 fprintf(output, "%svoid check_final(ac_storage* pdevice, log_list* pdevchange);\n",
         INDENT[2]);
 fprintf(output, "%s};\n", INDENT[0]);
 // End of file.
 fprintf(output, "#endif //_AC_VERIFY_H_\n");
 fclose(output);
 return;
}

//!Create the header file for ArchC statistics collection class.
void CreateStatsHeaderTmpl(void)
{
 extern ac_sto_list* storage_list;
 extern char* project_name;
 extern ac_dec_instr* instr_list;
 ac_dec_instr* pinstr;
 char filename[256];
 FILE* output;

 sprintf(filename, "%s_stats.H.tmpl", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 print_comment(output, "ArchC Processor statistics data header file.");
 fprintf(output, "#ifndef _%s_STATS_H_\n", caps_project_name);
 fprintf(output, "#define _%s_STATS_H_\n\n", caps_project_name);
 fprintf(output, "#include <fstream>\n");
 fprintf(output, "#include \"%s_parms.H\"\n", project_name);
 fprintf(output, "#include \"ac_stats.H\"\n");
 fprintf(output, "\n");
 // Declaring processor stats
 fprintf(output, "AC_SET_STATS(%s, INSTRUCTIONS, SYSCALLS);\n", project_name);
 // Declaring instruction stats
 fprintf(output, "AC_SET_INSTR_STATS(%s, COUNT);\n", project_name);
 fprintf(output, "\n");
 // Declaring proc_all_stats struct
 fprintf(output, "struct %s_all_stats\n%s{\n", project_name, INDENT[0]);
 // Declaring processor stats collector object
 fprintf(output, "%s%s_stats stats;\n", INDENT[2], project_name);
  // Declaring instruction stats collector objects
 for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
 {
  fprintf(output, "%s%s_instr_stats %s_istats;\n",
          INDENT[2], project_name, pinstr->name);
 }
 // Declaring instruction stats collector object array
 fprintf(output,
         "%s%s_instr_stats* instr_stats[%s_parms::AC_DEC_INSTR_NUMBER + 1];\n",
         INDENT[2], project_name, project_name);
 // Defining constructor
 fprintf(output, "%s%s_all_stats();\n", INDENT[2], project_name);
 // Closing proc_stats struct
 fprintf(output, "%s}; // struct %s_stats\n", INDENT[0], project_name);
 // END OF FILE!
 fprintf(output, "#endif // _%s_STATS_H_\n", caps_project_name);
 fclose(output);
 return;
}

/// Creates the header file for interrupt handlers.
void CreateIntrHeader(void)
{
 extern ac_sto_list* tlm_intr_port_list;
 extern char* project_name;
 extern char* caps_project_name;
 ac_sto_list *pport;
 char* filename;
 const char* description = "Interrupt Handlers header file.";
 FILE* output;

 filename = (char*) malloc(sizeof(char) * (17 + strlen(project_name)));
 sprintf(filename, "%s_intr_handlers.H", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, description);
 fprintf(output, "#ifndef _%s_INTR_HANDLERS_H\n", caps_project_name);
 fprintf(output, "#define _%s_INTR_HANDLERS_H\n", caps_project_name);
 fprintf(output, "\n");
 fprintf(output, "#include \"ac_intr_handler.H\"\n");
 fprintf(output, "#include \"%s_parms.H\"\n", project_name);
 fprintf(output, "#include \"%s_arch.H\"\n", project_name);
 fprintf(output, "#include \"%s_arch_ref.H\"\n", project_name);
 fprintf(output, "\n");
 /* Creating interrupt handler classes for each ac_tlm_intr_port */
 for (pport = tlm_intr_port_list; pport != NULL; pport = pport->next)
 {
  fprintf(output, "%sclass %s_%s_handler:\n", INDENT[0], project_name, pport->name);
  fprintf(output, "%spublic ac_intr_handler,\n", INDENT[1]);
  fprintf(output, "%spublic %s_arch_ref\n", INDENT[1], project_name);
  fprintf(output, "%s{\n", INDENT[0]);fprintf(output, "%spublic:\n", INDENT[1]);
  fprintf(output, "%sexplicit %s_%s_handler(%s_arch& ref): %s_arch_ref(ref) {}\n\n",
          INDENT[2], project_name, pport->name, project_name, project_name);
  fprintf(output, "%svoid handle(uint32_t value);\n\n", INDENT[2]);
  fprintf(output, "%s};\n\n", INDENT[0]);
 }
 fprintf(output, "#endif // _%s_INTR_HANDLERS_H_\n", caps_project_name);
 // End of file.
 fclose(output);
 return;
}

/// Creates the header file with ac_behavior macros for interrupt handlers.
void CreateIntrMacrosHeader(void)
{
 extern char* project_name;
 extern char* caps_project_name;
 char* filename;
 const char* description = "Interrupt Handlers ac_behavior macros header file.";
 FILE* output;

 filename = (char*) malloc(sizeof(char) * (17 + strlen(project_name)));
 sprintf(filename, "%s_ih_bhv_macros.H", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, description);
 fprintf(output, "#ifndef _%s_IH_BHV_MACROS_H_\n", caps_project_name);
 fprintf(output, "#define _%s_IH_BHV_MACROS_H_\n", caps_project_name);
 fprintf(output, "\n");
 /* ac_behavior main macro */
 fprintf(output, "#define ac_behavior(intrp, value) %s_##intrp##_handler::handle(uint32_t value)\n\n",
         project_name);
 fprintf(output, "#endif // _%s_IH_BHV_MACROS_H_\n", caps_project_name);
 // End of file.
 fclose(output);
 return;
}

/*! Create ArchC model syscalls. */
void CreateArchSyscallHeader(void)
{
 extern char* project_name;
 FILE* output;
 char* filename;

 filename = (char*) malloc(sizeof(char) * (11 + strlen(project_name)));
 sprintf(filename, "%s_syscall.H", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, "ArchC architecture-dependent syscall header file.");
 fprintf(output,
         "#ifndef _%s_SYSCALL_H_\n"
         "#define _%s_SYSCALL_H_\n"
         "\n"
         "#include \"%s_arch.H\"\n"
         "#include \"%s_arch_ref.H\"\n"
         "#include \"%s_parms.H\"\n"
         "#include \"ac_syscall.H\"\n"
         "\n"
         "// %s system calls.\n"
         "%sclass %s_syscall:\n"
         "%spublic ac_syscall<%s_parms::ac_word, %s_parms::ac_Hword>,"
         "%spublic %s_arch_ref\n"
         "%s{\n"
         "%spublic:\n"
         "%s%s_syscall(%s_arch& ref):\n"
         "%sac_syscall<%s_parms::ac_word, %s_parms::ac_Hword>(ref, %s_parms::AC_RAMSIZE),\n"
         "%s%s_arch_ref(ref)\n"
         "%s{\n"
         "%sreturn;\n"
         "%s}\n"
         "\n"
         "%svoid get_buffer(int argn, unsigned char* buf, unsigned int size);\n"
         "%svoid set_buffer(int argn, unsigned char* buf, unsigned int size);\n"
         "%svoid set_buffer_noinvert(int argn, unsigned char* buf, unsigned int size);\n"
         "%sint get_int(int argn);\n"
         "%svoid set_int(int argn, int val);\n"
         "%svoid return_from_syscall();\n"
         "%svoid set_prog_args(int argc, char** argv);\n"
         "};\n"
         "\n"
         "#endif // _%s_SYSCALL_H_ \n",
         caps_project_name, caps_project_name,
         project_name, project_name, project_name,
         project_name,
         INDENT[0], project_name,
         INDENT[1], project_name, project_name,
         INDENT[1], project_name,
         INDENT[0],
         INDENT[1],
         INDENT[2], project_name, project_name,
         INDENT[3], project_name, project_name, project_name,
         INDENT[3], project_name,
         INDENT[2],
         INDENT[3],
         INDENT[2],
         INDENT[2], INDENT[2], INDENT[2], INDENT[2], INDENT[2], INDENT[2],
         INDENT[2], caps_project_name);
 fclose(output);
 return;
}

/*! Create makefile. */
void CreateMakefile(void)
{
 extern ac_dec_format* format_ins_list;
 extern ac_pipe_list* pipe_list;
 extern char *project_name;
 extern int HaveMemHier, HaveFormattedRegs, HaveTLMPorts, HaveTLMIntrPorts;
 ac_dec_format* pformat;
 ac_stg_list* pstage;
 ac_pipe_list* ppipe;
 FILE* output;
 const char* make_filename = "Makefile.archc";

 if (!(output = fopen(make_filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 COMMENT_MAKE("####################################################");
 COMMENT_MAKE("This is the Makefile for building the %s ArchC model", project_name);
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
 fprintf(output, "\n");
 COMMENT_MAKE("Variable that points to SystemC installation path.");
 fprintf(output, "SYSTEMC :=%s\n", SYSTEMC_PATH);
 COMMENT_MAKE("Variable that points to ArchC installation path");
 fprintf(output, "ARCHC :=%s\n", BINDIR);
 COMMENT_MAKE("Target Arch used by SystemC");
 fprintf(output, "TARGET_ARCH :=%s\n", TARGET_ARCH);
 fprintf(output, "INC_DIR := -I. -I%s -I$(SYSTEMC)/include", INCLUDEDIR);
 if (HaveTLMPorts || HaveTLMIntrPorts)
  fprintf(output, " -I$(SYSTEMC)/include/sysc/tlm");
 fprintf(output, "\n");
 fprintf(output, "LIB_DIR := -L. -L$(SYSTEMC)/lib-$(TARGET_ARCH) -L%s\n", LIBDIR);
 fprintf(output, "\n");
 fprintf(output, "LIB_SYSTEMC := %s\n",
         (strlen(SYSTEMC_PATH) > 2) ? "-lsystemc" : "");
 fprintf(output, "LIBS := $(LIB_SYSTEMC) -lm $(EXTRA_LIBS) -larchc\n");
 fprintf(output, "CC := %s\n", CC_PATH);
 fprintf(output, "OPT := %s\n", OPT_FLAGS);
 fprintf(output, "DEBUG := %s\n", DEBUG_FLAGS);
 fprintf(output, "OTHER := %s\n", OTHER_FLAGS);
 fprintf(output, "CFLAGS := $(DEBUG) $(OPT) $(OTHER) %s\n",
         ((ACGDBIntegrationFlag) ? "-DUSE_GDB" : ""));
 fprintf(output, "\n");
 fprintf(output, "MODULE := %s\n", project_name);
 fprintf(output, "\n");
 // Declaring ACSRCS variable.
 COMMENT_MAKE("These are the source files automatically generated by ArchC, that must appear in the SRCS variable");
 fprintf(output, "ACSRCS := $(MODULE)_arch_ref.cpp");
 // Checking if we have a pipelined architecture or not.
 if (pipe_list) // Pipeline list exists. Used for ac_pipe declarations.
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
    fprintf(output, " $(MODULE)_%s_%s.cpp", ppipe->name, pstage->name);
 fprintf(output, " $(MODULE).cpp");
 fprintf(output, "\n\n");
 // Declaring ACINCS variable.
 COMMENT_MAKE("These are the source files automatically generated by ArchC that are included by other files in ACSRCS.");
 fprintf(output, "ACINCS := $(MODULE)_isa_init.cpp\n");
 fprintf(output, "\n");
 // Declaring ACHEAD variable.
 COMMENT_MAKE("These are the header files automatically generated by ArchC");
 fprintf(output, "ACHEAD := $(MODULE)_parms.H $(MODULE)_arch.H $(MODULE)_arch_ref.H $(MODULE)_isa.H $(MODULE)_bhv_macros.H");
 if (HaveFormattedRegs)
  fprintf(output, " $(MODULE)_fmt_regs.H");
 if (ACStatsFlag)
  fprintf(output, " %s_stats.H", project_name);
 if (pipe_list) // Pipeline list exists. Used for ac_pipe declarations.
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
    fprintf(output, " $(MODULE)_%s_%s.H", ppipe->name, pstage->name);
 if (ACABIFlag)
  fprintf(output, " $(MODULE)_syscall.H");
 fprintf(output, " $(MODULE).H");
 if (pipe_list)
  fprintf(output, " $(MODULE)_stages_ref.H");
 if (HaveTLMIntrPorts)
  fprintf(output, " $(MODULE)_intr_handlers.H $(MODULE)_ih_bhv_macros.H");
 fprintf(output, "\n\n");
 // Declaring FILES variable.
 COMMENT_MAKE("These are the source files provided by ArchC that must be compiled together with the ACSRCS.");
 COMMENT_MAKE("They are stored in the archc/src/aclib directory.");
 fprintf(output, "ACFILES := %s",
         ((ACGDBIntegrationFlag) ? " ac_gdb.cpp ac_breakpoints.cpp" : ""));
 if (HaveMemHier)
  fprintf(output, " ac_cache.cpp ac_mem.cpp ac_cache_if.cpp");
 fprintf(output, "\n\n");
 // Declaring ACLIBFILES variable
 COMMENT_MAKE("These are the library files provided by ArchC.");
 COMMENT_MAKE("They are stored in the archc/lib directory.");
 fprintf(output, "ACLIBFILES := ac_decoder_rt.o ac_module.o ac_storage.o ac_utils.o");
 if (ACABIFlag)
  fprintf(output, " ac_syscall.o");
 if (HaveTLMPorts)
  fprintf(output, " ac_tlm_port.o");
 if (HaveTLMIntrPorts)
  fprintf(output, " ac_tlm_intr_port.o");
 fprintf(output, "\n\n");
 //Declaring FILESHEAD variable
 COMMENT_MAKE("These are the headers files provided by ArchC.");
 COMMENT_MAKE("They are stored in the archc/include directory.");
 fprintf(output, "ACFILESHEAD := $(ACFILES:.cpp=.H) $(ACLIBFILES:.o=.H) ac_sync_reg.H ac_regbank.H ac_debug_model.H ac_sighandlers.H ac_ptr.H ac_memport.H ac_arch.H ac_arch_dec_if.H ac_arch_ref.H ac_stage.H");
 if (ACABIFlag)
  fprintf(output, " ac_syscall.H");
 if (HaveTLMPorts)
  fprintf(output, " ac_tlm_port.H");
 if (HaveTLMIntrPorts)
  fprintf(output, " ac_tlm_intr_port.H");
 if (HaveTLMPorts || HaveTLMIntrPorts)
  fprintf(output, " ac_tlm_protocol.H");
 if (ACStatsFlag)
  fprintf(output, " ac_stats.H ac_stats_base.H");
 fprintf(output, "\n\n");
 // Declaring SRCS variable.
 COMMENT_MAKE("These are the source files provided by the user + ArchC sources.");
 fprintf(output, "SRCS := main.cpp $(ACSRCS) $(ACFILES) $(MODULE)_isa.cpp%s",
         ((ACGDBIntegrationFlag) ? " $(MODULE)_gdb_funcs.cpp" : ""));
 if (ACABIFlag)
  fprintf(output, " $(MODULE)_syscall.cpp");
 if (HaveTLMIntrPorts)
  fprintf(output, " $(MODULE)_intr_handlers.cpp");
 if (ACStatsFlag)
  fprintf(output, " $(MODULE)_stats.cpp");
 fprintf(output, "\n\n");
 // Declaring OBJS variable.
 fprintf(output, "OBJS := $(SRCS:.cpp=.o)");
 fprintf(output, "\n\n");
 // Declaring executable name.
 fprintf(output, "EXE := $(MODULE).x");
 fprintf(output, "\n\n");
 // Declaring dependence rules.
 fprintf(output, ".SUFFIXES: .cc .cpp .o .x\n\n");
 fprintf(output, "all: $(addprefix %s/, $(ACFILESHEAD)) $(ACHEAD) $(ACFILES) $(EXE)\n\n", INCLUDEDIR);
 fprintf(output, "$(EXE): $(OBJS) %s\n",
         (strlen(SYSTEMC_PATH) > 2) ? "$(SYSTEMC)/lib-$(TARGET_ARCH)/libsystemc.a" : "");
 fprintf(output, "\t$(CC) $(CFLAGS) $(INC_DIR) $(LIB_DIR) -o $@ $(OBJS) $(LIBS) 2>&1 | c++filt\n\n");
 COMMENT_MAKE("Copy from template if main.cpp not exist.");
 fprintf(output, "main.cpp:\n");
 fprintf(output, "\tcp main.cpp.tmpl main.cpp\n\n");
 if (ACStatsFlag)
 {
  COMMENT_MAKE("Copy from template if %s_stats.H not exist.", project_name);
  fprintf(output, "%s_stats.H:\n", project_name);
  fprintf(output, "\tcp %s_stats.H.tmpl %s_stats.H\n\n", project_name, project_name);
  COMMENT_MAKE("Copy from template if %s_stats.cpp not exist.", project_name);
  fprintf(output, "%s_stats.cpp:\n", project_name);
  fprintf(output, "\tcp %s_stats.cpp.tmpl %s_stats.cpp\n\n", project_name, project_name);
 }
 if (HaveTLMIntrPorts)
 {
  COMMENT_MAKE("Copy from template if %s_intr_handlers.cpp not exist.",
               project_name);
  fprintf(output, "%s_intr_handlers.cpp:\n", project_name);
  fprintf(output, "\tcp %s_intr_handlers.cpp.tmpl %s_intr_handlers.cpp\n\n",
          project_name, project_name);
 }
 fprintf(output, ".cpp.o:\n");
 fprintf(output, "\t$(CC) $(CFLAGS) $(INC_DIR) -c $<\n\n");
 fprintf(output, ".cc.o:\n");
 fprintf(output, "\t$(CC) $(CFLAGS) $(INC_DIR) -c $<\n\n");
 fprintf(output, "clean:\n");
 fprintf(output, "\trm -f $(OBJS) *~ $(EXE) core *.o\n\n");
 fprintf(output, "model_clean:\n");
 fprintf(output, "\trm -f $(ACSRCS) $(ACHEAD) $(ACINCS) $(ACFILESHEAD) $(ACFILES) *.tmpl loader.ac\n\n");
 fprintf(output, "sim_clean: clean model_clean\n\n");
 fprintf(output, "distclean: sim_clean\n");
 fprintf(output, "\trm -f main.cpp Makefile.archc\n\n");
 fclose(output);
 return;
}

/////////////////////////// Create Implementation Functions ////////////////////////////

/*! Create architecture-specific resources header file. */
void CreateArchRefImpl(void)
{
 extern char* project_name;
 extern ac_pipe_list* pipe_list;
 extern ac_sto_list* storage_list;
 extern ac_dec_format* format_reg_list;
 extern int HaveMultiCycleIns;
 ac_pipe_list* ppipe;
 ac_stg_list* pstage;
 ac_sto_list* pstorage;
 ac_dec_format* pformat;
 ac_dec_field* pfield;
 FILE* output;
 char* filename;

 filename = malloc(sizeof(char) * (14 + strlen(project_name)));
 sprintf(filename, "%s_arch_ref.cpp", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  free(filename);
  exit(1);
 }
 free(filename);
 print_comment(output, "ArchC arch-specific references implementation file.");
 fprintf(output, "#include \"%s_arch.H\"\n", project_name);
 fprintf(output, "#include \"%s_arch_ref.H\"\n", project_name);
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
 {
  fprintf(output, "#include \"ac_msgbuf.H\"\n");
  fprintf(output, "#include <sys/ipc.h>\n");
  fprintf(output, "#include <unistd.h>\n");
  fprintf(output, "#include <sys/msg.h>\n");
  fprintf(output, "#include <sys/types.h>\n");
 }
#endif
 fprintf(output, "\n");
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
 if (ACGDBIntegrationFlag)
 {
  fprintf(output, "#ifndef PORT_NUM\n" );
  fprintf(output, "#define PORT_NUM 5000 // Port to which GDB binds for remote debugging.\n");
  fprintf(output, "#endif // PORT_NUM\n" );
 }
 fprintf(output, "\n");
#endif
 // Constructor.
 COMMENT(INDENT[0], "Default constructor.");
 fprintf(output, "%s%s_arch_ref::%s_arch_ref(%s_arch& arch): ac_arch_ref<%s_parms::ac_word, %s_parms::ac_Hword>(arch), p(arch)",
         INDENT[0], project_name, project_name, project_name, project_name, project_name);
 /* Finding storage devices */
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  fprintf(output, ", %s(arch.%s)", pstorage->name, pstorage->name);
 // The cycle control variable. Only available for multi-cycle archs.
 if (HaveMultiCycleIns)
  fprintf(output, ", ac_cycle(arch.ac_cycle)");
 fprintf(output, ", ac_pc(arch.ac_pc)");
 if (ACDasmFlag)
  fprintf(output, ", dasmfile(arch.dasmfile)");
 fprintf(output, "\n%s{\n", INDENT[0]);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 // End of constructor.
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
 {
  int ndevice = 0;
  // Set co-verification msg queue method.
  fprintf(output, "%svoid %s_arch_ref::set_queue(char* exec_name)\n%s{\n",
          INDENT[0], project_name, INDENT[0]);
  fprintf(output, "%sstruct start_msgbuf sbuf;\n", INDENT[1]);
  fprintf(output, "%sstruct dev_msgbuf dbuf;\n", INDENT[1]);
  fprintf(output, "%sextern key_t key;\n", INDENT[1]);
  fprintf(output, "%sextern int msqid;\n\n", INDENT[1]);
  fprintf(output, "%sif ((key = ftok(exec_name, 'A')) == -1)\n%s{\n", INDENT[1],
          INDENT[1]);
  fprintf(output, "%sAC_ERROR(\"Could not attach to the co-verification msg queue. Process: \" << getpid());\n",
          INDENT[2]);
  fprintf(output, "%sperror(\"ftok\");\n", INDENT[2]);
  fprintf(output, "%sexit(1);\n", INDENT[2]);
  fprintf(output, "%s}\n", INDENT[1]);
  fprintf(output, "%sif ((msqid = msgget(key, 0644)) == -1)\n%s{\n", INDENT[1],
          INDENT[1]);
  fprintf(output, "%sAC_ERROR(\"Could not attach to the co-verification msg queue. Process: \" << getpid());\n",
          INDENT[2]);
  fprintf(output, "%sperror(\"msgget\");\n", INDENT[2]);
  fprintf(output, "%sexit(1);\n", INDENT[2]);
  fprintf(output, "%s}\n", INDENT[1]);
  for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  {
   if ((pstorage->type == MEM) || (pstorage->type == ICACHE) ||
       (pstorage->type == DCACHE) || (pstorage->type == CACHE) ||
       (pstorage->type == REGBANK))
    ndevice++;
  }
  fprintf(output, "%ssbuf.mtype = 1;\n", INDENT[1]);
  fprintf(output, "%ssbuf.ndevice = %d;\n", INDENT[1], ndevice);
  fprintf(output, "%sif (msgsnd(msqid, reinterpret_cast<void*>(&sbuf), sizeof(sbuf), 0) == -1)\n",
          INDENT[1]);
  fprintf(output, "%sperror(\"msgsnd\");\n", INDENT[2]);
  fprintf(output, "%sdbuf.mtype = 2;\n", INDENT[1]);
  for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  {
   if ((pstorage->type == MEM) || (pstorage->type == ICACHE) ||
       (pstorage->type == DCACHE) || (pstorage->type == CACHE) ||
       (pstorage->type == REGBANK))
   {
    fprintf(output, "%sstrcpy(dbuf.name,%s.get_name());\n", INDENT[1],
            pstorage->name);
    fprintf(output, "%sif (msgsnd(msqid, reinterpret_cast<void*>(&dbuf), sizeof(dbuf), 0) == -1)\n",
            INDENT[1]);
    fprintf(output, "%sperror(\"msgsnd\");\n", INDENT[2]);
   }
  }
  fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]); // End of set_queue.
 }
#endif
 // Delegated ac_pc read access.
 COMMENT(INDENT[2], "Delegated read access to ac_pc.");
 fprintf(output, "%sunsigned %s_arch_ref::get_ac_pc()\n%s{\n%sreturn static_cast<unsigned>(ac_pc.read());\n%s}\n",
         INDENT[2], project_name, INDENT[2], INDENT[3], INDENT[2]); // End of get_ac_pc method.
 fclose(output);
 return;
}

/*! Create stage module implementation file for single pipelined architectures. */
void CreateStgImpl(ac_stg_list* stage_list, char* pipe_name)
{
 extern char* project_name;
 extern int stage_num;
 ac_stg_list* pstage;
 extern ac_dec_instr* instr_list;
 ac_dec_instr* pinstr;
 extern ac_dec_format *format_ins_list;
 ac_dec_format *pformat;
 extern ac_dec_field* common_instr_field_list;
 ac_dec_field *pfield;
 int base_indent;
 char* stage_filename;
 char* stage_name;
 FILE* output;

 for (pstage = stage_list; pstage != NULL; pstage = pstage->next)
 {
  // Stage module names are always PIPENAME_STAGENAME. The pipe created by an
  // ac_stage declaration is defined to be named "". --Marilia
  if (pipe_name)
  {
   stage_name = (char*) malloc(sizeof(char) * (2 + strlen(pstage->name) + strlen(pipe_name)));
   sprintf(stage_name, "%s_%s", pipe_name, pstage->name);
  }
  else
  {
   stage_name = (char*) malloc(sizeof(char) * (2 + strlen(pstage->name)));
   sprintf(stage_name, "_%s", pstage->name);
  }
  stage_filename = (char*) malloc(sizeof(char) * (6 + strlen(project_name) + strlen(stage_name)));
  sprintf(stage_filename, "%s_%s.cpp", project_name, stage_name);
  if (!(output = fopen(stage_filename, "w")))
  {
   perror("ArchC could not open output file");
   exit(1);
  }
  free(stage_filename);
  print_comment(output, "Stage module implementation file.");
  fprintf(output, "#include \"%s_%s.H\"\n", project_name, stage_name);
#if 0 // Co-verification is currently unmaintained. --Marilia
  if ((pstage->id == 1) && ACVerifyFlag)
  {
   fprintf(output, "#include \"ac_msgbuf.H\"\n");
   fprintf(output, "#include \"sys/msg.h\"\n");
  }
#endif
  // Emitting stage behavior method implementation.
  fprintf(output, "\n%sinline void %s_%s::%s_%s::behavior()\n%s{\n", INDENT[0],
          project_name, stage_name, project_name, stage_name, INDENT[0]);
  fprintf(output, "%sunsigned ins_id;\n", INDENT[1]);
  if (pstage->id == 1)
   fprintf(output, "%sunsigned* instr_dec;\n", INDENT[1]);
  fprintf(output, "%sac_instr_t* instr_vec;\n", INDENT[1]);
  if (pstage->id != 1)
  {
   fprintf(output, "\n");
   fprintf(output, "%sif (ac_stop_flag)\n%sreturn;\n", INDENT[1], INDENT[2]);
   fprintf(output, "%sif (is_stalled())\n%sctrl.request_update();\n", INDENT[1], INDENT[2]);
   fprintf(output, "%sinstr_vec = new ac_instr_t(regin->read());\n", INDENT[1]);
   fprintf(output, "%sins_id = instr_vec->get(IDENT);\n", INDENT[1]);
   fprintf(output, "%sif (ins_id != 0)\n%s{\n", INDENT[1], INDENT[1]);
   fprintf(output, "%sisa.current_instruction_id = ins_id;\n", INDENT[2]);
   fprintf(output, "%sisa._behavior_instruction(static_cast<%s_parms::ac_stage_list>(id), 0",
           INDENT[2], project_name);
   /* common_instr_field_list has the list of fields for the generic instruction. */
   for (pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next)
    fprintf(output, ", instr_vec->get(%d)", pfield->id);
   fprintf(output, ");\n");
   fprintf(output, "%sswitch (ins_id)\n%s{\n", INDENT[2], INDENT[2]);
   for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
   {
    fprintf(output, "%scase %d: // %s\n", INDENT[3], pinstr->id, pinstr->name);
    // Format behavior call.
    fprintf(output, "%sisa._behavior_%s_%s(static_cast<%s_parms::ac_stage_list>(id), 0",
            INDENT[4], project_name, pinstr->format, project_name);
    for (pformat = format_ins_list;
         (pformat != NULL) && strcmp(pinstr->format, pformat->name);
         pformat = pformat->next);
    for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
     fprintf(output, ", instr_vec->get(%d)", pfield->id);
    fprintf(output, ");\n");
    // Instruction behavior call.
    fprintf(output, "%sisa.behavior_%s(static_cast<%s_parms::ac_stage_list>(id), 0",
            INDENT[4], pinstr->name, project_name);
    for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
     fprintf(output, ", instr_vec->get(%d)", pfield->id);
    fprintf(output, ");\n");
    fprintf(output, "%sbreak;\n", INDENT[4]);
   }
   fprintf(output, "%s}\n", INDENT[2]);
   fprintf(output, "%s}\n", INDENT[1]);
   if (pstage->id != stage_num)
    fprintf(output, "%sregout->write(*instr_vec);\n", INDENT[1]);
   fprintf(output, "%sdelete instr_vec;\n", INDENT[1]);
   fprintf(output, "%sdone.notify();\n", INDENT[1]);
   fprintf(output, "%sreturn;\n", INDENT[1]);
   fprintf(output, "%s}\n", INDENT[0]);
  }
  // First stage needs to fetch and decode instructions.
  else
  {
   if (ACDebugFlag)
   {
    fprintf(output, "%sextern bool ac_do_trace;\n", INDENT[1]);
    fprintf(output, "%sextern ofstream trace_file;\n", INDENT[1]);
   }
   if (ACABIFlag)
    fprintf(output, "%sstatic ac_instr_t* the_nop = new ac_instr_t;\n", INDENT[1]);
#if 0 // Co-verification is currently unmaintained. --Marilia
   if (ACVerifyFlag)
   {
    fprintf(output, "%sextern int msqid;\n", INDENT[1]);
    fprintf(output, "%sstruct log_msgbuf end_log;\n", INDENT[1]);
   }
#endif
#if 0 // Deleted. --Marilia
   if (ac_host_endian == 0)
    fprintf(output, "%schar fetch[AC_WORDSIZE / 8];\n", INDENT[1]);
#endif
   if (ACDecCacheFlag)
    fprintf(output, "%scache_item_t* ins_cache;\n", INDENT[1]);
   fprintf(output, "\n");
   fprintf(output, "%sif (ac_stop_flag)\n%sreturn;\n", INDENT[1], INDENT[2]);
   fprintf(output, "%sif (is_stalled())\n%sctrl.request_update();\n", INDENT[1], INDENT[2]);
   EmitFetchInit(output, 1);
   base_indent = 2;
   if (ACABIFlag)
   {
    base_indent += 2;
    // Emitting system call handler.
    COMMENT(INDENT[2], "Handling system calls.");
    fprintf(output, "%sif (ap.decode_pc %% 2)\n%sap.decode_pc--;\n", INDENT[2],
            INDENT[3]);
    fprintf(output, "%sswitch (ap.decode_pc)\n%s{\n", INDENT[2], INDENT[2]);
    EmitPipeABIDefine(output);
    EmitABIAddrList(output, 3);
    fprintf(output, "%sdefault:\n", INDENT[3]);
   }
   EmitDecodification(output, base_indent);
   fetch_stage = pstage;
   EmitInstrExec(output, base_indent);
   fprintf(output, "%sac_instr_counter++;\n", INDENT[base_indent]);
   if (ACABIFlag)
   {
    // Closing default case.
    fprintf(output, "%sbreak;\n", INDENT[4]);
    // Closing switch.
    fprintf(output, "%s}\n", INDENT[2]);
   }
   // Closing else.
   fprintf(output, "%s}\n", INDENT[1]);
   fprintf(output, "%sdone.notify();\n", INDENT[1]);
   fprintf(output, "%sreturn;\n%s}\n", INDENT[1], INDENT[0]);
  }
  free(stage_name);
  fclose(output);
 }
 return;
}

/*! Create processor implementation file. */
void CreateProcessorImpl(void)
{
 extern ac_pipe_list* pipe_list;
 extern ac_sto_list* storage_list;
 extern char* project_name;
 extern int HaveMultiCycleIns, HaveMemHier;
 ac_sto_list* pstorage;
 ac_stg_list* pstage;
 ac_pipe_list* ppipe;
 char* filename;
 FILE* output;

 filename = (char*) malloc(sizeof(char) * (5 + strlen(project_name)));
 sprintf(filename, "%s.cpp", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, "Processor module implementation file.");
 fprintf(output, "#include \"%s_parms.H\"\n", project_name);
 fprintf(output, "#include \"%s_arch.H\"\n", project_name);
 fprintf(output, "#include \"%s.H\"\n", project_name);
 if (!pipe_list && ACABIFlag)
  fprintf(output, "#include \"%s_syscall.H\"\n", project_name);
 fprintf(output, "#include <signal.h>\n");
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
 {
  fprintf(output, "#include \"ac_msgbuf.H\"\n");
  fprintf(output, "#include \"sys/msg.h\"\n");
 }
#endif
 fprintf(output, "\n");
 // Signal handling.
 fprintf(output, "#include <ac_sighandlers.H>\n\n");
 // init() and stop() methods.
 // init() with 3 parameters.
 COMMENT(INDENT[0], "Initialization (2 arguments).");
 fprintf(output, "%svoid %s::init(int ac, char** av)\n%s{\n",
         INDENT[0], project_name, INDENT[0]);
 fprintf(output, "%sextern char* appfilename;\n", INDENT[1]);
 fprintf(output, "%sac_init_opt(ac, av);\n", INDENT[1]);
 fprintf(output, "%sac_init_app(ac, av);\n", INDENT[1]);
 fprintf(output, "%sAPP_MEM->load(appfilename);\n", INDENT[1]);
 fprintf(output,
         "%stime_step = period / (sc_get_default_time_unit()).to_double();\n",
         INDENT[1]);
 fprintf(output, "%sset_args(ac_argc, ac_argv);\n", INDENT[1]);
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
  fprintf(output, "%sset_queue(av[0]);\n", INDENT[1]);
#endif
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
 if (ACGDBIntegrationFlag)
 {
  fprintf(output, "%sif (gdbstub && !gdbstub->is_disabled())\n", INDENT[1]);
  fprintf(output, "%sgdbstub->connect();\n", INDENT[2]);
 }
#endif
 fprintf(output, "%sisa._behavior_begin();\n", INDENT[1]);
 fprintf(output,
         "%scerr << \"ArchC: -------------------- Starting Simulation --------------------\" << endl;\n",
         INDENT[1]);
 fprintf(output, "%sInitStat();\n\n", INDENT[1]);
 fprintf(output, "%ssignal(SIGINT, sigint_handler);\n", INDENT[1]);
 fprintf(output, "%ssignal(SIGTERM, sigint_handler);\n", INDENT[1]);
 fprintf(output, "%ssignal(SIGSEGV, sigsegv_handler);\n", INDENT[1]);
 fprintf(output, "%ssignal(SIGUSR1, sigusr1_handler);\n", INDENT[1]);
 fprintf(output, "#ifdef USE_GDB\n");
 fprintf(output, "%ssignal(SIGUSR2, sigusr2_handler);\n", INDENT[1]);
 fprintf(output, "#endif\n");
 fprintf(output, "%sset_running();\n", INDENT[1]);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 // init() with 2 parameters.
 COMMENT(INDENT[0], "Initialization (no arguments).");
 fprintf(output, "%svoid %s::init()\n%s{\n",
         INDENT[0], project_name, INDENT[0]);
 fprintf(output, "%stime_step = period / (sc_get_default_time_unit()).to_double();\n", INDENT[1]);
 fprintf(output, "%sset_args(ac_argc, ac_argv);\n", INDENT[1]);
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
  fprintf(output, "%sset_queue(av[0]);\n", INDENT[1]);
#endif
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
 if (ACGDBIntegrationFlag)
 {
  fprintf(output, "%sif (gdbstub && !gdbstub->is_disabled())\n", INDENT[1]);
  fprintf(output, "%sgdbstub->connect();\n", INDENT[2]);
 }
#endif
 fprintf(output, "%sisa._behavior_begin();\n", INDENT[1]);
 fprintf(output,
         "%scerr << \"ArchC: -------------------- Starting Simulation --------------------\" << endl;\n",
         INDENT[1]);
 fprintf(output, "%sInitStat();\n\n", INDENT[1]);
 fprintf(output, "%ssignal(SIGINT, sigint_handler);\n", INDENT[1]);
 fprintf(output, "%ssignal(SIGTERM, sigint_handler);\n", INDENT[1]);
 fprintf(output, "%ssignal(SIGSEGV, sigsegv_handler);\n", INDENT[1]);
 fprintf(output, "%ssignal(SIGUSR1, sigusr1_handler);\n", INDENT[1]);
 fprintf(output, "#ifdef USE_GDB\n");
 fprintf(output, "%ssignal(SIGUSR2, sigusr2_handler);\n", INDENT[1]);
 fprintf(output, "#endif\n");
 fprintf(output, "%sset_running();\n", INDENT[1]);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 // stop().
 COMMENT(INDENT[0], "Stop simulation (may receive exit status).");
 fprintf(output, "%svoid %s::stop(int status)\n%s{\n", INDENT[0], project_name,
         INDENT[0]);
 fprintf(output,
         "%scerr << \"ArchC: -------------------- Simulation Finished --------------------\" << endl;\n",
         INDENT[1]);
 fprintf(output, "%sisa._behavior_end();\n", INDENT[1]);
 fprintf(output, "%sac_stop_flag = 1;\n", INDENT[1]);
 fprintf(output, "%sac_exit_status = status;\n", INDENT[1]);
 fprintf(output, "%sset_stopped();\n", INDENT[1]);
 fprintf(output, "%sac_arch<%s_parms::ac_word, %s_parms::ac_Hword>::PrintStat();\n",
         INDENT[1], project_name, project_name);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 // load().
 fprintf(output, "%svoid %s::load(char* program)\n%s{\n", INDENT[0],
         project_name, INDENT[0]);
 fprintf(output, "%sAPP_MEM->load(program);\n", INDENT[1]);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 // delayed_load().
 fprintf(output, "%svoid %s::delayed_load(char* program)\n%s{\n", INDENT[0],
         project_name, INDENT[0]);
 fprintf(output, "%shas_delayed_load = true;\n", INDENT[1]);
 fprintf(output, "%sdelayed_load_program = new char[strlen(program)];\n", INDENT[1]);
 fprintf(output, "%sstrcpy(delayed_load_program, program);\n", INDENT[1]);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 // PrintStat wrapper.
 COMMENT(INDENT[0], "PrintStat wrapper method.");
 fprintf(output, "%svoid %s::PrintStat()\n%s{\n", INDENT[0], project_name,
         INDENT[0]);
 fprintf(output,
         "%sac_arch<%s_parms::ac_word, %s_parms::ac_Hword>::PrintStat();\n",
         INDENT[1], project_name, project_name);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 // Verification method.
 COMMENT(INDENT[0], "Verification method.");
 fprintf(output, "%svoid %s::ac_verify()\n%s{\n", INDENT[0], project_name, INDENT[0]);
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
 {
  fprintf(output, "%sextern int msqid;\n", INDENT[1]);
  fprintf(output, "%sstruct log_msgbuf lbuf;\n", INDENT[1]);
  fprintf(output, "%slog_list::iterator itor;\n", INDENT[1]);
  fprintf(output, "%slog_list* plog;\n\n", INDENT[1]);
 }
#endif
 fprintf(output, "#ifdef AC_VERBOSE\n");
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  fprintf(output, "%s%s.change_dump(cerr);\n", INDENT[1], pstorage->name);
 fprintf(output, "#endif\n");
 fprintf(output, "#ifdef AC_UPDATE_LOG\n");
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
 {
  int next_type = 3;

  fprintf(output, "%sif (sc_simulation_time())\n%s{\n", INDENT[1], INDENT[1]);
  // Sending logs for every storage device. We just consider for co-verification caches, regbanks and memories.
  for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
   if ((pstorage->type == MEM) || (pstorage->type == ICACHE) ||
       (pstorage->type == DCACHE) || (pstorage->type == CACHE) ||
       (pstorage->type == REGBANK))
   {
    fprintf(output, "%splog = %s.get_changes();\n", INDENT[2], pstorage->name);
    fprintf(output, "%sif (plog->size())\n%s{\n", INDENT[2], INDENT[2]);
    fprintf(output, "%sitor = plog->begin();\n", INDENT[3]);
    fprintf(output, "%slbuf.mtype = %d;\n", INDENT[3], next_type);
    fprintf(output, "%swhile (itor != plog->end())\n%s{\n", INDENT[3],
            INDENT[3]);
    fprintf(output, "%slbuf.log = *itor;\n", INDENT[4]);
    fprintf(output, "%sif (msgsnd(msqid, reinterpret_cast<struct log_msgbuf*>(&lbuf), sizeof(lbuf), 0) == -1)\n",
            INDENT[4]);
    fprintf(output, "%sperror(\"msgsnd\");\n", INDENT[5]);
    fprintf(output, "%sitor = plog->erase(itor);\n", INDENT[4]);
    fprintf(output, "%s}\n", INDENT[3]);
    fprintf(output, "%s}\n", INDENT[2]);
			 next_type++;
   }
  fprintf(output, "%s}\n", INDENT[1]);
 }
#endif
 for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next)
  //fprintf(output, "%s%s.change_save();\n", INDENT[1], pstorage->name);
    fprintf(output, "%s%s.reset_log();\n", INDENT[1], pstorage->name);
 fprintf(output, "#endif\n");
 if (pipe_list)
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage->next != NULL; pstage = pstage->next)
    if (ppipe->next || pstage->next)
    {
    fprintf(output, "%sif (%s_%s.will_stall())\n%s{\n", INDENT[1],
            ppipe->name, pstage->name, INDENT[1]);
    fprintf(output, "%sac_pc.suspend();\n", INDENT[2]);
    fprintf(output, "%sac_instr_counter--;\n", INDENT[2]);
    fprintf(output, "%s}\n", INDENT[1]);
   }
 fprintf(output, "%sreturn;\n%s}\n", INDENT[1], INDENT[0]);
 fprintf(output, "\n");
 if (!pipe_list)
 {
  fprintf(output, "%svoid %s::behavior()\n%s{\n", INDENT[0], project_name,
          INDENT[0]);
  fprintf(output, "%sif (ac_stop_flag)\n%sreturn;\n", INDENT[1], INDENT[2]);
  // Since main() bails out if the model isn't pipelined or muticycle, I'm just assuming it's multicycle. --Marilia
  fprintf(output, "%s%s_arch& ap = *this;\n", INDENT[1], project_name); // I'm shameless, I know. --Marilia
  if (ACDebugFlag)
  {
   fprintf(output, "%sextern bool ac_do_trace;\n", INDENT[1]);
   fprintf(output, "%sextern ofstream trace_file;\n", INDENT[1]);
  }
  if (ACVerifyFlag)
  {
   fprintf(output, "%sextern int msqid;\n", INDENT[1]);
   fprintf(output, "%sstruct log_msgbuf end_log;\n", INDENT[1]);
  }
  if (ACABIFlag)
   fprintf(output, "%s%s_syscall syscall(*ap);\n", INDENT[1], project_name);
  if (ACDecCacheFlag)
   fprintf (output, "%scache_item_t* ins_cache;\n", INDENT[1]);
  if (ac_host_endian == 0)
   fprintf(output, "%schar fetch[%s_parms::AC_WORDSIZE / 8];\n\n", INDENT[1], project_name);
  if (HaveMemHier)
  {
   fprintf(output, "%sif (ac_wait_sig)\n", INDENT[1]);
   fprintf(output, "%sreturn;\n", INDENT[2]);
  }
  // Emitting processor behavior method implementation.
  EmitMultiCycleProcessorBhv(output);
  fprintf(output, "%sac_verify();\n", INDENT[1]);
  fprintf(output, "%sreturn;\n%s}\n", INDENT[1], INDENT[0]);
 }
 if (!pipe_list && ACDecCacheFlag)
  fprintf(output, "\n%svoid %s::init_dec_cache()\n%s{\n%sDEC_CACHE = reinterpret_cast<cache_item_t*>(calloc(sizeof(cache_item_t), dec_cache_size));\n%sreturn;\n%s}\n",
          INDENT[0], project_name, INDENT[0], INDENT[1], INDENT[1], INDENT[0]);
 // End of file.
 fclose(output);
 return;
}

/*! Create template for the .cpp file where the user has the basic code for the main function. */
void CreateMainTmpl(void)
{
 extern char* project_name;
 extern ac_pipe_list* pipe_list;
 ac_pipe_list* ppipe;
 ac_stg_list* pstage;
 char* description;
 FILE* output;

 description = (char*) malloc(sizeof(char) * (43 + strlen(project_name)));
 sprintf(description, "This is the main file for the %s ArchC model", project_name);
 if (!(output = fopen(main_filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 print_comment(output, description);
 free(description);
 fprintf(output, "%sconst char* project_name = \"%s\";\n", INDENT[0],
         project_name);
 fprintf(output, "%sconst char* project_file = \"%s\";\n", INDENT[0],
         arch_filename);
 fprintf(output, "%sconst char* archc_version = \"%s\";\n", INDENT[0], ACVersion);
 fprintf(output, "%sconst char* archc_options = \"%s\";\n", INDENT[0], ACOptions);
 fprintf(output, "\n");
 fprintf(output, "#include <systemc.h>\n");
 fprintf(output, "#include \"ac_stats_base.h\"\n");
 fprintf(output, "#include \"%s.H\"\n", project_name);
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
 if (ACGDBIntegrationFlag)
 {
  fprintf(output, "#include \"ac_gdb.H\"\n\n");
  fprintf(output, "%sAC_GDB* gdbstub;\n", INDENT[0]);
 }
#endif
 fprintf(output, "\n");
 fprintf(output, "%sint sc_main(int ac, char** av)\n%s{\n", INDENT[0],
         INDENT[0]);
 COMMENT(INDENT[1], "Clock.");
 fprintf(output, "%ssc_clock clk(\"clk\", 20, 0.5, true);\n", INDENT[1]);
 COMMENT(INDENT[1], "ISA simulator.");
 fprintf(output, "%s%s %s_p0(\"%s\", clk.period().to_double());\n\n", INDENT[1], project_name,
         project_name, project_name);
 fprintf(output, "%s%s_p0.clock(clk.signal());\n", INDENT[1], project_name,
         project_name);
#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
 if (ACGDBIntegrationFlag)
 {
  if (pipe_list)
  {
   for (ppipe = pipe_list; ppipe ! = NULL; ppipe = ppipe->next)
    for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
     if (pstage->id == 1)
      break;
   fprintf(output, "%sgdbstub = new AC_GDB(%s_p0.%s_%s_stage, %s_p0, PORT_NUM);\n\n",
           INDENT[1], project_name, ppipe->name, pstage->name, project_name);
  }
  else
   fprintf(output, "%sgdbstub = new AC_GDB(%s_p0, %s_p0, PORT_NUM);\n\n",
           INDENT[1], project_name, project_name);
 }
#endif
 fprintf(output, "#ifdef AC_DEBUG\n");
 fprintf(output, "%sac_trace(\"%s.trace\");\n", INDENT[1], project_name);
 fprintf(output, "#endif\n");
 fprintf(output, "%s%s_p0.init(ac, av);\n",
         INDENT[1], project_name);
 fprintf(output, "%ssc_start(-1); // Starts SystemC simulation.\n", INDENT[1]);
 fprintf(output, "#ifdef AC_STATS\n");
 fprintf(output, "%sac_stats_base::print_all_stats(std::cerr);\n", INDENT[1], project_name);
 fprintf(output, "#endif\n");
 fprintf(output, "#ifdef AC_DEBUG\n");
 fprintf(output, "%sac_close_trace();\n", INDENT[1]);
 fprintf(output, "#endif\n");
 fprintf(output, "%sreturn %s_p0.ac_exit_status;\n%s}\n", INDENT[1],
         project_name, INDENT[0]);
 return;
}

/*! Create template for the .cpp file where the user has to fill out the instruction and format behaviors. */
void CreateImplTmpl(void)
{
 extern ac_dec_format* format_ins_list;
 extern ac_pipe_list* pipe_list;
 extern ac_dec_instr* instr_list;
 extern char* project_name;
 ac_dec_format* pformat;
 ac_dec_instr* pinstr;
 ac_pipe_list* ppipe;
 ac_stg_list* pstage;
 char* filename;
 // File containing ISA declaration
 FILE* output;

 filename = (char*) malloc(sizeof(char) * (14 + strlen(project_name)));
 sprintf(filename, "%s_isa.cpp.tmpl", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, isa_template_description);
 fprintf(output, "#include \"%s_isa.H\"\n", project_name);
 fprintf(output, "#include \"%s_isa_init.cpp\"\n", project_name);
 fprintf(output, "#include \"%s_bhv_macros.H\"\n", project_name);
 if (pipe_list) // Stage includes. Yeah, ugly, but what else can I do?
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
    fprintf(output, "#include \"%s_%s_%s.H\"\n", project_name, ppipe->name,
            pstage->name);
 fprintf(output, "\n\n");
 COMMENT(INDENT[0], "'using namespace' statement to allow access to all %s-specific datatypes", project_name);
 fprintf(output, "using namespace %s_parms;\n\n", project_name);
 // Behavior to begin simulation.
 COMMENT(INDENT[0], "Behavior executed before simulation begins.");
 fprintf(output, "%svoid ac_behavior(begin)\n%s{\n%sreturn;\n%s}\n\n",
         INDENT[0], INDENT[0], INDENT[1], INDENT[0]);
 // Behavior to end simulation.
 COMMENT(INDENT[0], "Behavior executed after simulation ends.");
 fprintf(output, "%svoid ac_behavior(end)\n%s{\n%sreturn;\n%s}\n\n",
         INDENT[0], INDENT[0], INDENT[1], INDENT[0]);
 // Declaring ac_instruction behavior method.
 COMMENT(INDENT[0], "Generic instruction behavior method.");
 // Testing if should emit a switch or not.
 if (pipe_list)
 {
  fprintf(output, "%svoid ac_behavior(instruction)\n%s{\n", INDENT[0],
          INDENT[0]);
  fprintf(output, "%sswitch (stage)\n%s{\n", INDENT[1], INDENT[1]);
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
   {
    fprintf(output, "%scase id_%s_%s:\n", INDENT[2], ppipe->name, pstage->name);
    fprintf(output, "%sbreak;\n", INDENT[3]);
   }
  fprintf(output, "%sdefault:\n", INDENT[2]);
  fprintf(output, "%s}\n", INDENT[1]);
  fprintf(output, "%sreturn;\n%s}\n", INDENT[1], INDENT[0]);
 }
 else
  fprintf(output, "%svoid ac_behavior(instruction)\n%s{\n%sreturn;\n%s}\n\n",
          INDENT[0], INDENT[0], INDENT[1], INDENT[0]);
 // Declaring instruction format behavior methods.
 COMMENT(INDENT[0], "Instruction format behavior methods.");
 for (pformat = format_ins_list; pformat != NULL; pformat = pformat->next)
  // Testing if should emit a switch or not.
  if (pipe_list)
  {
   fprintf(output, "\n%svoid ac_behavior(%s)\n%s{\n", INDENT[0], pformat->name,
           INDENT[0]);
   fprintf(output, "%sswitch (stage)\n%s{\n", INDENT[1], INDENT[1]);
   for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
    for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
    {
     fprintf(output, "%scase id_%s_%s:\n", INDENT[2], ppipe->name, pstage->name);
     fprintf(output, "%sbreak;\n", INDENT[3]);
    }
   fprintf(output, "%sdefault:\n", INDENT[2]);
   fprintf(output, "%s}\n", INDENT[1]);
   fprintf(output, "%sreturn;\n%s}\n", INDENT[1], INDENT[0]);
  }
  else
   fprintf(output, "\n%svoid ac_behavior(%s)\n%s{\n%sreturn;\n%s}\n", INDENT[0],
           pformat->name, INDENT[0], INDENT[1], INDENT[0]);
 // Declaring each instruction behavior method.
 for (pinstr = instr_list; pinstr!= NULL; pinstr = pinstr->next)
  // Testing if should emit a switch or not.
  if (pipe_list)
  {
   fprintf(output, "\n%svoid ac_behavior(%s)\n%s{\n", INDENT[0], pinstr->name,
           INDENT[0]);
   fprintf(output, "%sswitch (stage)\n%s{\n", INDENT[1], INDENT[1]);
   for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
    for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
    {
     fprintf(output, "%scase id_%s_%s:\n", INDENT[2], ppipe->name, pstage->name);
     fprintf(output, "%sbreak;\n", INDENT[3]);
    }
   fprintf(output, "%sdefault:\n", INDENT[2]);
   fprintf(output, "%s}\n", INDENT[1]);
   fprintf(output, "%sreturn;\n%s}\n", INDENT[1], INDENT[0]);
  }
  else
   fprintf(output, "\n%svoid ac_behavior(%s)\n%s{\n%sreturn;\n%s}\n", INDENT[0],
           pinstr->name, INDENT[0], INDENT[1], INDENT[0]);
 // End of file.
 fclose(output);
 return;
}

/*! Create template for the .cpp file for GDB integration. */
void CreateArchGDBImplTmpl(void)
{
 extern ac_pipe_list* pipe_list;
 extern char* project_name;
 ac_stg_list* pstage;
 ac_pipe_list* ppipe;
 char* filename;
 char* if_name;
 FILE* output;
 int i;

 filename = malloc(sizeof(char) * (19 + strlen(project_name)));
 sprintf(filename, "%s_gdb_functions.cpp", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 i = 0;
 if (pipe_list)
 {
  for (ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next)
   for (pstage = ppipe->stages; pstage != NULL; pstage = pstage->next)
    if (pstage->id == 1)
    {
     if_name = (char*) malloc(sizeof(char) * (3 + strlen(project_name) + strlen(ppipe->name) + strlen(pstage->name)));
     sprintf(if_name, "%s_%s_%s", project_name, ppipe->name, pstage->name);
     break;
    }
 }
 else
  if_name = project_name;
 fprintf(output, "#include \"%s.H\"\n\n", if_name);
 fprintf(output, "%sint %s::nRegs()\n%s{\n", INDENT[0], if_name, INDENT[0]);
 fprintf(output, "%s}\n\n", INDENT[0]);
 fprintf(output, "%sac_word %s::reg_read()\n%s{\n", INDENT[0], if_name,
         INDENT[0]);
 fprintf(output, "%s}\n\n", INDENT[0]);
 fprintf(output, "%svoid %s::reg_write()\n%s{\n", INDENT[0], if_name,
         INDENT[0]);
 fprintf(output, "%s}\n\n", INDENT[0]);
 fprintf(output, "%sunsigned char %s::mem_read()\n%s{\n", INDENT[0], if_name,
         INDENT[0]);
 fprintf(output, "%s}\n\n", INDENT[0]);
 fprintf(output, "%svoid %s::mem_write()\n%s{\n", INDENT[0], if_name,
         INDENT[0]);
 fprintf(output, "%s}\n", INDENT[0]);
 if (if_name != project_name)
  free(if_name);
 fclose(output);
 return;
}

/*! Create ISA initialization file. */
void CreateISAInitImpl(void)
{
 extern char* project_name;
 extern ac_dec_instr* instr_list;
 extern ac_grp_list* group_list;
 ac_dec_instr* pinstr;
 ac_grp_list* pgroup;
 ac_instr_ref_list* pref;
 char* filename;
 FILE* output;

 // Now writing ISA initialization file.
 filename = malloc(sizeof(char) * (14 + strlen(project_name)));
 sprintf(filename, "%s_isa_init.cpp", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, "ISA initialization file.");
 fprintf(output, "#include \"%s_parms.H\"\n\n", project_name);
 fprintf(output, "#include \"%s_isa.H\"\n\n", project_name);
 for (pgroup = group_list; pgroup != NULL; pgroup = pgroup->next)
 {
  COMMENT(INDENT[0], "Group %s table initialization.", pgroup->name);
  fprintf(output, "%sbool %s_parms::%s_isa::group_%s[%s_parms::AC_DEC_INSTR_NUMBER] =\n%s{\n",
          INDENT[0], project_name, project_name, pgroup->name, project_name, INDENT[1]);
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
  {
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
 EmitDecStruct(output);
 fclose(output);
 return;
}

/*! Create template for the .cpp file for ABI implementation. */
void CreateArchABIImplTmpl(void)
{
 extern char* project_name;
 char* filename;
 FILE* output;

 filename = malloc(sizeof(char) * (18 + strlen(project_name)));
 sprintf(filename, "%s_syscall.cpp.tmpl", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 fprintf(output, "#include \"%s_syscall.H\"\n\n", project_name);
 fprintf(output, "%svoid %s_syscall::get_buffer(int argn, unsigned char* buf, unsigned int size)\n%s{\n",
         INDENT[0], project_name, INDENT[0]);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 fprintf(output, "%svoid %s_syscall::set_buffer(int argn, unsigned char* buf, unsigned int size)\n%s{\n",
         INDENT[0], project_name, INDENT[0]);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 fprintf(output, "%svoid %s_syscall::set_buffer_noinvert(int argn, unsigned char* buf, unsigned int size)\n%s{\n",
         INDENT[0], project_name, INDENT[0]);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 fprintf(output, "%sint %s_syscall::get_int(int argn)\n%s{\n\n%s}\n\n",
         INDENT[0], project_name, INDENT[0], INDENT[0]);
 fprintf(output, "%svoid %s_syscall::set_int(int argn, int val)\n%s{\n",
         INDENT[0], project_name, INDENT[0]);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 fprintf(output, "%svoid %s_syscall::return_from_syscall()\n%s{\n", INDENT[0],
         project_name, INDENT[0]);
 fprintf(output, "%sreturn;\n%s}\n\n", INDENT[1], INDENT[0]);
 fprintf(output, "%svoid %s_syscall::set_prog_args(int argc, char** argv)\n%s{\n",
         INDENT[0], project_name, INDENT[0]);
 fprintf(output, "%sreturn;\n%s}\n", INDENT[1], INDENT[0]);
 fclose(output);
 return;
}

//!Create the implementation file for ArchC statistics collection class.
void CreateStatsImplTmpl()
{
 extern char* project_name;
 extern ac_dec_instr* instr_list;
 ac_dec_instr* pinstr;
 char filename[256];
 FILE* output;

 sprintf(filename, "%s_stats.cpp.tmpl", project_name);
 if (!(output = fopen( filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 print_comment(output, "ArchC Processor statistics data implementation file.");
 fprintf(output, "#include \"%s_stats.H\"\n", project_name);
 fprintf(output, "\n");
 // Defining processor stat list
 fprintf(output, "AC_CONF_STAT_LIST(%s, INSTRUCTIONS, SYSCALLS);\n",
         project_name);
 // Defining instruction stat list
 fprintf(output, "AC_CONF_INSTR_STAT_LIST(%s, COUNT);\n\n", project_name);
  // Defining constructor
 fprintf(output, "%s%s_all_stats::%s_all_stats():\n", INDENT[0], project_name,
         project_name);
 fprintf(output, "%sstats(\"%s\")", INDENT[1], project_name);
 for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
  fprintf(output, ",\n%s%s_istats(\"%s\", stats)",
          INDENT[1], pinstr->name, pinstr->name);
 fprintf(output, "\n%s{\n", INDENT[0]);
 COMMENT(INDENT[1], "Configuring stats collectors for each instruction");
 for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
  fprintf(output, "%sinstr_stats[%d] = &%s_istats;\n",
          INDENT[1], pinstr->id, pinstr->name);
 fprintf(output, "%s}\n", INDENT[0]);
 // END OF FILE!
 fclose(output);
 return;
}

/*! Create formatted registers implementation file. */
void CreateRegsImpl(void)
{
 extern ac_dec_format* format_reg_list;
 extern char* project_name;
 ac_dec_format* pformat;
 ac_dec_field* pfield;
 int flag = 1;
 FILE* output;
 char* filename;

 filename = (char*) malloc(sizeof(char) * (19 + strlen(project_name)));
 sprintf(filename, "%s_fmt_regs.cpp.tmpl", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, fregs_template_description);
 fprintf(output, "#include \"%s_fmt_regs.H\"\n", project_name);
 fprintf(output, "#include \"archc.H\"\n");
 fprintf(output, "\n");
 // Declaring formatted registers behavior methods.
 for (pformat = format_reg_list; pformat != NULL; pformat = pformat->next)
  fprintf(output, "%svoid ac_behavior(%s)\n%s{\n%sreturn;\n%s}\n\n", INDENT[0],
          pformat->name, INDENT[0], INDENT[1], INDENT[0]);
 // End of file.
 fclose(output);
 return;
}

///Creates the .cpp template file for interrupt handlers.
void CreateIntrTmpl(void)
{
 extern ac_sto_list* tlm_intr_port_list;
 extern char* project_name;
 ac_sto_list *pport;
 FILE* output;
 char* filename;
 const char* description = "Interrupt Handlers implementation file.";

 filename = (char*) malloc(sizeof(char) * (24 + strlen(project_name)));
 sprintf(filename, "%s_intr_handlers.cpp.tmpl", project_name);
 if (!(output = fopen(filename, "w")))
 {
  perror("ArchC could not open output file");
  exit(1);
 }
 free(filename);
 print_comment(output, description);
 fprintf(output, "#include \"ac_intr_handler.H\"\n");
 fprintf(output, "#include \"%s_intr_handlers.H\"\n", project_name);
 fprintf(output, "#include \"%s_ih_bhv_macros.H\"\n\n", project_name);
 COMMENT(INDENT[0], "'using namespace' statement to allow access to all %s-specific datatypes", project_name);
 fprintf(output, "using namespace %s_parms;\n\n", project_name);
 // Declaring formatted register behavior methods.
 for (pport = tlm_intr_port_list; pport != NULL; pport = pport->next)
 {
  COMMENT(INDENT[0], "Interrupt handler behavior for interrupt port %s.", pport->name);
  fprintf(output, "%svoid ac_behavior(%s, value)\n%s{\n%sreturn;\n%s}\n\n",
          INDENT[0], pport->name, INDENT[0], INDENT[1], INDENT[0]);
 }
 // End of file.
 fclose(output);
 return;
}

////////////////////////////////////////////////////////////////////////////////////
// Emit functions ...                                                             //
// These Functions are used by the Create functions declared above to write files //
////////////////////////////////////////////////////////////////////////////////////

/**************************************/
/*! Emit declaration of decoding structures
  Used by CreateISAInitImpl function.   */
/***************************************/
void EmitDecStruct(FILE* output)
{
 extern ac_dec_format* format_ins_list;
 extern ac_dec_instr* instr_list;
 extern char* project_name;
 extern int declist_num;
 extern int HaveMultiCycleIns;
 extern int wordsize;
 ac_dec_field* pdecfield;
 ac_dec_format* pformat;
 ac_dec_instr* pinstr;
 ac_dec_list* pdeclist;
 int i;
 int count_fields;

 // Field structure.
 i = 0;
 fprintf(output, "%sac_dec_field %s_parms::%s_isa::fields[%s_parms::AC_DEC_FIELD_NUMBER] =\n%s {\n",
         INDENT[0], project_name, project_name, project_name, INDENT[0]);
 for (pformat = format_ins_list; pformat != NULL; pformat = pformat->next)
  for (pdecfield = pformat->fields; pdecfield != NULL; pdecfield = pdecfield->next)
  {
   i++;
   fprintf(output, "%s{\"%s\", %d, %d, %d, %ld, %d, ", INDENT[1],
           pdecfield->name, pdecfield->size, pdecfield->first_bit,
           pdecfield->id, pdecfield->val, pdecfield->sign);
   if (pdecfield->next)
    fprintf(output, "&(%s_parms::%s_isa::fields[%d])},\n", project_name, project_name, i);
   else
   {
    fprintf(output, "NULL}");
    if (pformat->next)
     fprintf(output, ",");
    else
     fprintf(output, "\n%s };\n", INDENT[0]);
    fprintf(output, "\n");
   }
  }
 // Format structure.
 i = 0;
 count_fields = 0;
 fprintf(output, "%sac_dec_format %s_parms::%s_isa::formats[%s_parms::AC_DEC_FORMAT_NUMBER] =\n%s {\n",
         INDENT[0], project_name, project_name, project_name, INDENT[0]);
 for (pformat = format_ins_list; pformat != NULL; pformat = pformat->next)
 {
  i++;
  fprintf(output, "%s{\"%s\", %d, &(%s_parms::%s_isa::fields[%d]), ", INDENT[1],
          pformat->name, pformat->size, project_name, project_name, count_fields);
  if (pformat->next)
   fprintf(output, "&(%s_parms::%s_isa::formats[%d])},\n", project_name, project_name, i);
  else
   fprintf(output, "NULL}\n%s };\n\n", INDENT[0]);
  for (pdecfield = pformat->fields; pdecfield != NULL; pdecfield = pdecfield->next)
   count_fields++;
 }
 // Decode list structure.
 i = 0;
 fprintf(output, "%sac_dec_list %s_parms::%s_isa::dec_list[%s_parms::AC_DEC_LIST_NUMBER] =\n%s {\n",
         INDENT[0], project_name, project_name, project_name, INDENT[0]);
 for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
  for (pdeclist = pinstr->dec_list; pdeclist != NULL; pdeclist = pdeclist->next)
  {
   i++;
   fprintf(output, "%s{\"%s\", %d, ", INDENT[1], pdeclist->name,
           pdeclist->value);
   if (pdeclist->next)
    fprintf(output, "&(%s_parms::%s_isa::dec_list[%d])},\n", project_name, project_name, i);
   else
   {
    fprintf(output, "NULL}");
    if (pinstr->next)
     fprintf(output, ",");
    else
     fprintf(output, "\n%s };\n", INDENT[0]);
    fprintf(output, "\n");
   }
  }
 declist_num = i;
// Instruction structure.
 i = 0;
 count_fields = 0;
 fprintf(output, "%sac_dec_instr %s_parms::%s_isa::instructions[%s_parms::AC_DEC_INSTR_NUMBER] =\n%s {\n",
         INDENT[0], project_name, project_name, project_name, INDENT[0]);
 for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
 {
  i++;
  fprintf(output, "%s{\"%s\", %d, \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, &(%s_parms::%s_isa::dec_list[%d]), 0, ",
          INDENT[1], pinstr->name, pinstr->size, pinstr->mnemonic,
          pinstr->asm_str, pinstr->format, pinstr->id, pinstr->cycles,
          pinstr->min_latency, pinstr->max_latency, project_name, project_name, count_fields);
  if (pinstr->next)
   fprintf(output, "&(%s_parms::%s_isa::instructions[%d])},\n", project_name, project_name, i);
  else
   fprintf(output, "NULL}\n%s };\n\n", INDENT[0]);
  for (pformat = format_ins_list; pformat != NULL; pformat = pformat->next)
   if (!strcmp(pformat->name, pinstr->format))
    break;
  for (pdeclist = pinstr->dec_list; pdeclist != NULL; pdeclist = pdeclist->next)
   count_fields++;
 }
 // Instruction information structure.
 fprintf(output, "%sconst ac_instr_info %s_parms::%s_isa::instr_table[%s_parms::AC_DEC_INSTR_NUMBER + 1] =\n%s {\n",
         INDENT[0], project_name, project_name, project_name, INDENT[0]);
 fprintf(output, "%sac_instr_info(0, \"_ac_invalid_\", \"_ac_invalid_\", %d, 0, 0, 0)",
         INDENT[1], (wordsize / 8));
 for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
  fprintf(output, ",\n%sac_instr_info(%d, \"%s\", \"%s\", %d, %d, %d, %d)",
          INDENT[1], pinstr->id, pinstr->name, pinstr->mnemonic, pinstr->size,
          pinstr->cycles, pinstr->min_latency, pinstr->max_latency);
 fprintf(output, "\n%s };\n", INDENT[0]);
 return;
}

/**************************************/
/*!  Emits the if statement that handles instruction decodification
  \brief Used by EmitMultiCycleProcessorBhv and CreateStgImpl functions */
/***************************************/
void EmitDecodification(FILE* output, int base_indent)
{
 extern ac_pipe_list* pipe_list;
 extern char* project_name;
 extern int wordsize, fetchsize, HaveMemHier;

 if (HaveMemHier)
 {
  if (fetchsize == wordsize)
   fprintf(output, "%s*(reinterpret_cast<ac_fetch*>(fetch)) = IM->read(ap.decode_pc);\n",
           INDENT[base_indent]);
  else if (fetchsize == (wordsize / 2))
   fprintf(output, "%s*(reinterpret_cast<ac_fetch*>(fetch)) = IM->read_half(ap.decode_pc);\n",
           INDENT[base_indent]);
  else if (fetchsize == 8)
   fprintf(output, "%s*(reinterpret_cast<ac_fetch*>(fetch)) = IM->read_byte(ap.decode_pc);\n",
           INDENT[base_indent]);
  else
  {
   AC_ERROR("Fetchsize differs from wordsize or (wordsize/2) or 8: not implemented.");
   exit(EXIT_FAILURE);
  }
  fprintf(output, "%sif (ac_wait_sig)\n", INDENT[base_indent]);
  fprintf(output, "%sreturn;\n", INDENT[base_indent + 1]);
 }
 if (ACDecCacheFlag)
 {
  fprintf(output, "%sins_cache = DEC_CACHE + ap.decode_pc;\n",
          INDENT[base_indent]);
  fprintf(output, "%sif (!ins_cache->valid)\n%s{\n", INDENT[base_indent],
          INDENT[base_indent]);
  base_indent++;
 }
 if (!HaveMemHier)
 {
#if 0 // What is supposed to go in here??? --Marilia
  if (fetchsize == wordsize)
   fprintf(output, "%s*(reinterpret_cast<ac_fetch*>(fetch)) = IM->read(decode_pc);\n",
           INDENT[base_indent]);
  else if (fetchsize == (wordsize / 2))
   fprintf(output, "%s*(reinterpret_cast<ac_fetch*>(fetch)) = IM->read_half(decode_pc);\n",
           INDENT[base_indent]);
  else if (fetchsize == 8)
   fprintf(output, "%s*(reinterpret_cast<ac_fetch*>(fetch)) = IM->read_byte(decode_pc);\n",
           INDENT[base_indent]);
  else
  {
   AC_ERROR("Fetchsize differs from wordsize or (wordsize/2) or 8: not implemented.");
   exit(EXIT_FAILURE);
  }
#endif
 }
 fprintf(output, "%sap.quant = 0;\n", INDENT[base_indent]);
 if (ACDecCacheFlag)
 {
  fprintf(output,
          "%sins_cache->instr_p = new ac_instr_t((isa.decoder)->Decode(reinterpret_cast<unsigned char*>(ap.buffer), ap.quant));\n",
          INDENT[base_indent], project_name);
  fprintf(output, "%sins_cache->valid = 1;\n", INDENT[base_indent]);
  base_indent--;
  fprintf(output, "%s}\n", INDENT[base_indent]);
  fprintf(output, "%sinstr_vec = ins_cache->instr_p;\n", INDENT[base_indent]);
 }
 else
 {
  fprintf(output,
          "%sinstr_dec = (isa.decoder)->Decode(reinterpret_cast<unsigned char*>(ap.buffer), ap.quant);\n",
          INDENT[base_indent]);
  fprintf(output, "%sinstr_vec = new ac_instr_t(instr_dec);\n",
          INDENT[base_indent], project_name);
 }
 // Checking if it is a valid instruction.
 fprintf(output, "%sins_id = instr_vec->get(IDENT);\n",
         INDENT[base_indent]);
 fprintf(output, "%sif (ins_id == 0)\n%s{\n", INDENT[base_indent],
         INDENT[base_indent]);
 fprintf(output, "%scerr << \"ArchC Error: Unidentified instruction. \" << endl;\n",
         INDENT[base_indent + 1]);
 fprintf(output, "%scerr << \"PC = \" << hex << ap.decode_pc << dec << endl;\n",
         INDENT[base_indent + 1]);
 fprintf(output, "%sap.stop();\n", INDENT[base_indent + 1]);
 if (pipe_list)
  fprintf(output, "%sdone.notify();\n", INDENT[base_indent + 1]);
 fprintf(output, "%sreturn;\n", INDENT[base_indent + 1]);
 fprintf(output, "%s}\n", INDENT[base_indent]);
 fprintf(output, "%sisa.current_instruction_id = ins_id;\n", INDENT[base_indent]);
 return;
}

/**************************************/
/*!  Emit code for executing instructions
  \brief Used by CreateStgImpl function */
/***************************************/
void EmitInstrExec(FILE* output, int base_indent)
{
 extern char* project_name;
 extern ac_pipe_list* pipe_list;
 extern ac_stg_list* fetch_stage;
 extern ac_dec_instr* instr_list;
 extern ac_dec_format* format_ins_list;
 extern ac_dec_field* common_instr_field_list;
 extern int HaveCycleRange;
 ac_dec_instr* pinstr;
 ac_dec_format* pformat;
 ac_dec_field* pfield;

#if 0 // GDB integration is currently not supported for the cycle-accurate simulator. --Marilia
 if (ACGDBIntegrationFlag)
  fprintf(output, "%sif (gdbstub && gdbstub->stop(decode_pc))\n%sgdbstub->process_bp();\n",
          INDENT[base_indent], INDENT[base_indent + 1]);
#endif
 // Pipelined archs can annul an instruction through pipelining flushing.
 // Generic behavior call.
 if (pipe_list)
 {
  fprintf(output, "%sisa._behavior_instruction(static_cast<%s_parms::ac_stage_list>(id), 0",
          INDENT[base_indent], project_name);
  /* common_instr_field_list has the list of fields for the generic instruction. */
  for (pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next)
   fprintf(output, ", instr_vec->get(%d)", pfield->id);
  fprintf(output, ");\n");
 }
 else
 {
  fprintf(output, "%sisa._behavior_instruction(", INDENT[base_indent]);
  /* common_instr_field_list has the list of fields for the generic instruction. */
  for (pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next)
  {
   fprintf(output, "instr_vec->get(%d)", pfield->id);
   if (pfield->next)
    fprintf(output, ", ");
  }
  fprintf(output, ");\n");
 }
 fprintf(output, "%sswitch (ins_id)\n%s{\n", INDENT[base_indent],
         INDENT[base_indent]);
 for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
 {
  fprintf(output, "%scase %d: // %s\n", INDENT[base_indent + 1], pinstr->id, pinstr->name);
  // Format behavior call.
  if (pipe_list)
   fprintf(output, "%sisa._behavior_%s_%s(static_cast<%s_parms::ac_stage_list>(id), 0, ",
           INDENT[base_indent + 2], project_name, pinstr->format, project_name);
  else
   fprintf(output, "%sif(!ac_annul_sig)\n%sisa._behavior_%s_%s(",
           INDENT[base_indent + 2], INDENT[base_indent + 3], project_name,
           pinstr->format);
  for (pformat = format_ins_list;
       (pformat != NULL) && strcmp(pinstr->format, pformat->name);
       pformat = pformat->next);
  for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
  {
   fprintf(output, "instr_vec->get(%d)", pfield->id);
   if (pfield->next)
    fprintf(output, ", ");
  }
  fprintf(output, ");\n");
  // Instruction behavior call.
  if (pipe_list)
   fprintf(output, "%sisa.behavior_%s(static_cast<%s_parms::ac_stage_list>(id), 0, ",
           INDENT[base_indent + 2], pinstr->name, project_name);
  else
   fprintf(output, "%sif(!ac_annul_sig)\n%sisa.behavior_%s(",
           INDENT[base_indent + 2], INDENT[base_indent + 3], pinstr->name);
  for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
  {
   fprintf(output, "instr_vec->get(%d)", pfield->id);
   if (pfield->next)
    fprintf(output, ", ");
  }
  fprintf(output, ");\n");
  fprintf(output, "%sbreak;\n", INDENT[base_indent + 2]);
 }
 fprintf(output, "%s}\n", INDENT[base_indent]);
 if (ACDasmFlag)
  fprintf(output, PRINT_DASM, INDENT[base_indent], project_name, project_name);
 if (ACStatsFlag)
 {
  if (!pipe_list)
  {
   fprintf(output, "%sif ((!ac_annul_sig) && (!ac_wait_sig))\n%s{\n",
           INDENT[base_indent], INDENT[base_indent]);
   base_indent++;
  }
  fprintf(output, "%sisa.stats[%s_stat_ids::INSTRUCTIONS]++;\n",
          INDENT[base_indent], project_name);
  fprintf(output, "%s(*(isa.instr_stats[ins_id]))[%s_instr_stat_ids::COUNT]++;\n",
          INDENT[base_indent], project_name);
  // If cycle range for instructions were declared, include them on the statistics.
#if 0 // No cycle counting as of yet. --Marilia
  if (HaveCycleRange)
  {
   fprintf(output, "%sac_sim_stats.ac_min_cycle_count += %s_parms::%s_isa::instr_table[ins_id].ac_instr_min_latency;\n",
           INDENT[base_indent], project_name, project_name);
   fprintf(output, "%sac_sim_stats.ac_max_cycle_count += %s_parms::%s_isa::instr_table[ins_id].ac_instr_max_latency;\n",
           INDENT[base_indent], project_name, project_name);
  }
#endif
  if (!pipe_list)
  {
   base_indent--;
   fprintf(output, "%s}\n", INDENT[base_indent]);
  }
 }
 if (ACDebugFlag)
 {
  fprintf(output, "%sif (ac_do_trace != 0)\n", INDENT[base_indent]);
  fprintf(output, PRINT_TRACE, INDENT[base_indent + 1]);
 }
 if (fetch_stage && fetch_stage->next)
 // Not very bright, but someone might have done it! The monostage pipeline! --Marilia
  fprintf(output, "%sregout->write(*instr_vec);\n", INDENT[base_indent]);
 if (!ACDecCacheFlag)
  fprintf(output, "%sdelete instr_vec;\n", INDENT[base_indent]);
 return;
}

/**************************************/
/*!  Emits the if statement executed before
  fetches are performed.
  \brief Used by EmitMultiCycleProcessorBhv and CreateStgImpl functions */
/***************************************/
void EmitFetchInit(FILE* output, int base_indent)
{
 extern int HaveMultiCycleIns;
 extern ac_pipe_list* pipe_list;

 if (!ACDecCacheFlag)
  fprintf(output, "%sif (ac_pc >= APP_MEM->get_size())\n%s{\n",
          INDENT[base_indent], INDENT[base_indent]);
 else
  fprintf(output, "%sif (ac_pc >= dec_cache_size)\n%s{\n", INDENT[base_indent],
          INDENT[base_indent]);
 fprintf(output, "%scerr << \"ArchC: Address out of bounds (pc = 0x\" << hex << static_cast<unsigned long long>(ac_pc.read()) << \").\" << endl;\n",
         INDENT[base_indent + 1]);
#if 0 // Co-verification is currently unmaintained. --Marilia
 if (ACVerifyFlag)
 {
  fprintf(output, "%send_log.mtype = 1;\n", INDENT[base_indent + 1]);
  fprintf(output, "%send_log.log.time = -1;\n", INDENT[base_indent + 1]);
  fprintf(output, "%sif(msgsnd(msqid, reinterpret_cast<struct log_msgbuf*>(&end_log), sizeof(end_log), 0) == -1)\n",
          INDENT[base_indent + 1]);
  fprintf(output, "%sperror(\"msgsnd\");\n", INDENT[base_indent + 2]);
 }
#endif
 fprintf(output, "%sap.stop();\n", INDENT[base_indent + 1]);
 if (pipe_list)
  fprintf(output, "%sdone.notify();\n", INDENT[base_indent + 1]);
 fprintf(output, "%sreturn;\n", INDENT[base_indent + 1]);
 fprintf(output, "%s}\n", INDENT[base_indent]);
 fprintf(output, "%selse\n%s{\n", INDENT[base_indent], INDENT[base_indent]);
 fprintf(output, "%sif (start_up)\n%s{\n", INDENT[base_indent + 1],
         INDENT[base_indent + 1]);
 fprintf(output, "%sap.decode_pc = ac_start_addr;\n", INDENT[base_indent + 2]);
 if (ACABIFlag)
  fprintf(output, "%ssyscall.set_prog_args(argc, argv);\n",
          INDENT[base_indent + 2]);
 fprintf(output, "%sstart_up = 0;\n", INDENT[base_indent + 2]);
 if (ACDecCacheFlag)
  fprintf(output, "%sinit_dec_cache();\n", INDENT[base_indent + 2]);
 fprintf(output, "%s}\n", INDENT[base_indent + 1]);
 fprintf(output, "%selse\n%s{\n", INDENT[base_indent + 1],
         INDENT[base_indent + 1]);
 if (HaveMultiCycleIns && !ACDecCacheFlag)
  fprintf(output, "%sdelete instr_vec;\n", INDENT[base_indent + 2]);
 fprintf(output, "%sap.decode_pc = ac_pc;\n", INDENT[base_indent + 2]);
 fprintf(output, "%s}\n", INDENT[base_indent + 1]);
 return;
}

/**************************************/
/*!  Emits the body of a processor implementation for
  a multicycle processor
  \brief Used by CreateProcessorImpl function      */
/***************************************/
void EmitMultiCycleProcessorBhv(FILE* output)
{
 extern char* project_name;
 extern ac_dec_instr* instr_list;
 ac_dec_instr* pinstr;
 extern ac_dec_format* format_ins_list;
 ac_dec_format* pformat;
 extern ac_dec_field* common_instr_field_list;
 ac_dec_field* pfield;
 int base_indent;

 fprintf(output, "%sif (ac_cycle == 1)\n%s{\n", INDENT[1], INDENT[1]);
 EmitFetchInit(output, 2);
 base_indent = 4;
 if (ACABIFlag)
 {
  base_indent++;
  // Emitting system call handler.
  COMMENT(INDENT[3], "Handling system calls.");
  fprintf(output, "%sif (ap.decode_pc %% 2)\n%sap.decode_pc--;\n", INDENT[3],
          INDENT[3]);
  fprintf(output, "%sswitch (ap.decode_pc)\n%s{\n", INDENT[3], INDENT[3]);
  EmitABIDefine(output, 4);
  EmitABIAddrList(output, 4);
  fprintf(output, "%sdefault:\n", INDENT[4]);
 }
 EmitDecodification(output, base_indent);
 // Multicycle execution demands a different control. Do not use EmitInstrExec.
 fprintf(output, "%sac_cycle = 1;\n", INDENT[base_indent]);
 if (ACDebugFlag)
 {
  fprintf(output, "%sif (ac_do_trace != 0)\n", INDENT[base_indent]);
  fprintf(output, PRINT_TRACE, INDENT[base_indent + 1]);
 }
 if (ACDasmFlag)
  fprintf(output, PRINT_DASM, INDENT[base_indent + 1], project_name, project_name);
 if (ACStatsFlag)
 {
  fprintf(output, "%sisa.stats[%s_stat_ids::INSTRUCTIONS]++;\n",
          INDENT[base_indent], project_name);
  fprintf(output, "%s(*(isa.instr_stats[ins_id]))[%s_instr_stat_ids::COUNT]++;\n",
          INDENT[base_indent], project_name);
#if 0 // No cycle counting as of yet. --Marilia
  fprintf(output, "%sac_sim_stats.ac_min_cycle_count += %s_parms::%s_isa::instr_table[ins_id].ac_instr_min_latency;\n",
          INDENT[base_indent], project_name, project_name);
  fprintf(output, "%sac_sim_stats.ac_max_cycle_count += %s_parms::%s_isa::instr_table[ins_id].ac_instr_max_latency;\n",
          INDENT[base_indent], project_name, project_name);
#endif
 }
 fprintf(output, "%s}\n", INDENT[base_indent - 1]);
 fprintf(output, "%sac_instr_counter += 1;\n", INDENT[base_indent - 1]);
 fprintf(output, "%s}\n", INDENT[base_indent - 2]);
 fprintf(output, "%sisa.current_instruction_id = ins_id;\n", INDENT[base_indent - 2]);
 // Generic behavior call.
 fprintf(output, "%sisa._behavior_instruction(static_cast<%s_parms::ac_stage_list>(0), ac_cycle",
         INDENT[base_indent - 2], project_name);
 /* common_instr_field_list has the list of fields for the generic instruction. */
 for (pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next)
  fprintf(output, ", instr_vec->get(%d)", pfield->id);
 fprintf(output, ");\n");
 fprintf(output, "%sswitch (ins_id)\n%s{\n", INDENT[base_indent - 2],
         INDENT[base_indent - 2]);
 for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next)
 {
  fprintf(output, "%scase %d: // %s\n", INDENT[base_indent - 1], pinstr->id, pinstr->name);
  // Format behavior call.
  fprintf(output, "%sif(!ac_annul_sig)\n%sisa._behavior_%s_%s(static_cast<%s_parms::ac_stage_list>(0), ac_cycle",
          INDENT[base_indent], INDENT[base_indent + 1], project_name,
          pinstr->format, project_name);
  for (pformat = format_ins_list;
       (pformat != NULL) && strcmp(pinstr->format, pformat->name);
       pformat = pformat->next);
  for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
   fprintf(output, ", instr_vec->get(%d)", pfield->id);
  fprintf(output, ");\n");
  // Instruction behavior call.
  fprintf(output, "%sif(!ac_annul_sig)\n%sisa.behavior_%s(static_cast<%s_parms::ac_stage_list>(0), ac_cycle",
          INDENT[base_indent], INDENT[base_indent + 1], pinstr->name, project_name);
  for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
   fprintf(output, ", instr_vec->get(%d)", pfield->id);
  fprintf(output, ");\n");
  fprintf(output, "%sbreak;\n", INDENT[base_indent]);
 }
 fprintf(output, "%s}\n", INDENT[base_indent - 2]);
 fprintf(output, "%sif (ac_cycle > %s_parms::%s_isa::instr_table[ins_id].ac_instr_cycles)\n",
         INDENT[base_indent - 2], project_name, project_name);
 fprintf(output, "%sac_cycle = 1;\n", INDENT[base_indent - 1]);
 if (ACABIFlag)
 {
  // Closing default case.
  fprintf(output, "%sbreak;\n", INDENT[2]);
  // Closing switch.
  fprintf(output, "%s}\n", INDENT[2]);
 }
 return;
}

/**************************************/
/*!  Emits the define that implements the ABI control
  for pipelined architectures
  \brief Used by CreateStgImpl function      */
/***************************************/
void EmitPipeABIDefine(FILE* output)
{
 extern char* project_name;

 fprintf(output, "%s#define AC_SYSC(NAME, LOCATION) \\\n", INDENT[0]);
 fprintf(output, "%scase LOCATION: \\\n", INDENT[3]);
 if (ACStatsFlag)
  fprintf(output, "%sisa.stats[%s_stat_ids::SYSCALLS]++; \\\n", INDENT[4],
          project_name);
 fprintf(output, "%sif (flushes_left) \\\n%s{ \\\n", INDENT[4], INDENT[4]);
 fprintf(output, "%sfflush(0); \\\n", INDENT[5]);
 fprintf(output, "%sflushes_left--; \\\n", INDENT[5]);
 fprintf(output, "%s} \\\n", INDENT[4]);
 fprintf(output, "%selse \\\n%s{ \\\n", INDENT[4], INDENT[4]);
 fprintf(output, "%sfflush(0); \\\n", INDENT[5]);
 if (ACDebugFlag)
 {
  fprintf(output, "%sif (ac_do_trace != 0) \\\n", INDENT[5]);
  fprintf(output, "%strace_file << hex << ap.decode_pc << dec << endl; \\\n",
          INDENT[6]);
 }
 fprintf(output, "%ssyscall.NAME(); \\\n", INDENT[5]);
 fprintf(output, "%sac_instr_counter++; \\\n", INDENT[5]);
 fprintf(output, "%sflushes_left = %d; \\\n", INDENT[5], pipe_maximum_path_length);
 fprintf(output, "%s} \\\n", INDENT[4]);
 fprintf(output, "%sregout->write(*the_nop); \\\n", INDENT[4]);
 fprintf(output, "%sbreak;\n", INDENT[4]);
 return;
}

/**************************************/
/*!  Emits the define that implements the ABI control
  for non-pipelined architectures
  \brief Used by CreateStgImpl function      */
/***************************************/
void EmitABIDefine(FILE* output, int base_indent)
{
 extern char* project_name;

 fprintf(output, "%s#define AC_SYSC(NAME, LOCATION) \\\n", INDENT[0]);
 fprintf(output, "%scase LOCATION: \\\n", INDENT[base_indent]);
 if (ACStatsFlag)
  fprintf(output, "%sisa.stats[%s_stat_ids::SYSCALLS]++; \\\n", INDENT[4],
          project_name);
 if (ACDebugFlag)
 {
  fprintf(output, "%sif (ac_do_trace != 0) \\\n", INDENT[base_indent + 1]);
  fprintf(output, "%strace_file << hex << ap.decode_pc << dec << endl; \\\n",
          INDENT[base_indent + 2]);
 }
 fprintf(output, "%ssyscall.NAME(); \\\n", INDENT[base_indent + 1]);
 fprintf(output, "%sac_instr_counter++; \\\n", INDENT[base_indent + 1]);
 fprintf(output, "%sbreak;\n", INDENT[base_indent + 1]);
 return;
}

/**************************************/
/*!  Emits the ABI special address list
  to be used inside the ABI switchs
  \brief Used by CreateStgImpl function      */
/***************************************/
void EmitABIAddrList(FILE* output, int base_indent)
{
 fprintf(output, "#include <ac_syscall.def>\n", INDENT[base_indent]);
 fprintf(output, "#undef AC_SYSC\n");
 return;
}

/**************************************/
/*!  Emits a ac_cache instantiation.
  \brief Used by CreateResourcesImpl function      */
/***************************************/
void EmitCacheDeclaration(FILE* output, ac_sto_list* pstorage, int base_indent) // TODO -- maybe.
{
 char* cache_str;

 cache_str = (char*) malloc(sizeof(char) * (534 + (2 * strlen(pstorage->name))));
 EmitCacheInitialization(cache_str, pstorage);
 // Printing cache declaration.
 fprintf(output, "%sac_cache ac_resources::%s;\n", INDENT[base_indent],
         cache_str);
 free(cache_str);
 return;
}

/**************************************/
/*!  Writes the bulk of a ac_cache instantiation.
  \brief Used by EmitCacheDeclaration function     */
/***************************************/
void EmitCacheInitialization(char* output, ac_sto_list* pstorage)
{
 // Parameter 1 will be the pstorage->name string
 // Parameters passed to the ac_cache constructor. They are not exactly in the same
 // order used for ArchC declarations.
 // TODO: Include write policy
 char parm2[128];  // Block size
 char parm3[128];  // # if blocks
 char parm4[128];  // set  size
 char parm5[128];  // replacement strategy

 // Integer indicating the write-policy
 int wp = 0;   /* 0x11 (Write Through, Write Allocate)  */
               /* 0x12 (Write Back, Write Allocate)  */
               /* 0x21 (Write Through, Write Around)  */
               /* 0x22 (Write Back, Write Around)  */
 char* aux;
 int is_dm = 0, is_fully = 0;
 ac_cache_parms* pparms;
 int i = 1;

 for (pparms = pstorage->parms; pparms != NULL; pparms = pparms->next)
 {
  switch (i)
  {
   case 1: /* First parameter must be a valid associativity */
    if (!pparms->str)
    {
     AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
     printf("The first parameter must be a valid associativity: \"dm\", \"2w\", \"4w\", ... \n");
     exit(1);
    }
#ifdef DEBUG_STORAGE
    printf("CacheDeclaration: Processing parameter: %d, which is: %s\n", i,
           pparms->str);
#endif
    if (!strcmp(pparms->str, "dm") || !strcmp(pparms->str, "DM"))
    { // It is a direct-mapped cache
     is_dm = 1;
     sprintf(parm4, "1");  // Set size will be the 4th parameter for ac_cache constructor.
     sprintf(parm5, "DEFAULT");  // DM caches do not need a replacement strategy, use a default value in this parameter.
    }
    else if (!strcmp(pparms->str, "fully") || !strcmp(pparms->str, "FULLY"))
     // It is a fully associative cache
     is_fully =1;
    else
    {  // It is a n-way cache
     aux = strchr(pparms->str,'w');
     if (!aux)
     {
      // Checking if the string has a 'w'
      AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
      printf("The first parameter must be a valid associativity: \"dm\", \"2w\", \"4w\", ..., \"fully\" \n");
      exit(1);
     }
     aux = (char*) malloc(sizeof(char) * strlen(pparms->str));
     strncpy(aux, pparms->str, (strlen(pparms->str) - 1));
     aux[strlen(pparms->str) - 1] = '\0';
     sprintf(parm4, "%s", aux);  // Set size will be the 4th parameter for ac_cache constructor.
     free(aux);
    }
    break;
   case 2: /* Second parameter is the number of blocks (lines) */
    if(!(pparms->value > 0))
    {
     AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
     printf("The second parameter must be a valid (>0) number of blocks (lines).\n");
     exit(1);
    }
    if (is_fully)
     sprintf(parm4, "%d", pparms->value);  //Set size will be the number of blocks (lines) for fully associative caches
    sprintf(parm3, "%d", pparms->value);
    break;
   case 3: /* Third parameter is the block (line) size */
    if (!(pparms->value > 0))
    {
     AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
     printf("The third parameter must be a valid (>0) block (line) size.\n");
     exit(1);
    }
    sprintf(parm2, "%d", pparms->value);
    break;
   case 4: /* The fourth  parameter may be the write policy or the replacement strategy.
              If it is a direct-mapped cache, then we don't have a replacement strategy,
              so this parameter must be the write policy, which is "wt" (write-through) or
              "wb" (write-back). Otherwise, it must be a replacement strategy, which is "lru"
              or "random", and the fifth parameter will be the write policy. */
    if (is_dm)
    { //This value is set when the first parameter is being processed.
      /* So this is a write-policy */
     if (!strcmp(pparms->str, "wt") || !strcmp(pparms->str, "WT"))
      // One will tell that a wt cache was declared
      wp = WRITE_THROUGH;
     else if (!strcmp(pparms->str, "wb") || !strcmp(pparms->str, "WB"))
      // Zero will tell that a wb cache was declared
      wp = WRITE_BACK;
     else
     {
      AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
      printf("For direct-mapped caches, the fourth parameter must be a valid write policy: \"wt\" or \"wb\".\n");
      exit(1);
     }
    }
    else
    {
     /* So, this is a replacement strategy */
     if (!strcmp(pparms->str, "lru") || !strcmp(pparms->str, "LRU"))
      sprintf(parm5, "LRU");  //Including parameter
     else if (!strcmp(pparms->str, "random") || !strcmp(pparms->str, "RANDOM"))
      sprintf(parm5, "RANDOM");  //Including parameter
     else
     {
      AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
      printf("For non-direct-mapped caches, the fourth parameter must be a valid replacement strategy: \"lru\" or \"random\".\n");
      exit(1);
     }
    }
    break;
   case 5: /* The fifth parameter is a write policy. */
    if (!is_dm)
    { // This value is set when the first parameter is being processed.
     if (!strcmp(pparms->str, "wt") || !strcmp(pparms->str, "WT"))
      wp = WRITE_THROUGH;
     else if (!strcmp(pparms->str, "wb") || !strcmp(pparms->str, "WB"))
      wp = WRITE_BACK;
     else
     {
      AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
      printf("For non-direct-mapped caches, the fourth parameter must be a valid replacement strategy: \"lru\" or \"random\".\n");
      exit(1);
     }
    }
    else
    { // This value is "war" for write-around or "wal" for "write-allocate"
     if (!strcmp(pparms->str, "war") || !strcmp(pparms->str, "WAR"))
      wp = wp | WRITE_AROUND;
     else if (!strcmp(pparms->str, "wal") || !strcmp(pparms->str, "WAL"))
      wp = wp | WRITE_ALLOCATE;
     else
     {
      AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
      printf("For non-direct-mapped caches, the fourth parameter must be a valid replacement strategy: \"lru\" or \"random\".\n");
      exit(1);
     }
    }
    break;
   case 6: /* The sixth parameter, if it is present, is a write policy.
              It must not be present for direct-mapped caches.*/
    if (!is_dm)
    { // This value is set when the first parameter is being processed.
     if (!strcmp(pparms->str, "war") || !strcmp(pparms->str, "WAR"))
      wp = wp | WRITE_AROUND;
     else if (!strcmp(pparms->str, "wal") || !strcmp(pparms->str, "WAL"))
      wp = wp | WRITE_ALLOCATE;
     else
     {
      AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
      printf("For non-direct-mapped caches, the fifth parameter must be  \"war\" or \"wal\".\n");
      exit(1);
     }
    }
    else
    {
     AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
     printf("For direct-mapped caches there must be only five parameters (do not need a replacement strategy).\n");
     exit(1);
    }
    break;
   default:
    break;
  }
  i++;
 }
 // Printing cache initialization.
 sprintf(output, "%s(\"%s\", %s, %s, %s, %s, 0x%x)",
         pstorage->name, pstorage->name, parm2, parm3, parm4, parm5, wp);
 return;
}

////////////////////////////////////
// Utility Functions              //
////////////////////////////////////

//! Read the archc.conf configuration file.
void ReadConfFile(void)
{
 char* conf_filename_local;
 char* conf_filename_global;
 char* user_home;
 //extern char* ARCHC_PATH;
 extern char* SYSTEMC_PATH;
 extern char* CC_PATH;
 extern char* OPT_FLAGS;
 extern char* DEBUG_FLAGS;
 extern char* OTHER_FLAGS;
 FILE* conf_file;
 char line[CONF_MAX_LINE];
 char var[CONF_MAX_LINE];
 char value[CONF_MAX_LINE];

 user_home = getenv("HOME");
 conf_filename_local = (char*) malloc(sizeof(char) * (strlen(user_home) + 19));
 strcpy(conf_filename_local, user_home);
 strcat(conf_filename_local, "/.archc/archc.conf");
 conf_filename_global = (char*) malloc(sizeof(char) * (strlen(SYSCONFDIR) + 12));
 strcpy(conf_filename_global, SYSCONFDIR);
 strcat(conf_filename_global, "/archc.conf");
 conf_file = fopen(conf_filename_local, "r");
 if (!conf_file)
  conf_file = fopen(conf_filename_global, "r");
 free(conf_filename_local);
 free(conf_filename_global);
 if (!conf_file)
 {
  // ERROR.
  AC_ERROR("Could not open archc.conf configuration file.\n");
  exit(1);
 }
 else
 {
  while (fgets(line, CONF_MAX_LINE, conf_file))
  {
   var[0]= '\0';
   value[0]= '\0';
   if ((line[0] == '#') || (line[0] == '\n'))
    continue; // Comments or blank lines.
   else
   {
    sscanf(line, "%s", var);
    strcpy(value, (strchr(line, '=') + 1));
    if (!strcmp(var, "SYSTEMC_PATH"))
    {
     SYSTEMC_PATH = (char*) malloc(sizeof(char) * (strlen(value) + 1));
     SYSTEMC_PATH = strcpy(SYSTEMC_PATH, value);
     if (strlen(value) <= 2)
     {
      AC_ERROR("Please configure a SystemC path running install.sh script in ArchC directory.\n");
      exit(1);
     }
    }
    else if (!strcmp(var, "CC"))
    {
     CC_PATH = (char*) malloc(sizeof(char) * (strlen(value) + 1));
     CC_PATH = strcpy(CC_PATH, value);
    }
    else if (!strcmp(var, "OPT"))
    {
     OPT_FLAGS = (char*) malloc(sizeof(char) * (strlen(value) + 1));
     OPT_FLAGS = strcpy(OPT_FLAGS, value);
    }
    else if (!strcmp(var, "DEBUG"))
    {
     DEBUG_FLAGS = (char*) malloc(sizeof(char) * (strlen(value) + 1));
     DEBUG_FLAGS = strcpy(DEBUG_FLAGS, value);
    }
    else if (!strcmp(var, "OTHER"))
    {
     OTHER_FLAGS = (char*) malloc(sizeof(char) * (strlen(value) + 1));
     OTHER_FLAGS = strcpy(OTHER_FLAGS, value);
    }
    else if (!strcmp(var, "TARGET_ARCH"))
    {
     TARGET_ARCH = (char*) malloc(sizeof(char) * (strlen(value) + 1));
     TARGET_ARCH = strcpy(TARGET_ARCH, value);
    }
   }
  }
 }
 fclose(conf_file);
 return;
}
