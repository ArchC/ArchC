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
 */

#include "acsim.h"
#include "stdlib.h"
#include "string.h"


//#define DEBUG_STORAGE

//Defining Traces and Dasm strings
#define PRINT_TRACE "%strace_file << hex << decode_pc << dec <<\"\\n\";\n"

//Command-line options flags
int  ACABIFlag=0;                               //!<Indicates whether an ABI was provided or not
int  ACDebugFlag=0;                             //!<Indicates whether debugger option is turned on or not
int  ACDecCacheFlag=1;                          //!<Indicates whether the simulator will cache decoded instructions or not
int  ACDelayFlag=0;                             //!<Indicates whether delay option is turned on or not
int  ACDDecoderFlag=0;                          //!<Indicates whether decoder structures are dumped or not
//int  ACQuietFlag=0;                             //!<Indicates whether storage update logs are displayed during simulation or not
int  ACStatsFlag=0;                             //!<Indicates whether statistics collection is enable or not
int  ACVerboseFlag=0;                           //!<Indicates whether verbose option is turned on or not
int  ACVerifyFlag=0;                            //!<Indicates whether verification option is turned on or not
int  ACVerifyTimedFlag=0;                       //!<Indicates whether verification option is turned on for a timed behavioral model
int  ACGDBIntegrationFlag=0;                    //!<Indicates whether gdb support will be included in the simulator
int  ACWaitFlag=1;                              //!<Indicates whether the instruction execution thread issues a wait() call or not

//char *ACVersion = "2.0alpha1";                        //!<Stores ArchC version number.
char ACOptions[500];                            //!<Stores ArchC recognized command line options
char *ACOptions_p = ACOptions;                  //!<Pointer used to append options in ACOptions
char *arch_filename;                            //!<Stores ArchC arquitecture file

int ac_host_endian;                             //!<Indicates the endianess of the host machine
extern int ac_tgt_endian;                       //!<Indicates the endianess of the host machine
int ac_match_endian;                            //!<Indicates whether host and target endianess match on or not

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
  {"--abi-included"  , "-abi"        ,"Indicate that an ABI for system call emulation was provided." ,"o"},
  {"--debug"         , "-g"          ,"Enable simulation debug features: traces, update logs." ,"o"},
  {"--delay"         , "-dy"          ,"Enable delayed assignments to storage elements." ,"o"},
  {"--dumpdecoder"   , "-dd"         ,"Dump the decoder data structure." ,"o"},
  {"--help"          , "-h"          ,"Display this help message."       , 0},
  {"--no-dec-cache"  , "-ndc"        ,"Disable cache of decoded instructions." ,"o"},
  {"--stats"         , "-s"          ,"Enable statistics collection during simulation." ,"o"},
  {"--verbose"       , "-vb"         ,"Display update logs for storage devices during simulation.", "o"},
  {"--version"       , "-vrs"        ,"Display ACSIM version.", 0},
  {"--gdb-integration", "-gdb"       ,"Enable support for debbuging programs running on the simulator.", 0},
  {"--no-wait"       , "-nw"        ,"Disable wait() at execution thread.", 0},
  0
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
    printf ("    %-17s, %-11s %s\n", option_map[i].name, option_map[i].equivalent, option_map[i].arg_desc);

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
/*!Writes a standard comment at the begining of each file
   generated by ArchC.
   OBS: Description must have 50 characteres at most!!
  \param output The output file pointer.
  \param description A brief description of the file being emited.*/
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
  ac_pipe_list *ppipe;
  extern int HaveFormattedRegs;
  extern int HaveTLMIntrPorts;
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

    else{
      if(!acppLoad(argv[0])){
        AC_ERROR("Invalid input file: %s\n", argv[0]);
        printf("   Try acsim --help for more information.\n");
        return EXIT_FAILURE;
      }
      arch_filename = argv[0];
    }
  }
  else{
    AC_ERROR("No input file provided.\n");
    printf("   Try acsim --help for more information.\n");
    return EXIT_FAILURE;
  }


  ++argv, --argc;  /* skip over arch file name */

  if( argc > 0){

    argn = argc;
    /* Handling command line options */
    for(j= 0; j<argn; j++ ){

      /* Searching option map.*/
      for( i=0; i<ACNumberOfOptions; i++){

        if( (!strcmp(argv[0], option_map[i].name)) || (!strcmp(argv[0], option_map[i].equivalent))){

          switch (i)
            {
            case OPABI:
              ACABIFlag = 1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
            case OPDebug:
              ACDebugFlag = 1;
              ACOptions_p += sprintf( ACOptions_p, "%s ", argv[0]);
              break;
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

            default:
              break;
            }
          ++argv, --argc;  /* skip over founded argument */

          break;
        }
      }
    }
  }

  if(argc >0){
    AC_ERROR("Invalid argument %s.\n", argv[0]);
    return EXIT_FAILURE;
  }

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
  else{

    if( wordsize == 0){
      AC_MSG("Warning: No wordsize defined. Default value is 32 bits.\n");
      wordsize = 32;
    }

    if( fetchsize == 0){
      //AC_MSG("Warning: No fetchsize defined. Default is to be equal to wordsize (%d).\n", wordsize);
      fetchsize = wordsize;
    }

    //Testing host endianess.
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

    //If target is little endian, invert the order of fields in each format. This is the
    //way the little endian decoder expects format fields.
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

    if( stage_list  ){  //List of ac_stage declarations. Used only for single pipe archs

      //Creating Stage Module Header Files
      CreateStgHeader(stage_list, NULL);
      //Creating Stage Module Implementation Files
      CreateStgImpl(stage_list, NULL);

    }
    else if( pipe_list ){  //Pipeline list exist. Used for ac_pipe declarations.

      for( ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next ){

        //Creating Stage Module Header Files
        CreateStgHeader( ppipe->stages, ppipe->name );

        //Creating Stage Module Implementation Files
        CreateStgImpl(ppipe->stages, ppipe->name);
      }
    }
    else{    //No pipe was declared.

      //Creating Processor Files
      CreateProcessorHeader();
      CreateProcessorImpl();
    }


    if( HaveFormattedRegs ){
      //Creating Formatted Registers Header and Implementation Files.
      CreateRegsHeader();
      //CreateRegsImpl();  This is not used anymore.
    }

    if (HaveTLMIntrPorts) {
      CreateIntrHeader();
      CreateIntrMacrosHeader();
      CreateIntrTmpl();
    }

    //Creating co-verification class header file.
    //if( ACVerifyFlag )
    //  CreateCoverifHeader();

    //Creating Simulation Statistics class header file.
    if (ACStatsFlag) {
      CreateStatsHeaderTmpl();
      CreateStatsImplTmpl();
    }

      //Creating model syscall header file.
      if( ACABIFlag )
	CreateArchSyscallHeader();

      /* Create the template for the .cpp instruction and format behavior file */
      CreateImplTmpl();

      /* Creating Parameters Header File */
      CreateParmHeader();

      /* Create the template for the main.cpp  file */
      CreateMainTmpl();

      /* Create the Makefile */
      CreateMakefile();
		
      /* Create dummy functions to use if real ones are undefined */
		
      //Issuing final messages to the user.
      AC_MSG("%s model files generated.\n", project_name);
    }

    return 0;
  }


  ////////////////////////////////////////////////////////////////////////////////////
  // Create functions ...                                                           //
  // These Functions are used to create the behavioral simulato files               //
  // All of them use structures built by the parser.                                //
  ////////////////////////////////////////////////////////////////////////////////////

  /*!Create ArchC Resources Header File */
  void CreateArchHeader() {

    extern ac_pipe_list *pipe_list;
    extern ac_sto_list *storage_list;
    extern ac_stg_list *stage_list;
    extern char* project_name;
    extern char* upper_project_name;

    extern int HaveFormattedRegs, HaveMultiCycleIns, HaveMemHier, HaveTLMPorts, HaveTLMIntrPorts;

    ac_sto_list *pstorage;
    ac_stg_list *pstage;
    char Globals[5000];
    char *Globals_p = Globals;
    ac_pipe_list *ppipe;

    FILE *output;
    char filename[256];

    sprintf(filename, "%s_arch.H", project_name);

    if ( !(output = fopen( filename, "w"))){
      perror("ArchC could not open output file");
      exit(1);
    }

    print_comment( output, "ArchC Resources header file.");
    fprintf( output, "#ifndef  %s_ARCH_H\n", upper_project_name);
    fprintf( output, "#define  %s_ARCH_H\n\n", upper_project_name);

    fprintf( output, "#include  \"%s_parms.H\"\n", project_name);
    fprintf( output, "#include  \"ac_arch_dec_if.H\"\n");
    fprintf( output, "#include  \"ac_storage.H\"\n");
    fprintf( output, "#include  \"ac_memport.H\"\n");
    fprintf( output, "#include  \"ac_regbank.H\"\n");
    fprintf( output, "#include  \"ac_reg.H\"\n");

    if (HaveTLMPorts)
      fprintf(output, "#include  \"ac_tlm_port.H\"\n");

    if (HaveTLMIntrPorts)
      fprintf(output, "#include  \"ac_tlm_intr_port.H\"\n");

    if( HaveFormattedRegs )
      fprintf( output, "#include  \"%s_fmt_regs.H\"\n", project_name);
    fprintf( output, " \n");

    if (ACGDBIntegrationFlag) {
      fprintf(output, "// AC_GDB template class forward declaration\n");
      fprintf(output, "template <typename ac_word> class AC_GDB;\n\n");
    }

    //Declaring Architecture Resources class.
    COMMENT(INDENT[0],"ArchC class for model-specific architectural resources.\n");
    fprintf( output, "class %s_arch : public ac_arch_dec_if<%s_parms::ac_word, %s_parms::ac_Hword> {\n", project_name, project_name, project_name);
    fprintf( output, "public:\n");
    fprintf( output, " \n");

    /* Declaring Program Counter */
    COMMENT(INDENT[1], "Program Counter.");
    fprintf(output, "%sac_reg<unsigned> ac_pc;\n\n", INDENT[1]);

    /* Declaring storage devices */
    COMMENT(INDENT[1],"Storage Devices.");
    for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){

      switch( pstorage->type ){

      case REG:

	//Formatted registers have a special class.
	if( pstorage->format != NULL ){
	  fprintf( output, "%s%s_fmt_%s %s;\n", INDENT[1], project_name, pstorage->name, pstorage->name);
	}
	else{
	  switch( (unsigned)(pstorage->width) ){
	    case 0:
	      fprintf( output, "%sac_reg<%s_parms::ac_word> %s;\n", INDENT[1], project_name, pstorage->name);
	      break;
	    case 1:
	      fprintf( output, "%sac_reg<bool> %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 8:
	      fprintf( output, "%sac_reg<unsigned char> %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 16:
	      fprintf( output, "%sac_reg<unsigned short> %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 32:
	      fprintf( output, "%sac_reg<unsigned long> %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 64:
	      fprintf( output, "%sac_reg<unsigned long long> %s;\n", INDENT[1], pstorage->name);
	      break;
	    default:
	      AC_ERROR("Register width not supported: %d\n", pstorage->width);
	      break;
	  }
	}
	break;

      case REGBANK:
	//Emiting register bank. Checking is a register width was declared.
	switch( (unsigned)(pstorage->width) ){
	case 0:
	  fprintf( output, "%sac_regbank<%d, %s_parms::ac_word, %s_parms::ac_Dword> %s;\n", INDENT[1], pstorage->size, project_name, project_name, pstorage->name);
	  break;
	case 8:
	  fprintf( output, "%sac_regbank<%d, unsigned char, unsigned char> %s;\n", INDENT[1], pstorage->size, pstorage->name);
	  break;
	case 16:
	  fprintf( output, "%sac_regbank<%d, unsigned short, unsigned long> %s;\n", INDENT[1], pstorage->size, pstorage->name);
	  break;
	case 32:
	  fprintf( output, "%sac_regbank<%d, unsigned long, unsigned long long> %s;\n", INDENT[1], pstorage->size, pstorage->name);
	  break;
	case 64:
	  fprintf( output, "%sac_regbank<%d, unsigned long long, unsigned long> %s;\n", INDENT[1], pstorage->size, pstorage->name);
	  break;
	default:
	  AC_ERROR("Register width not supported: %d\n", pstorage->width);
	  break;
	}

	break;

      case CACHE:
      case ICACHE:
      case DCACHE:

	if( !HaveMemHier ) { //It is a generic cache. Just emit a base container object.
	  fprintf( output, "%sac_storage %s_stg;\n", INDENT[1], pstorage->name);
	  fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword> %s;\n", INDENT[1], project_name, project_name, pstorage->name);
	}
	else{
	  //It is an ac_cache object.
	  fprintf( output, "%sac_cache %s;\n", INDENT[1], pstorage->name);
	}

	break;

      case MEM:

	if( !HaveMemHier ) { //It is a generic mem. Just emit a base container object.
	  fprintf( output, "%sac_storage %s_stg;\n", INDENT[1], pstorage->name);
	  fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword> %s;\n", INDENT[1], project_name, project_name, pstorage->name);
	}
	else{
	  //It is an ac_mem object.
	  fprintf( output, "%sac_mem %s;\n", INDENT[1], pstorage->name);
	}

	break;

      case TLM_PORT:
	fprintf(output, "%sac_tlm_port %s_port;\n", INDENT[1], pstorage->name);
	fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword> %s;\n", INDENT[1], project_name, project_name, pstorage->name);
	break;

      default:
	fprintf( output, "%sac_storage %s_stg;\n", INDENT[1], pstorage->name);
	fprintf(output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword> %s;\n", INDENT[1], project_name, project_name, pstorage->name);
	break;
      }
    }

    fprintf( output, " \n");
    fprintf( output, "\n");

    //ac_resources constructor declaration
    COMMENT(INDENT[1],"Constructor.");
    fprintf( output, "%sexplicit %s_arch();\n", INDENT[1], project_name);
	
    fprintf( output, "\n");

    //We have different methods for pipelined and non-pipelined archs
    if(stage_list){
      COMMENT(INDENT[1],"Stall method.");
      fprintf( output, "%svoid ac_stall( char *stage ){\n", INDENT[1]);
		
      for( pstage = stage_list; pstage != NULL; pstage=pstage->next)
	if( pstage->next ){
	  if( pstage->id ==1 )
	    fprintf( output, "%sif( !strcmp( \"%s\", stage ) )\n", INDENT[2], pstage->name);
	  else
	    fprintf( output, "%selse if( !strcmp( \"%s\", stage ) )\n", INDENT[2], pstage->name);
				
	  fprintf( output, "%s%s_stall = 1;\n", INDENT[3], pstage->name);
	}
      fprintf( output, "%s};\n", INDENT[1]);
    }
    else  if(pipe_list){
      COMMENT(INDENT[1],"Stall method.");
      fprintf( output, "%svoid ac_stall( char *stage ){\n", INDENT[1]);
      for( ppipe = pipe_list; ppipe!=NULL; ppipe= ppipe->next ){
			
	for( pstage = ppipe->stages; pstage != NULL; pstage=pstage->next)
	  if( pstage->next ){
	    if( pstage->id ==1 )
	      fprintf( output, "%sif( !strcmp( \"%s\", stage ) )\n", INDENT[2], pstage->name);
	    else
	      fprintf( output, "%selse if( !strcmp( \"%s\", stage ) )\n", INDENT[2], pstage->name);
	    fprintf( output, "%s%s_%s_stall = 1;\n", INDENT[3], ppipe->name, pstage->name);
	  }
      }
      fprintf( output, "%s};\n", INDENT[1]);
    }

    if(ACVerifyFlag){
      COMMENT(INDENT[1],"Set co-verification msg queue.");
      fprintf( output, "%svoid set_queue(char *exec_name);\n", INDENT[1]);
    }

    COMMENT(INDENT[1],"Module initialization method.");
    fprintf( output, "%svirtual void init(int ac, char* av[]) = 0;\n\n", INDENT[1]);

    COMMENT(INDENT[1],"Module finalization method.");
    fprintf( output, "%svirtual void stop(int status = 0) = 0;\n\n", INDENT[1]);

    if (ACGDBIntegrationFlag) {
      COMMENT(INDENT[1], "GDB stub access virtual method declaration.");
      fprintf(output, "%svirtual AC_GDB<%s_parms::ac_word>* get_gdbstub() = 0;\n\n", INDENT[1], project_name);
    }

    COMMENT(INDENT[1],"Virtual destructor declaration.");
    fprintf( output, "%svirtual ~%s_arch() {};\n\n", INDENT[1], project_name);


    fprintf( output, "};\n\n"); //End of ac_resources class

    fprintf( output, "#endif  //_%s_ARCH_H\n", upper_project_name);
    fclose( output);

  }

  /*!Create ArchC Resources Reference Header File */
  void CreateArchRefHeader() {

    extern ac_pipe_list *pipe_list;
    extern ac_sto_list *storage_list;
    extern ac_stg_list *stage_list;
    extern char* project_name;
    extern char* upper_project_name;

    extern int HaveFormattedRegs, HaveMultiCycleIns, HaveMemHier, HaveTLMIntrPorts;

    ac_sto_list *pstorage;
    ac_stg_list *pstage;
    char Globals[5000];
    char *Globals_p = Globals;
    ac_pipe_list *ppipe;

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

    /* Declaring storage devices */
    COMMENT(INDENT[1],"Storage Devices.");
    for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){

      switch( pstorage->type ){

      case REG:

	//Formatted registers have a special class.
	if( pstorage->format != NULL ){
	  fprintf( output, "%s%s_fmt_%s& %s;\n", INDENT[1], project_name, pstorage->name, pstorage->name);
	}
	else{
	  switch( (unsigned)(pstorage->width) ){
	    case 0:
	      fprintf( output, "%sac_reg<%s_parms::ac_word>& %s;\n", INDENT[1], project_name, pstorage->name);
	      break;
	    case 1:
	      fprintf( output, "%sac_reg<bool>& %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 8:
	      fprintf( output, "%sac_reg<unsigned char>& %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 16:
	      fprintf( output, "%sac_reg<unsigned short>& %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 32:
	      fprintf( output, "%sac_reg<unsigned long>& %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 64:
	      fprintf( output, "%sac_reg<unsigned long long>& %s;\n", INDENT[1], pstorage->name);
	      break;
	    default:
	      AC_ERROR("Register width not supported: %d\n", pstorage->width);
	      break;
	  }
	}
	break;

      case REGBANK:
	//Emiting register bank. Checking is a register width was declared.
	switch( (unsigned)(pstorage->width) ){
	case 0:
	  fprintf( output, "%sac_regbank<%d, %s_parms::ac_word, %s_parms::ac_Dword>& %s;\n", INDENT[1], pstorage->size, project_name, project_name, pstorage->name);
	  break;
	case 8:
	  fprintf( output, "%sac_regbank<%d, unsigned char, unsigned char>& %s;\n", INDENT[1], pstorage->size, pstorage->name);
	  break;
	case 16:
	  fprintf( output, "%sac_regbank<%d, unsigned short, unsigned long>& %s;\n", INDENT[1], pstorage->size, pstorage->name);
	  break;
	case 32:
	  fprintf( output, "%sac_regbank<%d, unsigned long, unsigned long long>& %s;\n", INDENT[1], pstorage->size, pstorage->name);
	  break;
	case 64:
	  fprintf( output, "%sac_regbank<%d, unsigned long long, unsigned long long>& %s;\n", INDENT[1], pstorage->size, pstorage->name);
	  break;
	default:
	  AC_ERROR("Register width not supported: %d\n", pstorage->width);
	  break;
	}

	break;

      case CACHE:
      case ICACHE:
      case DCACHE:

	if( !HaveMemHier ) { //It is a generic cache. Just emit a base container object.
	  fprintf( output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword>& %s;\n", INDENT[1], project_name, project_name, pstorage->name);
	}
	else{
	  //It is an ac_cache object.
	  fprintf( output, "%sac_cache& %s;\n", INDENT[1], pstorage->name);
	}

	break;

      case MEM:

	if( !HaveMemHier ) { //It is a generic mem. Just emit a base container object.
	  fprintf( output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword>& %s;\n", INDENT[1], project_name, project_name, pstorage->name);
	}
	else{
	  //It is an ac_mem object.
	  fprintf( output, "%sac_mem& %s;\n", INDENT[1], pstorage->name);
	}

	break;

      default:
	fprintf( output, "%sac_memport<%s_parms::ac_word, %s_parms::ac_Hword>& %s;\n", INDENT[1], project_name, project_name, pstorage->name);
	break;
      }
    }

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

    extern ac_pipe_list *pipe_list;
    extern ac_sto_list *storage_list;
    extern ac_stg_list *stage_list;
    extern char* project_name;

    extern int HaveFormattedRegs, HaveMultiCycleIns, HaveMemHier, reg_width;

    ac_sto_list *pstorage;
    ac_stg_list *pstage;
    char Globals[5000];
    char *Globals_p = Globals;
    ac_pipe_list *ppipe;

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
    fprintf(output,
	    "%s_arch_ref::%s_arch_ref(%s_arch& arch) : ac_arch_ref<%s_parms::ac_word, %s_parms::ac_Hword>(arch),\n",
	    project_name, project_name, project_name, project_name, project_name);

    /* Declaring ac_pc reference */
    fprintf(output, "%sac_pc(arch.ac_pc),\n", INDENT[1]);

    /* Declaring storage devices */
    for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){
      fprintf(output, "%s%s(arch.%s)", INDENT[1], pstorage->name,
	      pstorage->name);
      if (pstorage->next != NULL) {
	fprintf(output, ", ");
      }
    }
    fprintf(output, " {}\n\n");
    fclose( output);

  }


  //!Creates Decoder Header File
  void CreateParmHeader() {

    extern ac_stg_list *stage_list;
    extern ac_pipe_list *pipe_list;
    extern ac_dec_format *format_ins_list;
    extern int instr_num;
    extern int declist_num;
    extern int format_num, largest_format_size;
    extern int wordsize, fetchsize, HaveMemHier, HaveCycleRange;
    extern ac_sto_list* load_device;

    extern ac_decoder_full *decoder;
    extern char* project_name;
    extern char* upper_project_name;
    ac_stg_list *pstage;
    ac_pipe_list *ppipe;

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
    if( ACVerboseFlag || ACVerifyFlag )
      fprintf( output, "#define  AC_UPDATE_LOG \t //!< Update log generation turned on.\n");

    if( ACVerboseFlag )
      fprintf( output, "#define  AC_VERBOSE \t //!< Indicates Verbose mode. Where update logs are dumped on screen.\n");

    if( ACVerifyFlag )
      fprintf( output, "#define  AC_VERIFY \t //!< Indicates that co-verification is turned on.\n");
	
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

    /* parms namespace definition */
    fprintf(output, "namespace %s_parms {\n\n", project_name);

    fprintf( output, "\nstatic const unsigned int AC_DEC_FIELD_NUMBER = %d; \t //!< Number of Fields used by decoder.\n", decoder->nFields);
    fprintf( output, "static const unsigned int AC_DEC_INSTR_NUMBER = %d; \t //!< Number of Instructions declared.\n", instr_num);
    fprintf( output, "static const unsigned int AC_DEC_FORMAT_NUMBER = %d; \t //!< Number of Formats declared.\n", format_num);
    fprintf( output, "static const unsigned int AC_DEC_LIST_NUMBER = %d; \t //!< Number of decodification lists used by decoder.\n", declist_num);
    fprintf( output, "static const unsigned int AC_MAX_BUFFER = %d; \t //!< This is the size needed by decoder buffer. It is equal to the biggest instruction size.\n", largest_format_size/8);
    fprintf( output, "static const unsigned int AC_WORDSIZE = %d; \t //!< Architecture wordsize in bits.\n", wordsize);
    fprintf( output, "static const unsigned int AC_FETCHSIZE = %d; \t //!< Architecture fetchsize in bits.\n", fetchsize);
    fprintf( output, "static const unsigned int AC_MATCH_ENDIAN = %d; \t //!< If the simulated arch match the endian with host.\n", ac_match_endian);
    fprintf( output, "static const unsigned int AC_PROC_ENDIAN = %d; \t //!< The simulated arch is big endian?\n", ac_tgt_endian);
    fprintf( output, "static const unsigned int AC_RAMSIZE = %uU; \t //!< Architecture RAM size in bytes (storage %s).\n", load_device->size, load_device->name);
    fprintf( output, "static const unsigned int AC_RAM_END = %uU; \t //!< Architecture end of RAM (storage %s).\n", load_device->size, load_device->name);

    if (ACGDBIntegrationFlag)
    fprintf( output, "static const unsigned int GDB_PORT_NUM = 5000; \t //!< GDB port number.\n", load_device->size, load_device->name);

    fprintf( output, "\n\n");
    COMMENT(INDENT[0],"Word type definitions.");

    //Emiting ArchC word types.
    switch( wordsize ){
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

    if( stage_list ){   //Enum type for pipes declared through ac_stage keyword
      fprintf( output, "%s=1,", stage_list->name);
      pstage = stage_list->next;
      for( pstage = stage_list->next; pstage && pstage->next != NULL; pstage=pstage->next){
	fprintf( output, "%s,", pstage->name);
      }
      fprintf( output, "%s", pstage->name);
    }
    else if(pipe_list){  //Enum type for pipes declared through ac_pipe keyword

      for(ppipe = pipe_list; ppipe!= NULL; ppipe=ppipe->next){

	for(pstage=ppipe->stages; pstage && pstage->next!= NULL; pstage=pstage->next){

	  //When we have just one pipe use only the stage name
	  if( ppipe->next )
	    fprintf( output, "%s_%s=%d,", ppipe->name, pstage->name, pstage->id);
	  else
	    fprintf( output, "%s=%d,", pstage->name, pstage->id);

	}
	if( ppipe->next )
	  fprintf( output, "%s_%s=%d", ppipe->name, pstage->name, pstage->id);  //The last doesn't need a comma
	else
	  fprintf( output, "%s=%d", pstage->name, pstage->id);
      }
    }
    else{
      fprintf( output, "ST0");
    }

    //Closing enum declaration
    fprintf( output, "};\n\n");

    /* closing namespace declaration */

    fprintf( output, "}\n\n");

    //Create a compiler error if delay assignment is used without the -dy option
    COMMENT(INDENT[0],"Create a compiler error if delay assignment is used without the -dy option");
    fprintf( output, "#ifndef AC_DELAY\n");
    fprintf( output, "extern %s_parms::ac_word ArchC_ERROR___PLEASE_USE_OPTION_DELAY_WHEN_CREATING_SIMULATOR___;\n", project_name);
    fprintf( output, "#define delay(a,b) ArchC_ERROR___PLEASE_USE_OPTION_DELAY_WHEN_CREATING_SIMULATOR___\n");
    fprintf( output, "#endif\n");


    fprintf( output, "\n\n");
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
    extern int HaveMultiCycleIns;
    extern int wordsize;
    extern ac_dec_field *common_instr_field_list;
    ac_grp_list* pgroup;
    ac_dec_format *pformat;
    ac_dec_instr *pinstr;
    ac_dec_field *pfield, *pf;

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
    fprintf(output, "%sstatic ac_dec_field fields[AC_DEC_FIELD_NUMBER];\n", INDENT[1]);
    fprintf(output, "%sstatic ac_dec_format formats[AC_DEC_FORMAT_NUMBER];\n", INDENT[1]);
    fprintf(output, "%sstatic ac_dec_list dec_list[AC_DEC_LIST_NUMBER];\n", INDENT[1]);
    fprintf(output, "%sstatic ac_dec_instr instructions[AC_DEC_INSTR_NUMBER];\n", INDENT[1]);
    fprintf(output, "%sstatic const ac_instr_info instr_table[AC_DEC_INSTR_NUMBER + 1];\n\n", INDENT[1]);

    fprintf( output, "%sac_decoder_full* decoder;\n\n", INDENT[1]);
    if (ACABIFlag)
      fprintf( output, "%s%s_syscall syscall;\n", INDENT[1], project_name );

    /* current instruction ID */
    fprintf(output, "%sint cur_instr_id;\n\n", INDENT[1]);
    /* ac_helper */
    if (helper_contents)
    {
     fprintf(output, "%s", helper_contents);
     fprintf(output, "\n");
    }

    //Emiting Constructor.
    COMMENT(INDENT[1], "Constructor.");
    fprintf( output,"%s%s_isa(%s_arch& ref) : %s_arch_ref(ref) ", INDENT[1],
	     project_name, project_name, project_name);
    if (ACABIFlag)
      fprintf(output, ", syscall(ref)");
    fprintf( output," {\n");

    COMMENT(INDENT[2], "Building Decoder.");
    fprintf( output,"%sdecoder = ac_decoder_full::CreateDecoder(%s_isa::formats, %s_isa::instructions, &ref);\n", INDENT[2], project_name, project_name );

    /* Closing constructor declaration. */
    fprintf( output,"%s}\n\n", INDENT[1] );

    /* getter methods for current instruction */
    fprintf(output, "%sinline const char* get_name() { return instr_table[cur_instr_id].ac_instr_name; }\n", INDENT[1]);
    fprintf(output, "%sinline const char* get_mnemonic() { return instr_table[cur_instr_id].ac_instr_mnemonic; }\n", INDENT[1]);
    fprintf(output, "%sinline unsigned get_size() { return instr_table[cur_instr_id].ac_instr_size; };\n", INDENT[1]);
    fprintf(output, "%sinline unsigned get_cycles() { return instr_table[cur_instr_id].ac_instr_cycles; };\n", INDENT[1]);
    fprintf(output, "%sinline unsigned get_min_latency() { return instr_table[cur_instr_id].ac_instr_min_latency; };\n", INDENT[1]);
    fprintf(output, "%sinline unsigned get_max_latency() { return instr_table[cur_instr_id].ac_instr_max_latency; };\n\n", INDENT[1]);
    // Group query methods.
    for (pgroup = group_list; pgroup != NULL; pgroup = pgroup->next)
    {
     fprintf(output, "%sinline const bool belongs_to_%s()\n%s{\n",
             INDENT[2], pgroup->name, INDENT[2]);
     fprintf(output, "%sreturn group_%s[cur_instr_id];\n%s}\n", INDENT[3], pgroup->name, INDENT[2]);
     fprintf(output, "\n");
    }
    /* Instruction Behavior Method declarations */
    /* instruction */
    fprintf(output, "%svoid _behavior_instruction(", INDENT[1]);
    /* common_instr_field_list has the list of fields for the generic instruction. */
    for( pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next){
      if( pfield->sign )
	fprintf(output, "int %s", pfield->name);
      else
	fprintf(output, "unsigned int %s", pfield->name);
      if (pfield->next != NULL)
	fprintf(output, ", ");
    }
    fprintf(output, ");\n\n");

    /* begin & end */
    fprintf(output, "%svoid _behavior_begin();\n", INDENT[1]);
    fprintf(output, "%svoid _behavior_end();\n\n", INDENT[1]);

    /* types/formats */
    for (pformat = format_ins_list; pformat!= NULL; pformat=pformat->next) {
      fprintf(output, "%svoid _behavior_%s_%s(",
	      INDENT[1], project_name, pformat->name);
      for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
	if (pfield -> sign)
	  fprintf(output, "int %s", pfield->name);
	else
	  fprintf(output, "unsigned int %s", pfield->name);
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
      fprintf(output, "%svoid behavior_%s(",
	      INDENT[1], pinstr->name);
      for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
	if (pfield -> sign)
	  fprintf(output, "int %s", pfield->name);
	else
	  fprintf(output, "unsigned int %s", pfield->name);
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
    fprintf(output, "typedef ac_memport<%s_parms::ac_word, %s_parms::ac_Hword> ac_memory;\n\n", project_name, project_name);

    /* ac_behavior main macro */
    fprintf( output, "#define ac_behavior(instr) AC_BEHAVIOR_##instr ()\n\n");

    /* ac_behavior 2nd level macros - generic instruction */
    fprintf(output, "#define AC_BEHAVIOR_instruction() %s_parms::%s_isa::_behavior_instruction(",
            project_name, project_name);
    /* common_instr_field_list has the list of fields for the generic instruction. */
    for( pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next){
      if( pfield->sign )
	fprintf(output, "int %s", pfield->name);
      else
	fprintf(output, "unsigned int %s", pfield->name);
      if (pfield->next != NULL)
	fprintf(output, ", ");
    }
    fprintf(output, ")\n\n");

    /* ac_behavior 2nd level macros - pseudo-instructions begin, end */
    fprintf(output, "#define AC_BEHAVIOR_begin() %s_parms::%s_isa::_behavior_begin()\n", project_name, project_name);
    fprintf(output, "#define AC_BEHAVIOR_end() %s_parms::%s_isa::_behavior_end()\n", project_name, project_name);

    fprintf(output, "\n");

    /* ac_behavior 2nd level macros - instruction types */
    for( pformat = format_ins_list; pformat!= NULL; pformat=pformat->next) {
      fprintf(output, "#define AC_BEHAVIOR_%s() %s_parms::%s_isa::_behavior_%s_%s(", pformat->name, project_name, project_name, project_name, pformat->name);
      for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
	if (pfield -> sign)
	  fprintf(output, "int %s", pfield->name);
	else
	  fprintf(output, "unsigned int %s", pfield->name);
	if (pfield->next != NULL)
	  fprintf(output, ", ");
      }
      fprintf(output, ")\n");
    }
    fprintf(output, "\n");

    /* ac_behavior 2nd level macros - instructions */
    for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
      fprintf(output, "#define AC_BEHAVIOR_%s() %s_parms::%s_isa::behavior_%s(", pinstr->name, project_name, project_name, pinstr->name);
      for (pformat = format_ins_list;
	   (pformat != NULL) && strcmp(pinstr->format, pformat->name);
	   pformat = pformat->next);
      for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
	if (pfield -> sign)
	  fprintf(output, "int %s", pfield->name);
	else
	  fprintf(output, "unsigned int %s", pfield->name);
	if (pfield->next != NULL)
	  fprintf(output, ", ");
      }
      fprintf(output, ")\n");
    }

    /* END OF FILE */
    fprintf( output, "\n\n#endif //_%s_BHV_MACROS_H\n\n", upper_project_name);
    fclose( output);

  }

  //!Creates Stage Module Header File  for Single Pipelined Architectures.
  void CreateStgHeader( ac_stg_list* stage_list, char* pipe_name) {

    extern char *project_name;
    extern int stage_num;
    ac_stg_list *pstage;

    char* stage_filename;
    FILE* output;

    for( pstage = stage_list; pstage != NULL; pstage=pstage->next){


      //IF a ac_pipe declaration was used, stage module names will be PIPENAME_STAGENAME,
      //otherwise they will be just STAGENAME.
      //This makes possible to define multiple pipelines containg stages with the same name.
      if( pipe_name ){
	stage_filename = (char*) malloc(strlen(pstage->name)+strlen(pipe_name)+strlen(".H")+2);
	sprintf( stage_filename, "%s_%s.H", pipe_name, pstage->name);
      }
      else{
	stage_filename = (char*) malloc(strlen(pstage->name)+strlen(".H")+1);
	sprintf( stage_filename, "%s.H", pstage->name);
      }

      if ( !(output = fopen( stage_filename, "w"))){
	perror("ArchC could not open output file");
	exit(1);
      }

      print_comment( output, "Stage Module Header File.");
      if( pipe_name ){
	fprintf( output, "#ifndef  _%s_%s_STAGE_H\n", pipe_name, pstage->name);
	fprintf( output, "#define  _%s_%s_STAGE_H\n\n", pipe_name, pstage->name);
      }
      else{
	fprintf( output, "#ifndef  _%s_STAGE_H\n", pstage->name);
	fprintf( output, "#define  _%s_STAGE_H\n\n", pstage->name);
      }

      fprintf( output, "#include \"archc.H\"\n");
      fprintf( output, "#include \"%s_isa.H\"\n\n", project_name);

      if( pstage->id == 1 && ACDecCacheFlag )
	fprintf( output, "extern unsigned dec_cache_size;\n\n");

      //Declaring stage namespace.
      if( pipe_name ){
	fprintf( output, "namespace AC_%s_%s{\n\n", pipe_name, pstage->name);
	fprintf( output, "class %s_%s: public ac_stage {\n", pipe_name, pstage->name);
      }
      else{
	fprintf( output, "namespace AC_%s{\n", pstage->name);
	fprintf( output, "class %s: public ac_stage {\n", pstage->name);
      }
      //Declaring stage module.
      //It already includes the behavior method.
      fprintf( output, "public:\n");

      if( pstage->id != 1 )
	fprintf( output, "%ssc_in<ac_instr> regin;\n", INDENT[1]);
      fprintf( output, "%ssc_inout<unsigned> bhv_pc;\n", INDENT[1]);

      if( pstage->id != stage_num ){
	fprintf( output, "%ssc_out<ac_instr> regout;\n", INDENT[1]);
	fprintf( output, "%ssc_out<bool> bhv_start;\n", INDENT[1]);
      }

      fprintf( output, "%ssc_out<bool> bhv_done;\n", INDENT[1]);
      fprintf( output, "%s%s_parms::%s_isa ISA;\n", INDENT[1], project_name, project_name);



      fprintf( output, "%sbool start_up;\n", INDENT[1]);
      fprintf( output, "%sunsigned id;\n\n", INDENT[1]);
      fprintf( output, "%svoid behavior();\n\n", INDENT[1]);

      if(pstage->id==1 && ACDecCacheFlag){
	fprintf( output, "%scache_item* DEC_CACHE;\n\n", INDENT[1]);
      }
		
      if( pipe_name ){
	fprintf( output, "%sSC_HAS_PROCESS( %s_%s );\n\n", INDENT[1], pipe_name, pstage->name);
	fprintf( output, "%s%s_%s( sc_module_name name_ ): ac_stage(name_){\n\n", INDENT[1], pipe_name, pstage->name);
      }
      else{
	fprintf( output, "%sSC_HAS_PROCESS( %s );\n\n", INDENT[1], pstage->name);
	fprintf( output, "%s%s( sc_module_name name_ ): ac_stage(name_){\n\n", INDENT[1], pstage->name);
      }

      //Declaring Constructor.
      fprintf( output, "%sSC_METHOD( behavior );\n", INDENT[2]);
      if( pstage->id != stage_num)
	fprintf( output, "%ssensitive_pos << bhv_start;\n", INDENT[2]);
      else
	fprintf( output, "%ssensitive << bhv_pc;\n", INDENT[2]);

      //We need this in order to not fetch the first instruction
      //during initialization.
      if( pstage->id == 1){
	fprintf( output, "%sdont_initialize();\n\n", INDENT[2]);
	fprintf( output, "%sstart_up=1;\n", INDENT[2]);
      }
      fprintf( output, "%sid = %d;\n\n", INDENT[2], pstage->id);

      //end of constructor
      fprintf( output, "%s}\n", INDENT[1]);

      if(pstage->id==1 && ACDecCacheFlag){
	fprintf( output, "%svoid init_dec_cache() {\n", INDENT[1]);  //end constructor
	fprintf( output, "%sDEC_CACHE = (cache_item*)calloc(sizeof(cache_item),dec_cache_size);\n", INDENT[2]);  //end constructor
	fprintf( output, "%s}\n", INDENT[1]);  //end init_dec_cache
      }

		
      fprintf( output, "};\n");

      //End of namespace
      fprintf( output, "}\n");

      fprintf( output, "#endif \n");
      fclose(output);
      free(stage_filename);
    }
  }

  //!Creates Processor Module Header File
  void CreateProcessorHeader() {

    extern ac_stg_list *stage_list;
    extern ac_pipe_list *pipe_list;
    extern char *project_name;
    extern char *upper_project_name;
    extern int stage_num;
    extern int HaveMultiCycleIns;
    extern int HaveTLMIntrPorts;
    extern ac_sto_list *tlm_intr_port_list;
    ac_stg_list *pstage;
    ac_pipe_list *ppipe;
    ac_sto_list *pport;
    int i;
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

    fprintf( output, "#include \"systemc.h\"\n");
    fprintf( output, "#include \"ac_module.H\"\n");
    fprintf( output, "#include \"ac_utils.H\"\n");
    fprintf( output, "#include \"%s_parms.H\"\n", project_name);
    fprintf( output, "#include \"%s_arch.H\"\n", project_name);
    fprintf( output, "#include \"%s_isa.H\"\n", project_name);

    if (ACABIFlag)
      fprintf( output, "#include \"%s_syscall.H\"\n", project_name);

    if (HaveTLMIntrPorts) {
      fprintf(output, "#include \"ac_tlm_intr_port.H\"\n");
      fprintf(output, "#include \"%s_intr_handlers.H\"\n", project_name);
    }

    if(ACGDBIntegrationFlag) {
      fprintf( output, "#include \"ac_gdb_interface.H\"\n");
      fprintf( output, "#include \"ac_gdb.H\"\n");
    }

    fprintf(output, "\n\n");

    fprintf(output, "class %s: public ac_module, public %s_arch", project_name, project_name);

    if (ACGDBIntegrationFlag)
      fprintf(output, ", public AC_GDB_Interface<%s_parms::ac_word>", project_name);

    fprintf(output, " {\n");

    fprintf(output, "private:\n");
    fprintf(output, "%stypedef cache_item<%s_parms::AC_DEC_FIELD_NUMBER> cache_item_t;\n", INDENT[1], project_name);
    fprintf(output, "%stypedef ac_instr<%s_parms::AC_DEC_FIELD_NUMBER> ac_instr_t;\n", INDENT[1], project_name);

    fprintf( output, "public:\n\n");

    fprintf( output, "%sunsigned bhv_pc;\n", INDENT[1]);

    if( HaveMultiCycleIns)
      fprintf( output, "%ssc_signal<unsigned> bhv_cycle;\n", INDENT[1]);

    fprintf( output, " \n");

    if (ACVerboseFlag || ACVerifyFlag || ACVerifyTimedFlag)
      fprintf( output, "%ssc_signal<bool> done;\n\n", INDENT[1]);

    fprintf( output, "\n");

    fprintf(output, "%sbool has_delayed_load;\n", INDENT[1]);
    fprintf(output, "%schar* delayed_load_program;\n\n", INDENT[1]);

    fprintf( output, "%s%s_parms::%s_isa ISA;\n", INDENT[1], project_name, project_name);
    /*    if (ACABIFlag)
          fprintf( output, "%s%s_syscall syscall;\n", INDENT[1], project_name );*/

    if (HaveTLMIntrPorts) {
      for (pport = tlm_intr_port_list; pport != NULL; pport = pport->next) {
	fprintf(output, "%s%s_%s_handler %s_hnd;\n", INDENT[1], project_name, pport->name, pport->name);
	fprintf(output, "%sac_tlm_intr_port %s;\n\n", INDENT[1], pport->name);
      }
    }

    if(ACDecCacheFlag){
      fprintf( output, "%scache_item_t* DEC_CACHE;\n\n", INDENT[1]);
    }

    fprintf( output, "%sunsigned id;\n\n", INDENT[1]);
    fprintf( output, "%sbool start_up;\n", INDENT[1]);
    fprintf( output, "%sunsigned* instr_dec;\n", INDENT[1]);
    fprintf( output, "%sac_instr_t* instr_vec;\n\n", INDENT[1]);

    if (ACGDBIntegrationFlag)
      fprintf(output, "%sAC_GDB<%s_parms::ac_word>* gdbstub;\n\n", INDENT[1], project_name);

    COMMENT(INDENT[1], "Behavior execution method.");
    fprintf( output, "%svoid behavior();\n\n", INDENT[1]);

    if (ACVerboseFlag || ACVerifyFlag || ACVerifyTimedFlag) {
      COMMENT(INDENT[1], "Verification method.");
      fprintf( output, "%svoid ac_verify();\n", INDENT[1]);
      fprintf( output, " \n");
    }

    fprintf( output, " \n");

    fprintf( output, "%sSC_HAS_PROCESS( %s );\n\n", INDENT[1], project_name);

    //!Declaring ARCH Constructor.
    COMMENT(INDENT[1], "Constructor.");
    fprintf( output, "%s%s( sc_module_name name_ ): ac_module(name_), %s_arch(), ISA(*this)", INDENT[1], project_name, project_name);
    /*if (ACABIFlag)
      fprintf(output, ", syscall(*this)");*/

    if (HaveTLMIntrPorts) {
      for (pport = tlm_intr_port_list; pport != NULL; pport = pport->next) {
	fprintf(output, ", %s_hnd(*this)", pport->name);
	fprintf(output, ", %s(\"%s\", %s_hnd)", pport->name, pport->name, pport->name);
      }
    }

    fprintf(output, " {\n\n");

    fprintf( output, "%sSC_THREAD( behavior );\n", INDENT[2]);

    if (ACVerboseFlag || ACVerifyFlag || ACVerifyTimedFlag) {
      fprintf( output, "%sSC_THREAD( ac_verify );\n", INDENT[2]);
      fprintf( output, "%ssensitive<< done;\n", INDENT[2]);
      fprintf( output, " \n");
    }

    fprintf( output,"%sbhv_pc = 0; \n", INDENT[2]);
    fprintf( output,"%shas_delayed_load = false; \n", INDENT[2]);

    fprintf( output, "%sstart_up=1;\n", INDENT[2]);
    fprintf( output, "%sid = %d;\n\n", INDENT[2], 1);

    if (ACGDBIntegrationFlag)
      fprintf(output, "%sgdbstub = new AC_GDB<%s_parms::ac_word>(this, %s_parms::GDB_PORT_NUM);\n\n", INDENT[2], project_name, project_name);

    fprintf( output, "%s}\n", INDENT[1]);  //end constructor

    if(ACDecCacheFlag){
      fprintf( output, "%svoid init_dec_cache() {\n", INDENT[1]);  //end constructor
      fprintf( output, "%sDEC_CACHE = (cache_item_t*) calloc(sizeof(cache_item_t),dec_cache_size);\n", INDENT[2]);  //end constructor
      fprintf( output, "%s}\n", INDENT[1]);  //end init_dec_cache
    }

    if(ACGDBIntegrationFlag) {
      fprintf( output, "%s/***********\n", INDENT[1]);
      fprintf( output, "%s * GDB Support - user supplied methods\n", INDENT[1]);
      fprintf( output, "%s * For further information, look at ~/src/aclib/ac_gdb/ac_gdb_interface.H\n", INDENT[1]);
      fprintf( output, "%s ***********/\n\n", INDENT[1]);

      fprintf( output, "%s/* Processor Feature Support */\n", INDENT[1]);
      fprintf( output, "%sbool get_ac_tgt_endian();\n\n", INDENT[1]);
      fprintf( output, "%svoid ac_stop();\n\n", INDENT[1]);

      fprintf( output, "%s/* Register access */\n", INDENT[1]);
      fprintf( output, "%sint nRegs(void);\n", INDENT[1]);
      fprintf( output, "%s%s_parms::ac_word reg_read(int reg);\n", INDENT[1], project_name);
      fprintf( output, "%svoid reg_write( int reg, %s_parms::ac_word value );\n", INDENT[1], project_name);

      fprintf( output, "%s/* Memory access */\n", INDENT[1]);
      fprintf( output, "%sunsigned char mem_read( unsigned int address );\n", INDENT[1]);
      fprintf( output, "%svoid mem_write( unsigned int address, unsigned char byte );\n", INDENT[1]);

      fprintf( output, "%s/* GDB stub access */\n", INDENT[1]);
      fprintf( output, "%sAC_GDB<%s_parms::ac_word>* get_gdbstub();\n", INDENT[1], project_name);
    }


    fprintf( output, "\n%sunsigned get_ac_pc();\n\n", INDENT[1]);
    fprintf( output, "%svoid set_ac_pc( unsigned int value );\n\n", INDENT[1]);

    fprintf( output, "%svirtual void PrintStat();\n\n", INDENT[1]);

    fprintf( output, "%svoid init(int ac, char* av[]);\n\n", INDENT[1]);
    fprintf( output, "%svoid init();\n\n", INDENT[1]);
    fprintf( output, "%svoid load(char* program);\n\n", INDENT[1]);
    fprintf( output, "%svoid delayed_load(char* program);\n\n", INDENT[1]);
    fprintf( output, "%svoid stop(int status = 0);\n\n", INDENT[1]);

    if (ACGDBIntegrationFlag)
      fprintf(output, "%svoid enable_gdb(int port = 5000);\n\n", INDENT[1]);

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
  FILE *output;
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

      //TO DO: Registers with parameterized size. The templated class ac_reg is still not
      //       working with sc_unit<x> types.
      for( pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
        fprintf( output,"%sac_reg<unsigned> %s;\n",INDENT[1],pfield->name );

      fprintf( output, "\n\n");

      //Declaring class constructor.
      if (ACDelayFlag) {
        fprintf( output, "%s%s_fmt_%s(char* n, double& ts): \n", INDENT[1], project_name, pstorage->name);
      }
      else {
        fprintf( output, "%s%s_fmt_%s(char* n): \n", INDENT[1], project_name, pstorage->name);
      }
      for( pfield = pformat->fields; pfield->next != NULL; pfield = pfield->next) {
        //Initializing field names with reg name. This is to enable Formatted Reg stats.
        //Need to be changed if we adopt statistics collection for each field individually.
        if (ACDelayFlag) {
          fprintf( output,"%s%s(\"%s\",%d,ts),\n",INDENT[2],pfield->name,pstorage->name, 0 );
        }
        else {
          fprintf( output,"%s%s(\"%s\",%d),\n",INDENT[2],pfield->name,pstorage->name, 0 );
        }
      }
      //Last field.
      if (ACDelayFlag) {
        fprintf( output,"%s%s(\"%s\",%d,ts){name = n;}\n\n",INDENT[2],pfield->name,pstorage->name, 0 );
      }
      else {
        fprintf( output,"%s%s(\"%s\",%d){name = n;}\n\n",INDENT[2],pfield->name,pstorage->name, 0 );
      }

      fprintf( output,"%svoid change_dump(ostream& output){}\n\n",INDENT[1] );
      fprintf( output,"%svoid reset_log(){}\n\n",INDENT[1] );
      if (ACDelayFlag) {
        fprintf( output,"%svoid commit_delays(double time)\n",INDENT[1] );
        fprintf( output,"%s{\n",INDENT[1] );
        for( pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
          fprintf( output,"%s%s.commit_delays(time);\n", INDENT[2], pfield->name);
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
void CreateStatsImplTmpl()
{
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

  fprintf(output, "#include \"%s_stats.H\"\n", project_name);
  fprintf(output, "\n");

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

//!Creates Stage Module Implementation File for Single Pipelined Architectures
void CreateStgImpl(ac_stg_list* stage_list, char* pipe_name) {

  extern char *project_name;
  extern int stage_num;
  ac_stg_list *pstage;
  int base_indent;
  char* stage_filename;
  FILE* output;

  for( pstage = stage_list; pstage != NULL; pstage=pstage->next){

    //IF a ac_pipe declaration was used, stage module names will be PIPENAME_STAGENAME,
    //otherwise they will be just STAGENAME.
    //This makes possible to define multiple pipelines containg stages with the same name.
    if( pipe_name ){
      stage_filename = (char*) malloc(strlen(pstage->name)+strlen(pipe_name)+strlen(".cpp")+2);
      sprintf( stage_filename, "%s_%s.cpp", pipe_name, pstage->name);
    }
    else{
      stage_filename = (char*) malloc(strlen(pstage->name)+strlen(".cpp")+1);
      sprintf( stage_filename, "%s.cpp", pstage->name);
    }

    if ( !(output = fopen( stage_filename, "w"))){
      perror("ArchC could not open output file");
      exit(1);
    }

    print_comment( output, "Stage Module Implementation File.");
    if( pipe_name ){
      fprintf( output, "#include  \"%s_%s.H\"\n\n", pipe_name, pstage->name);
    }
    else{
      fprintf( output, "#include  \"%s.H\"\n\n", pstage->name);
    }

    if( pstage->id == 1 && ACABIFlag )
      fprintf( output, "#include  \"%s_syscall.H\"\n\n", project_name);

    if( pstage->id == 1 && ACVerifyFlag ){
      fprintf( output, "#include  \"ac_msgbuf.H\"\n");
      fprintf( output, "#include  \"sys/msg.h\"\n");
    }


    //Emiting stage behavior method implementation.
    if( pstage->id != 1 ){

      if( pipe_name )			
        fprintf( output, "void AC_%s_%s::%s_%s::behavior() {\n\n", pipe_name, pstage->name, pipe_name, pstage->name);
      else
        fprintf( output, "void AC_%s::%s::behavior() {\n\n", pstage->name, pstage->name);

      fprintf( output, "%sac_instruction *instr, *format;\n", INDENT[1]);
      fprintf( output, "%sunsigned ins_id;\n", INDENT[1]);
      fprintf( output, "%sac_instr *instr_vec;\n\n", INDENT[1]);
      fprintf( output, "%sinstr_vec = new ac_instr(regin.read());\n\n", INDENT[1]);
      fprintf( output, "%sins_id = instr_vec->get(IDENT);\n", INDENT[1]);

      fprintf( output, "%sif( ins_id != 0 ) {\n", INDENT[1]);
      fprintf( output, "%sinstr = (ac_instruction *)ISA.instr_table[instr_vec->get(IDENT)][1];\n", INDENT[2]);
      fprintf( output, "%sformat = (ac_instruction *)ISA.instr_table[ins_id][2];\n", INDENT[2]);
      fprintf( output, "%sformat->set_fields( *instr_vec  );\n", INDENT[2]);
      fprintf( output, "%sinstr->set_fields( *instr_vec  );\n", INDENT[2]);
      fprintf( output, "%sISA.instruction.behavior( (ac_stage_list)id );\n", INDENT[2]);
      fprintf( output, "%sformat->behavior( (ac_stage_list)id );\n", INDENT[2]);
      fprintf( output, "%sinstr->behavior( (ac_stage_list)id );\n", INDENT[2]);
      fprintf( output, "%s}\n", INDENT[1]);

      if( pstage->id != stage_num)
        fprintf( output, "%sregout.write( *instr_vec);\n", INDENT[1]);

      fprintf( output, "%sdelete instr_vec;\n", INDENT[1]);
      fprintf( output, "%sbhv_done.write(1);\n", INDENT[1]);
      fprintf( output, "}\n\n");
    }

    //First Stage needs to fetch and decode instructions ....
    else{
      if( pipe_name )			
        fprintf( output, "void AC_%s_%s::%s_%s::behavior() {\n\n", pipe_name, pstage->name, pipe_name, pstage->name);
      else
        fprintf( output, "void AC_%s::%s::behavior() {\n\n", pstage->name, pstage->name);
			
      fprintf( output, "%sac_instruction *instr, *format;\n", INDENT[1]);
      fprintf( output, "%sunsigned  ins_id;\n", INDENT[1]);

      if( ACDebugFlag ){
        fprintf( output, "%sextern bool ac_do_trace;\n", INDENT[1]);
        fprintf( output, "%sextern ofstream trace_file;\n", INDENT[1]);
      }

      if( ACABIFlag ){
        /*fprintf( output, "%s%s_syscall syscall;\n", INDENT[1], project_name);*/
        fprintf( output, "%sstatic int flushes_left=7;\n", INDENT[1]);
        fprintf( output, "%sstatic ac_instr *the_nop = new ac_instr;\n", INDENT[1]);
      }			
			
      if( ACVerifyFlag ){
        fprintf( output, "%sextern int msqid;\n", INDENT[1]);
        fprintf( output, "%sstruct log_msgbuf end_log;\n", INDENT[1]);
      }

      if( ac_host_endian == 0 ){
        fprintf( output, "%schar fetch[AC_WORDSIZE/8];\n\n", INDENT[1]);
      }

      if(ACDecCacheFlag)
        fprintf( output, "%scache_item* ins_cache;\n", INDENT[1]);


      fprintf( output, "%sextern unsigned int decode_pc, quant;\n", INDENT[1]);
      fprintf( output, "%sextern unsigned char buffer[AC_MAX_BUFFER];\n", INDENT[1]);
      //      fprintf( output, "%sunsigned *instr_dec;\n", INDENT[1]);
      fprintf( output, "%sac_instr *instr_vec;\n\n", INDENT[1]);

      EmitFetchInit(output, 1);
      base_indent =2;

      if(ACABIFlag){

        base_indent++;
        //Emiting system calls handler.
        COMMENT(INDENT[1],"Handling System calls.");
        fprintf( output, "%sif (decode_pc %% 2) decode_pc--;\n", INDENT[2]);

        fprintf( output, "%sswitch( decode_pc ){\n", INDENT[2]);

        EmitPipeABIDefine(output);
        fprintf( output, "\n\n");
        EmitABIAddrList(output,3);

        fprintf( output, "%sdefault:\n", INDENT[2]);
      }

      EmitDecodification(output, 1);
      EmitInstrExec(output, base_indent);

      fprintf( output, "%sac_instr_counter++;\n", INDENT[3]);

      if( ACABIFlag ){
        //Closing default case.
        fprintf( output, "%sbreak;\n", INDENT[3]);
        //Closing switch
        fprintf( output, "%s}\n", INDENT[2]);
      }

      fprintf( output, "%sbhv_done.write(1);\n", INDENT[2]);
      //Closing else
      fprintf( output, "%s}\n", INDENT[1]);
      fprintf( output, "}\n\n");
    }

    fclose(output);
    free(stage_filename);
  }
}

//!Creates Processor Module Implementation File
void CreateProcessorImpl() {

  extern ac_stg_list *stage_list;
  extern ac_pipe_list *pipe_list;
  extern ac_stg_list *stage_list;
  extern ac_sto_list *storage_list;
  extern char *project_name;
  extern int stage_num;
  extern int HaveMultiCycleIns, HaveMemHier;
  extern int ACGDBIntegrationFlag;
  ac_sto_list *pstorage;
  ac_stg_list *pstage;
  ac_pipe_list *ppipe;
  int i;

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

  if( ACVerifyFlag ){
    fprintf( output, "#include  \"ac_msgbuf.H\"\n");
    fprintf( output, "#include  \"sys/msg.h\"\n");
  }

  if( ACABIFlag )
    fprintf( output, "#include  \"%s_syscall.H\"\n\n", project_name);
		
  fprintf( output, "void %s::behavior() {\n\n", project_name);
  if( ACDebugFlag ){
    fprintf( output, "%sextern bool ac_do_trace;\n", INDENT[1]);
    fprintf( output, "%sextern ofstream trace_file;\n", INDENT[1]);
  }
  fprintf( output, "%sunsigned ins_id;\n", INDENT[1]);

  if( ACVerifyFlag ){
    fprintf( output, "%sextern int msqid;\n", INDENT[1]);
    fprintf( output, "%sstruct log_msgbuf end_log;\n", INDENT[1]);
  }
/*   if( ACABIFlag ) */
/*     fprintf( output, "%s%s_syscall syscall;\n", INDENT[1], project_name); */

  if(ACDecCacheFlag)
    fprintf( output, "%scache_item_t* ins_cache;\n", INDENT[1]);

/*   if( ac_host_endian == 0 ){ */
/*     fprintf( output, "%schar fetch[AC_WORDSIZE/8];\n\n", INDENT[1]); */
/*   } */

  /* Delayed program loading */
  fprintf(output, "%sif (has_delayed_load) {\n", INDENT[1]);
  fprintf(output, "%sAPP_MEM->load(delayed_load_program);\n", INDENT[2]);
  fprintf(output, "%sac_pc = ac_start_addr;\n", INDENT[2]);
  fprintf(output, "%shas_delayed_load = false;\n", INDENT[2]);
  fprintf(output, "%s}\n\n", INDENT[1]);

  if( HaveMemHier ) {
    fprintf( output, "%sif( ac_wait_sig ) {\n", INDENT[1]);
    fprintf( output, "%sreturn;\n", INDENT[2]);
    fprintf( output, "%s}\n\n", INDENT[1]);
  }

  //Emiting processor behavior method implementation.
  if( HaveMultiCycleIns )
    EmitMultiCycleProcessorBhv(output);
  else{
    if( ACABIFlag )
      EmitProcessorBhv_ABI(output);
    else
      EmitProcessorBhv(output);
  }

  //!Emit update method.
  if( stage_list )
    EmitPipeUpdateMethod( output);
  else if ( pipe_list )
    EmitMultiPipeUpdateMethod( output);
  else
    EmitUpdateMethod( output);

  fprintf( output, " \n");

  //Emiting Verification Method.
  if (ACVerboseFlag || ACVerifyFlag || ACVerifyTimedFlag) {
    COMMENT(INDENT[0],"Verification method.\n");
    fprintf( output, "%svoid %s::ac_verify(){\n", INDENT[0], project_name);

    if( ACVerifyFlag ){

      fprintf( output, "extern int msqid;\n");
      fprintf( output, "struct log_msgbuf lbuf;\n");
      fprintf( output, "log_list::iterator itor;\n");
      fprintf( output, "log_list *plog;\n");
    }
    fprintf( output, " \n");


    fprintf(output, "%sfor (;;) {\n\n", INDENT[1]);

    fprintf( output, "%sif( ", INDENT[1]);

    if(stage_list){
      for( i =1; i<= stage_num-1; i++)
        fprintf( output, "st%d_done.read() && \n%s", i, INDENT[2]);
      fprintf( output, "st%d_done.read() )\n", stage_num);
    }
    else if ( pipe_list ){

      for( ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next ){

        for( pstage = ppipe->stages; pstage->next != NULL; pstage=pstage->next)
          fprintf( output, "%s%s_%s_done.read() && \n", INDENT[1], ppipe->name, pstage->name);

        if( ppipe->next )  //If we have another pipe in the list, do it normally, otherwise, close if condition
          fprintf( output, "%s%s_%s_done.read() && \n", INDENT[1], ppipe->name, pstage->name);
        else
          fprintf( output, "%s%s_%s_done.read() )\n", INDENT[1], ppipe->name, pstage->name);

      }
    }
    else{
      fprintf( output, "done.read() )\n");
    }

    fprintf( output, "%s  {\n", INDENT[2]);


    fprintf( output, "#ifdef AC_VERBOSE\n");
    for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){
      fprintf( output, "%s%s.change_dump(cerr);\n", INDENT[3],pstorage->name );
    }
    fprintf( output, "#endif\n");


    fprintf( output, "#ifdef AC_UPDATE_LOG\n");

    if( ACVerifyFlag ){

      int next_type = 3;

      fprintf( output, "%sif( sc_simulation_time() ){\n", INDENT[3]);

      //Sending logs for every storage device. We just consider for co-verification caches, regbanks and memories
      for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){

        if( pstorage->type == MEM ||
            pstorage->type == ICACHE ||
            pstorage->type == DCACHE ||
            pstorage->type == CACHE ||
            pstorage->type == REGBANK ){

          fprintf( output, "%splog = %s.get_changes();\n", INDENT[4],pstorage->name );
          fprintf( output, "%sif( plog->size()){\n", INDENT[4] );
          fprintf( output, "%sitor = plog->begin();\n", INDENT[5] );
          fprintf( output, "%slbuf.mtype = %d;\n", INDENT[5], next_type );
          fprintf( output, "%swhile( itor != plog->end()){\n\n", INDENT[5] );
          fprintf( output, "%slbuf.log = *itor;\n", INDENT[6] );
          fprintf( output, "%sif( msgsnd(msqid, (struct log_msgbuf *)&lbuf, sizeof(lbuf), 0) == -1)\n", INDENT[6] );
          fprintf( output, "%sperror(\"msgsnd\");\n", INDENT[7] );
          fprintf( output, "%sitor = plog->erase(itor);\n", INDENT[6] );
          fprintf( output, "%s}\n", INDENT[5] );
          fprintf( output, "%s}\n\n", INDENT[4] );

          next_type++;
        }
      }
      fprintf( output, "%s}\n\n", INDENT[3] );

    }
    for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){
      //fprintf( output, "%s%s.change_save();\n", INDENT[3],pstorage->name );
      fprintf( output, "%s%s.reset_log();\n", INDENT[3],pstorage->name );
    }

    fprintf( output, "#endif\n");


    if(stage_list){
      for( i =1; i<= stage_num; i++)
        fprintf( output, "%sst%d_done.write(0);\n", INDENT[3], i);
    }
    else  if ( pipe_list ){

      for( ppipe = pipe_list; ppipe != NULL; ppipe = ppipe->next ){
	
        for( pstage = ppipe->stages; pstage != NULL; pstage=pstage->next)
          fprintf( output, "%s%s_%s_done.write(0);\n", INDENT[1], ppipe->name, pstage->name);

      }
    }
    else{
      fprintf( output, "%sdone.write(0);\n", INDENT[3]);
    }

    fprintf( output, "%s  }\n\n", INDENT[2]);

    fprintf(output, "%swait();\n\n", INDENT[1]);
    fprintf(output, "%s}\n", INDENT[1]);

    fprintf( output, "%s}\n\n", INDENT[0]);
  }

  /* SIGNAL HANDLERS */
  fprintf(output, "#include <ac_sighandlers.H>\n\n");

  /* init() and stop() */
  /* init() with no parameters */
  fprintf(output, "void %s::init() {\n",
          project_name);
  fprintf(output, "%sset_args(ac_argc, ac_argv);\n", INDENT[1]);
  fprintf(output, "#ifdef AC_VERIFY\n");
  fprintf(output, "%sset_queue(av[0]);\n", INDENT[1]);
  fprintf(output, "#endif\n\n");
  fprintf(output, "%sac_pc = ac_start_addr;\n", INDENT[1]);
  fprintf(output, "%sISA._behavior_begin();\n", INDENT[1]);
  fprintf(output, "%scerr << \"ArchC: -------------------- Starting Simulation --------------------\" << endl;\n", INDENT[1]);
  fprintf(output, "%sInitStat();\n\n", INDENT[1]);

  fprintf(output, "%ssignal(SIGINT, sigint_handler);\n", INDENT[1]);
  fprintf(output, "%ssignal(SIGTERM, sigint_handler);\n", INDENT[1]);
  fprintf(output, "%ssignal(SIGSEGV, sigsegv_handler);\n", INDENT[1]);
  fprintf(output, "%ssignal(SIGUSR1, sigusr1_handler);\n", INDENT[1]);
  fprintf(output, "#ifdef USE_GDB\n");
  fprintf(output, "%ssignal(SIGUSR2, sigusr2_handler);\n", INDENT[1]);
  fprintf(output, "#endif\n");
  fprintf(output, "#ifndef AC_COMPSIM\n");
  fprintf(output, "%sset_running();\n", INDENT[1]);
  fprintf(output, "#else\n");
  //  fprintf(output, "%sac_pc = 0;\n", INDENT[1]);
  fprintf(output, "%svoid Execute(int argc, char *argv[]);\n", INDENT[1]);
  fprintf(output, "%sExecute(argc, argv);\n", INDENT[1]);
  fprintf(output, "#endif\n");
  fprintf(output, "}\n\n");

  /* init() with 3 parameters */
  fprintf(output, "void %s::init(int ac, char *av[]) {\n", project_name);
  fprintf(output, "%sextern char* appfilename;\n", INDENT[1]);
  fprintf(output, "%sac_init_opt( ac, av);\n", INDENT[1]);
  fprintf(output, "%sac_init_app( ac, av);\n", INDENT[1]);
  fprintf(output, "%sAPP_MEM->load(appfilename);\n", INDENT[1]);
  fprintf(output, "%sset_args(ac_argc, ac_argv);\n", INDENT[1]);
  fprintf(output, "#ifdef AC_VERIFY\n");
  fprintf(output, "%sset_queue(av[0]);\n", INDENT[1]);
  fprintf(output, "#endif\n\n");

  fprintf(output, "%sac_pc = ac_start_addr;\n", INDENT[1]);
  fprintf(output, "%sISA._behavior_begin();\n", INDENT[1]);
  fprintf(output, "%scerr << \"ArchC: -------------------- Starting Simulation --------------------\" << endl;\n", INDENT[1]);
  fprintf(output, "%sInitStat();\n\n", INDENT[1]);

  fprintf(output, "%ssignal(SIGINT, sigint_handler);\n", INDENT[1]);
  fprintf(output, "%ssignal(SIGTERM, sigint_handler);\n", INDENT[1]);
  fprintf(output, "%ssignal(SIGSEGV, sigsegv_handler);\n", INDENT[1]);
  fprintf(output, "%ssignal(SIGUSR1, sigusr1_handler);\n", INDENT[1]);
  fprintf(output, "#ifdef USE_GDB\n");
  fprintf(output, "%ssignal(SIGUSR2, sigusr2_handler);\n", INDENT[1]);
  fprintf(output, "#endif\n");
  fprintf(output, "#ifndef AC_COMPSIM\n");
  fprintf(output, "%sset_running();\n", INDENT[1]);
  fprintf(output, "#else\n");
  //fprintf(output, "%sac_pc = 0;\n", INDENT[1]);
  fprintf(output, "%svoid Execute(int argc, char *argv[]);\n", INDENT[1]);
  fprintf(output, "%sExecute(argc, argv);\n", INDENT[1]);
  fprintf(output, "#endif\n");
  fprintf(output, "}\n\n");

  /* stop() */
  fprintf(output, "//Stop simulation (may receive exit status)\n");
  fprintf(output, "void %s::stop(int status) {\n", project_name);
  fprintf(output, "%scerr << \"ArchC: -------------------- Simulation Finished --------------------\" << endl;\n", INDENT[1]);
  fprintf(output, "%sISA._behavior_end();\n", INDENT[1]);
  fprintf(output, "%sac_stop_flag = 1;\n", INDENT[1]);
  fprintf(output, "%sac_exit_status = status;\n", INDENT[1]);
  fprintf(output, "#ifndef AC_COMPSIM\n");
  fprintf(output, "%sset_stopped();\n", INDENT[1]);
  fprintf(output, "#endif\n");
  fprintf(output, "}\n\n");

  /* Program loading functions */
  /* load() */
  fprintf(output, "void %s::load(char* program) {\n",
          project_name);
  fprintf(output, "%sAPP_MEM->load(program);\n", INDENT[1]);
  fprintf(output, "}\n\n");

  /* delayed_load() */
  fprintf(output, "void %s::delayed_load(char* program) {\n",
          project_name);
  fprintf(output, "%shas_delayed_load = true;\n", INDENT[1]);
  fprintf(output, "%sdelayed_load_program = new char[strlen(program)];\n", INDENT[1]);
  fprintf(output, "%sstrcpy(delayed_load_program, program);\n", INDENT[1]);
  fprintf(output, "}\n\n");

  /* Some simple GDB support methods */
  if (ACGDBIntegrationFlag) {
    /* get_gdbstub() */
    fprintf(output, "// Returns pointer to gdbstub\n");
    fprintf(output, "AC_GDB<%s_parms::ac_word>* %s::get_gdbstub() {\n", project_name, project_name);
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
  fprintf(output, "%sac_arch<%s_parms::ac_word, %s_parms::ac_Hword>::PrintStat();\n", INDENT[1], project_name, project_name);
  fprintf(output, "}\n\n");

  /* GDB enable method */
  if (ACGDBIntegrationFlag) {
    fprintf(output, "// Enables GDB\n");
    fprintf(output, "void %s::enable_gdb(int port) {\n", project_name);
    fprintf(output, "%sgdbstub->set_port(port);\n", INDENT[1]);
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

  extern ac_pipe_list *pipe_list;
  extern ac_sto_list *storage_list, *fetch_device;
  extern ac_stg_list *stage_list;
  extern int HaveMultiCycleIns, HaveMemHier, HaveTLMPorts, HaveTLMIntrPorts, reg_width;
  extern ac_sto_list* load_device;

  extern char *project_name;

  ac_sto_list *pstorage, *pmem;
  ac_stg_list *pstage;
  char Globals[5000];
  char *Globals_p = Globals;
	ac_pipe_list *ppipe;

  FILE *output;
  char filename[256];

  sprintf(filename, "%s_arch.cpp", project_name);

  load_device= storage_list;
  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }


  print_comment( output, "ArchC Resources Implementation file.");

  fprintf( output, "#include \"%s_arch.H\"\n", project_name);

  if(ACVerifyFlag){
    fprintf( output, "#include  \"ac_msgbuf.H\"\n");
    fprintf( output, "#include  <sys/ipc.h>\n");
    fprintf( output, "#include  <unistd.h>\n");
    fprintf( output, "#include  <sys/msg.h>\n");
    fprintf( output, "#include  <sys/types.h>\n");
  }

  fprintf(output, "\n");

  /* Emitting Constructor */
  fprintf(output, "%s%s_arch::%s_arch() :\n", INDENT[0], project_name, project_name);
  fprintf(output, "%sac_arch_dec_if<%s_parms::ac_word, %s_parms::ac_Hword>(%s_parms::AC_MAX_BUFFER),\n", INDENT[1], project_name, project_name, project_name);

  /* Constructing ac_pc */
  fprintf(output, "%sac_pc(\"ac_pc\", 0", INDENT[1]);
  if (ACDelayFlag) {
    fprintf(output, ", time_step");
  }
  fprintf(output, "),\n");

  for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){

    switch( pstorage->type ){

    case REG:

      //Formatted registers have a special class.
      if (pstorage->format != NULL) {
        fprintf(output, "%s%s(\"%s\"", INDENT[1], pstorage->name, pstorage->name);
      }
      else {
        fprintf(output, "%s%s(\"%s\", 0", INDENT[1], pstorage->name, pstorage->name);
      }
      if (ACDelayFlag) {
        fprintf(output, ", time_step");
      }
      fprintf(output, ")");
      break;

    case REGBANK:
      //Emiting register bank. Checking is a register width was declared.
      fprintf( output, "%s%s(\"%s\"", INDENT[1], pstorage->name, pstorage->name);
      if (ACDelayFlag) {
        fprintf(output, ", time_step");
      }
      fprintf(output, ")");
      break;

    case CACHE:
    case ICACHE:
    case DCACHE:

      if( !pstorage->parms ) { //It is a generic cache. Just emit a base container object.
        fprintf(output, "%s%s_stg(\"%s_stg\", %uU),\n", INDENT[1], pstorage->name, pstorage->name, pstorage->size);
        fprintf( output, "%s%s(*this, %s_stg)", INDENT[1], pstorage->name, pstorage->name);
      }
      else{
        //It is an ac_cache object.
        EmitCacheDeclaration(output, pstorage, 0);
      }
      break;

    case MEM:

      if( !HaveMemHier ) { //It is a generic cache. Just emit a base container object.
        fprintf(output, "%s%s_stg(\"%s_stg\", %uU),\n", INDENT[1], pstorage->name, pstorage->name, pstorage->size);
        fprintf( output, "%s%s(*this, %s_stg)", INDENT[1], pstorage->name, pstorage->name);
      }
      else{
        //It is an ac_mem object.
        fprintf(output, "%s%s_stg(\"%s_stg\", %uU),\n", INDENT[1], pstorage->name, pstorage->name, pstorage->size);
        fprintf( output, "%s%s(*this, %s_stg)", INDENT[1], pstorage->name, pstorage->name);
      }
      break;

    case TLM_PORT:
      fprintf(output, "%s%s_port(\"%s_port\", %uU),\n", INDENT[1], pstorage->name, pstorage->name, pstorage->size);
      fprintf( output, "%s%s(*this, %s_port)", INDENT[1], pstorage->name, pstorage->name);
      break;

    default:
      fprintf(output, "%s%s_stg(\"%s_stg\", %uU),\n", INDENT[1], pstorage->name, pstorage->name, pstorage->size);
      fprintf( output, "%s%s(*this, %s_stg)", INDENT[1], pstorage->name, pstorage->name);
      break;
    }
    if (pstorage->next != NULL)
      fprintf(output, ",\n");
  }

  /* opening constructor body */
  fprintf(output, " {\n\n");

  /* setting endianness match */
  fprintf(output, "%sac_mt_endian = %s_parms::AC_MATCH_ENDIAN;\n", INDENT[1], project_name);

  /* setting target endianness */
  fprintf(output, "%sac_tgt_endian = %s_parms::AC_PROC_ENDIAN;\n\n", INDENT[1], project_name);

  /* Determining which device is gonna be used for fetching instructions */
  if( !fetch_device ){
    //The parser has not determined because there is not an ac_icache obj declared.
    //In this case, look for the object with the lowest (zero) hierarchy level.
    for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next)
      if( pstorage->level == 0 && pstorage->type != REG && pstorage->type != REGBANK && pstorage->type != TLM_INTR_PORT)
        fetch_device = pstorage;

    if( !fetch_device ) { //Couldn't find a fetch device. Error!
      AC_INTERNAL_ERROR("Could not determine a device for fetching.");
      exit(1);
    }
  }

  fprintf( output, "%sIM = &%s;\n", INDENT[1], fetch_device->name);

  /* Determining which device is going to be used for loading applications*/
  /* The device used for loading applications must be the one in the highest
     level of a memory hierachy.*/
  for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){
    if(pstorage->level > load_device->level)
      load_device = pstorage;
  }

  /* If there is only one level, which is gonna be zero, then it is the same
     object used for fetching. */
  if( load_device->level ==0 )
    load_device = fetch_device;

  fprintf( output, "%sAPP_MEM = &%s;\n", INDENT[1], load_device->name);

  fprintf( output, "\n");

  /* Connecting memory hierarchy */
  for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next)
    if( pstorage->higher ){
      fprintf( output, "%s%s.bindToNext(%s);\n", INDENT[1], pstorage->name, pstorage->higher->name );
    }

  fprintf( output, "}\n\n");

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
  fprintf( output, "%s%s %s_proc1(\"%s\");\n\n", INDENT[1], project_name, project_name, project_name);

  fprintf( output, "#ifdef AC_DEBUG\n");
  fprintf( output, "%sac_trace(\"%s_proc1.trace\");\n", INDENT[1], project_name);
  fprintf( output, "#endif \n\n");

  if (ACGDBIntegrationFlag == 1)
    fprintf(output, "%s%s_proc1.enable_gdb();\n", INDENT[1], project_name);

  fprintf(output, "%s%s_proc1.init(ac, av);\n", INDENT[1], project_name);
  fprintf(output, "%scerr << endl;\n\n", INDENT[1]);

  fprintf(output, "%ssc_start();\n\n", INDENT[1]);

  fprintf(output, "%s%s_proc1.PrintStat();\n", INDENT[1], project_name);
  fprintf(output, "%scerr << endl;\n\n", INDENT[1]);

  fprintf( output, "#ifdef AC_STATS\n");
  fprintf( output, "%sac_stats_base::print_all_stats(std::cerr);\n", INDENT[1], project_name);
  fprintf( output, "#endif \n\n");

  fprintf( output, "#ifdef AC_DEBUG\n");
  fprintf( output, "%sac_close_trace();\n", INDENT[1]);
  fprintf( output, "#endif \n\n");

  fprintf( output, "%sreturn %s_proc1.ac_exit_status;\n", INDENT[1], project_name);

  fprintf( output, "}\n");
}


/*!Create the template for the .cpp file where the user has
  to fill out the instruction and format behaviors. */
void CreateImplTmpl(){

  extern ac_dec_format *format_ins_list;
  extern ac_stg_list *stage_list;
  extern ac_dec_instr *instr_list;
  extern ac_dec_field *field_list;
  extern ac_grp_list* group_list;
  extern char *project_name;
  extern int wordsize;
  extern int declist_num;
  ac_grp_list* pgroup;
  ac_instr_ref_list* pref;
  ac_dec_format *pformat;
  ac_dec_instr *pinstr;
  ac_stg_list *pstage;
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

  COMMENT(INDENT[0], "'using namespace' statement to allow access to all %s-specific datatypes", project_name);
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
  //Testing if should emit a switch or not.
  if( stage_list ){
    fprintf( output, "%svoid ac_behavior( instruction ){;\n\n", INDENT[0]);
    fprintf( output, "%sswitch( stage ) {\n", INDENT[1]);

    for( pstage = stage_list; pstage != NULL; pstage=pstage->next){
      fprintf( output, "%scase _%s:\n", INDENT[1], pstage->name);
      fprintf( output, "%sbreak;\n", INDENT[1]);
    }

    fprintf( output, "%sdefault:\n", INDENT[1]);
                          fprintf( output, "%sbreak;\n", INDENT[1]);
                          fprintf( output, "%s}\n", INDENT[1]);
                          fprintf( output, "};\n\n");
  }
  else{
    fprintf( output, "%svoid ac_behavior( instruction ){};\n", INDENT[0]);
  }

  fprintf( output, " \n");


  //Declaring Instruction Format behavior methods.
  COMMENT(INDENT[0]," Instruction Format behavior methods.");
  for( pformat = format_ins_list; pformat!= NULL; pformat=pformat->next)
    //Testing if should emit a switch or not.
    if( stage_list ){
      fprintf( output, "%svoid ac_behavior( %s ){\n\n", INDENT[0], pformat->name);
      fprintf( output, "%sswitch( stage ) {\n", INDENT[1]);

      for( pstage = stage_list; pstage != NULL; pstage=pstage->next){
        fprintf( output, "%scase _%s:\n", INDENT[1], pstage->name);
        fprintf( output, "%sbreak;\n", INDENT[1]);
      }

      fprintf( output, "%sdefault:\n", INDENT[1]);
                            fprintf( output, "%sbreak;\n", INDENT[1]);
                            fprintf( output, "%s}\n", INDENT[1]);
                            fprintf( output, "};\n\n");
    }
    else{
      fprintf( output, "%svoid ac_behavior( %s ){}\n", INDENT[0], pformat->name);
    }

  fprintf( output, " \n");


  //Declaring each instruction behavior method.
  for( pinstr = instr_list; pinstr!= NULL; pinstr=pinstr->next){

    //Testing if should emit a switch or not.
    if( stage_list ){
      COMMENT(INDENT[0],"Instruction %s behavior method.",pinstr->name);
      fprintf( output, "%svoid ac_behavior( %s ){\n\n", INDENT[0], pinstr->name);
      fprintf( output, "%sswitch( stage ) {\n", INDENT[1]);

      for( pstage = stage_list; pstage != NULL; pstage=pstage->next){
        fprintf( output, "%scase _%s:\n", INDENT[1], pstage->name);
        fprintf( output, "%sbreak;\n", INDENT[1]);
      }

      fprintf( output, "%sdefault:\n", INDENT[1]);
                            fprintf( output, "%sbreak;\n", INDENT[1]);
                            fprintf( output, "%s}\n", INDENT[1]);
                            fprintf( output, "};\n\n");
    }
    else{
      COMMENT(INDENT[0],"Instruction %s behavior method.",pinstr->name);
      fprintf( output, "%svoid ac_behavior( %s ){}\n\n", INDENT[0], pinstr->name);
    }
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
    /* fprintf char* name, int size, ac_dec_field* fields, next */
    fprintf(output, "%s{\"%s\", %d, &(%s_parms::%s_isa::fields[%d]), ",
            INDENT[1],
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
    for( pdecfield = pformat->fields; pdecfield!= NULL; pdecfield=pdecfield->next)
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
        fprintf(output, "&(%s_parms::%s_isa::dec_list[%d])},\n", project_name, project_name, i + 1);
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
  fprintf(output, "%sac_instr_info(0, \"_ac_invalid_\", \"_ac_invalid_\", %d),\n", INDENT[1], wordsize / 8);
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
  fprintf(output, "\n};\n");

  //!END OF FILE.
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

  COMMENT(INDENT[0], "'using namespace' statement to allow access to all %s-specific datatypes", project_name);
  fprintf( output, "using namespace %s_parms;\n\n", project_name);

  //Declaring formatted register behavior methods.
  for (pport = tlm_intr_port_list; pport != NULL; pport = pport->next) {
    fprintf(output, "// Interrupt handler behavior for interrupt port %s.\n", pport->name);
    fprintf(output, "void ac_behavior(%s, value) {\n}\n\n", pport->name);
  }

  //END OF FILE.
  fclose(output);
}


//!Creates Formatted Registers Implementation File.
void CreateRegsImpl() {
  extern ac_sto_list *storage_list;
  extern char* project_name;

  ac_sto_list *pstorage;

  FILE *output;
  char filename[256];
  char description[] = "Formatted Register Behavior implementation file.";

  sprintf(filename, "%s_regs.cpp.tmpl", project_name);
  
  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }



  print_comment( output, description);

  fprintf( output, "#include \"%s_fmt_regs.H\"\n", project_name);
  fprintf( output, "#include \"ac_utils.H\"\n");
  fprintf( output, " \n");

  //Declaring formatted register behavior methods.
  for( pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next ){

    if(( pstorage->type == REG ) && (pstorage->format != NULL )){
      fprintf( output, "%svoid ac_behavior( %s ){}\n", INDENT[0], pstorage->name);
    }
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
    fprintf(output, "%sexplicit %s_%s_handler(%s_arch& ref) : %s_arch_ref(ref) {}\n\n",
            INDENT[1], project_name, pport->name, project_name, project_name);
    fprintf(output, "%svoid handle(uint32_t value);\n\n", INDENT[1]);
    fprintf(output, "};\n\n\n");
  }

  fprintf(output, "#endif // _%s_INTR_HANDLERS_H\n", upper_project_name);

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
  fprintf(output, "#define ac_behavior(intrp, value) %s_##intrp##_handler::handle(uint32_t value)\n\n",
          project_name);

  fprintf(output, "#endif // _%s_IH_BHV_MACROS_H\n", upper_project_name);

  //END OF FILE
  fclose(output);

}


//!Create Makefile
void CreateMakefile(){

  extern ac_dec_format *format_ins_list;
  extern ac_pipe_list* pipe_list;
  extern ac_stg_list* stage_list;
  extern char *project_name;
  extern int HaveMemHier;
  extern int HaveFormattedRegs;
  extern int HaveTLMPorts;
  extern int HaveTLMIntrPorts;
  ac_stg_list *pstage;
  ac_pipe_list *ppipe;
  ac_dec_format *pformat;
  FILE *output;
  char filename[] = "Makefile.archc";

  if ( !(output = fopen( filename, "w"))){
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

  fprintf( output, "\n\n");

  COMMENT_MAKE("Variable that points to SystemC installation path");
  fprintf( output, "SYSTEMC := %s\n", SYSTEMC_PATH);

  fprintf( output, "\n\n");
  COMMENT_MAKE("Variable that points to ArchC installation path");
  fprintf( output, "ARCHC := %s\n", BINDIR);

  fprintf( output, "\n");

  COMMENT_MAKE("Target Arch used by SystemC");
  fprintf( output, "TARGET_ARCH := %s\n", TARGET_ARCH);

  fprintf( output, "\n\n");

  fprintf( output, "INC_DIR := -I. -I%s -I$(SYSTEMC)/include ", INCLUDEDIR);
  if (HaveTLMPorts || HaveTLMIntrPorts)
    fprintf(output, "-I%s", TLM_PATH);
  fprintf(output, "\n");
  fprintf( output, "LIB_DIR := -L. -L$(SYSTEMC)/lib-$(TARGET_ARCH) -L%s\n", LIBDIR);

  fprintf( output, "\n");

  fprintf( output, "LIB_SYSTEMC := %s\n",
           (strlen(SYSTEMC_PATH) > 2) ? "-lsystemc" : "");
  fprintf( output, "LIBS := $(LIB_SYSTEMC) -lm $(EXTRA_LIBS) -larchc\n");
  fprintf( output, "CC :=  %s\n", CC_PATH);
  fprintf( output, "OPT :=  %s\n", OPT_FLAGS);
  fprintf( output, "DEBUG :=  %s\n", DEBUG_FLAGS);
  fprintf( output, "OTHER :=  %s\n", OTHER_FLAGS);
  fprintf( output, "CFLAGS := $(DEBUG) $(OPT) $(OTHER) %s\n",
           (ACGDBIntegrationFlag) ? "-DUSE_GDB" : "" );

  fprintf( output, "\n");

  fprintf( output, "MODULE := %s\n", project_name);

  fprintf( output, "\n");

  //Declaring ACSRCS variable

  COMMENT_MAKE("These are the source files automatically generated by ArchC, that must appear in the SRCS variable");
  fprintf( output, "ACSRCS := $(MODULE)_arch.cpp $(MODULE)_arch_ref.cpp ");

  //Checking if we have a pipelined architecture or not.
  if( stage_list  ){  //List of ac_stage declarations. Used only for single pipe archs

    for( pstage = stage_list; pstage!= NULL; pstage = pstage->next)
      fprintf( output, "%s.cpp ", pstage->name);
  }
  else if( pipe_list ){  //Pipeline list exist. Used for ac_pipe declarations.

    for(ppipe = pipe_list; ppipe!= NULL; ppipe=ppipe->next){

      for(pstage=ppipe->stages; pstage!= NULL; pstage=pstage->next)
        fprintf( output, "%s_%s.cpp ", ppipe->name, pstage->name);
    }
  }
  else{    //No pipe was declared. There is just the processor module source file

    fprintf( output, "$(MODULE).cpp");
  }

  fprintf( output, "\n\n");

  //Declaring ACINCS variable
  COMMENT_MAKE("These are the source files automatically generated  by ArchC that are included by other files in ACSRCS");
  fprintf( output, "ACINCS := $(MODULE)_isa_init.cpp");

  fprintf( output, "\n\n");

  //Declaring ACHEAD variable

  COMMENT_MAKE("These are the header files automatically generated by ArchC");
  fprintf( output, "ACHEAD := $(MODULE)_parms.H $(MODULE)_arch.H $(MODULE)_arch_ref.H $(MODULE)_isa.H $(MODULE)_bhv_macros.H ");

  if(HaveFormattedRegs)
    fprintf( output, "$(MODULE)_fmt_regs.H ");

  if(ACStatsFlag)
    fprintf( output, "%s_stats.H ", project_name);

  //Checking if we have a pipelined architecture or not.
  if( stage_list  ){  //List of ac_stage declarations. Used only for single pipe archs

    for( pstage = stage_list; pstage!= NULL; pstage = pstage->next)
      fprintf( output, "%s.H ", pstage->name);
  }
  else if( pipe_list ){  //Pipeline list exist. Used for ac_pipe declarations.

    for(ppipe = pipe_list; ppipe!= NULL; ppipe=ppipe->next){

      for(pstage=ppipe->stages; pstage!= NULL; pstage=pstage->next)
        fprintf( output, "%s_%s.H ", ppipe->name, pstage->name);
    }
  }
  else{    //No pipe was declared. There is just the processor module source file

    fprintf( output, "$(MODULE).H ");
  }

  /*  if(ACABIFlag)
      fprintf( output, "$(MODULE)_syscall.H ");*/

  if(HaveTLMIntrPorts)
    fprintf( output, "$(MODULE)_intr_handlers.H $(MODULE)_ih_bhv_macros.H ");

  fprintf( output, "\n\n");


  //Declaring FILES variable
  COMMENT_MAKE("These are the source files provided by ArchC that must be compiled together with the ACSRCS");
  COMMENT_MAKE("They are stored in the archc/src/aclib directory");
  fprintf( output, "ACFILES := ");


  if( HaveMemHier )
    fprintf( output, "ac_cache.cpp ac_mem.cpp ac_cache_if.cpp ");

  fprintf( output, "\n\n");

  //Declaring ACLIBFILES variable
  COMMENT_MAKE("These are the library files provided by ArchC");
  COMMENT_MAKE("They are stored in the archc/lib directory");
  fprintf(output, "ACLIBFILES := ac_decoder_rt.o ac_module.o ac_storage.o ac_utils.o ");
  if(ACABIFlag)
    fprintf(output, "ac_syscall.o ");
  if(HaveTLMPorts)
    fprintf(output, "ac_tlm_port.o ");
  if(HaveTLMIntrPorts)
    fprintf(output, "ac_tlm_intr_port.o ");
  fprintf(output, "\n\n");

  //Declaring FILESHEAD variable
  COMMENT_MAKE("These are the headers files provided by ArchC");
  COMMENT_MAKE("They are stored in the archc/include directory");
  fprintf( output, "ACFILESHEAD := $(ACFILES:.cpp=.H) ac_decoder_rt.H ac_module.H ac_storage.H ac_utils.H ac_regbank.H ac_debug_model.H ac_sighandlers.H ac_ptr.H ac_memport.H ac_arch.H ac_arch_dec_if.H ac_arch_ref.H ");
  if (ACABIFlag)
    fprintf(output, "ac_syscall.H ");
  if (HaveTLMPorts)
    fprintf(output, "ac_tlm_port.H ");
  if (HaveTLMIntrPorts)
    fprintf(output, "ac_tlm_intr_port.H ");
  if (HaveTLMPorts || HaveTLMIntrPorts)
    fprintf(output, "ac_tlm_protocol.H ");
  if (ACStatsFlag)
    fprintf(output, "ac_stats.H ac_stats_base.H ");
  fprintf(output, "\n\n");

  //Declaring SRCS variable
  //Note: Removed $(MODULE)_isa.cpp, added as include inside $(MODULE).cpp
  COMMENT_MAKE("These are the source files provided by the user + ArchC sources");
  fprintf( output, "SRCS := main.cpp $(ACSRCS) $(ACFILES) %s",
           (ACGDBIntegrationFlag)?"$(MODULE)_gdb_funcs.cpp":""); 

  if (ACABIFlag)
    fprintf( output, " $(MODULE)_syscall.cpp");

  if (HaveTLMIntrPorts)
    fprintf( output, " $(MODULE)_intr_handlers.cpp");

  if (ACStatsFlag)
    fprintf( output, " $(MODULE)_stats.cpp");

  fprintf( output, "\n\n");

  //Declaring OBJS variable
  fprintf( output, "OBJS := $(SRCS:.cpp=.o)\n\n");

  fprintf( output, "\n");
  //Declaring Executable name
  fprintf( output, "EXE := $(MODULE).x\n\n");

  //Declaring dependencie rules
  fprintf( output, ".SUFFIXES: .cc .cpp .o .x\n\n");

  fprintf( output, "all: $(addprefix %s/, $(ACFILESHEAD))", INCLUDEDIR);
  if (ACABIFlag)
    fprintf( output, " $(MODULE)_syscall.H");
  fprintf( output, " $(ACHEAD) $(ACFILES) $(EXE)\n\n");

  fprintf( output, "$(EXE): $(OBJS) %s\n",
           (strlen(SYSTEMC_PATH) > 2) ? "$(SYSTEMC)/lib-$(TARGET_ARCH)/libsystemc.a" : "");
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

  if (HaveTLMIntrPorts) {
    COMMENT_MAKE("Copy from template if %s_intr_handlers.cpp not exist", project_name);
    fprintf( output, "%s_intr_handlers.cpp:\n", project_name);
    fprintf( output, "\tcp %s_intr_handlers.cpp.tmpl %s_intr_handlers.cpp\n\n", project_name, project_name);
  }

  fprintf( output, ".cpp.o:\n");
  fprintf( output, "\t$(CC) $(CFLAGS) $(INC_DIR) -c $<\n\n");

  fprintf( output, ".cc.o:\n");
  fprintf( output, "\t$(CC) $(CFLAGS) $(INC_DIR) -c $<\n\n");

  fprintf( output, "clean:\n");
  fprintf( output, "\trm -f $(OBJS) *~ $(EXE) core *.o \n\n");

  fprintf( output, "model_clean:\n");
  fprintf( output, "\trm -f $(ACSRCS) $(ACHEAD) $(ACINCS) $(ACFILESHEAD) $(ACFILES) *.tmpl loader.ac \n\n");

  fprintf( output, "sim_clean: clean model_clean\n\n");

  fprintf( output, "distclean: sim_clean\n");
  fprintf( output, "\trm -f main.cpp Makefile.archc\n\n");

}


////////////////////////////////////////////////////////////////////////////////////
// Emit functions ...                                                             //
// These Functions are used by the Create functions declared above to write files //
////////////////////////////////////////////////////////////////////////////////////

/**************************************/
/*!Emit the generic instruction class
  Used by CreateTypesHeader function. */
/***************************************/
void EmitGenInstrClass(FILE *output) {
  extern ac_dec_format *format_ins_list;
  ac_dec_format *pformat;
  ac_dec_field *pfield , *pgenfield, *pf, *ppf;
  int initializing = 1;
	
  /* Emiting generic instruction class declaration */
  COMMENT(INDENT[0],"Generic Instruction Class declaration.\n");
  fprintf( output, "class ac_instruction: public ac_resources {\n");
  fprintf( output, "protected:\n");
  fprintf( output, "%schar* ac_instr_name;\n", INDENT[1]);
  fprintf( output, "%schar* ac_instr_mnemonic;\n", INDENT[1]);
  fprintf( output, "%sunsigned ac_instr_size;\n", INDENT[1]);
  fprintf( output, "%sunsigned ac_instr_cycles;\n", INDENT[1]);
  fprintf( output, "%sunsigned ac_instr_min_latency;\n", INDENT[1]);
  fprintf( output, "%sunsigned ac_instr_max_latency;\n", INDENT[1]);

  // THIS NEEDS TO GO INTO THE ISA MACROS AND STUFF
  //Selecting fields that are common to all formats.
  for( pformat = format_ins_list; pformat!= NULL; pformat=pformat->next){

    if( initializing ){

      //This is the first format being processed. Put all of its fields.
      for( pfield = pformat->fields; pfield != NULL; pfield = pfield->next){

        pf = (ac_dec_field*) malloc( sizeof(ac_dec_field));
        pf = memcpy( pf, pfield, sizeof(ac_dec_field) );

        if( pfield == pformat->fields ){
          pgenfield = pf;
          pgenfield->next = NULL;
        }
        else{
          pf->next = pgenfield;
          pgenfield = pf;
        }
      }
      initializing =0;
			
    }
    else{  //We already have candidate fields. Check if they are present in all formats.

      ppf = NULL;

      //Keep fields that are common to all instructions
      pf = pgenfield;

      while( pf ){

        //Looking for pf into pformat
        for( pfield = pformat->fields; pfield != NULL; pfield = pfield->next){

          if( !strcmp( pf->name, pfield->name) )
            break;
        }

        if( !pfield) { //Did not find. Delete pf from pgenfield
					
          if(ppf){
            ppf->next = pf->next;
            free(pf);
            pf = ppf->next;
          }
          else{  //Deleting the first field
            pgenfield = pf->next;
            free(pf);
            pf = pgenfield;
          }
        }
        else{  //Found. Keep the field and step to the next.
          ppf = pf;
          pf = pf->next;
        }
      }
    }	
  }

  //pgenfield has the list of fields for the generic instruction.
  for( pfield = pgenfield; pfield != NULL; pfield = pfield->next){
    if( pfield->sign )
      fprintf( output,"%sint %s;\n",INDENT[1],pfield->name );
    else
      fprintf( output,"%sunsigned %s;\n",INDENT[1],pfield->name );
  }

  //Now emiting public methods
  fprintf( output, "public:\n");

  fprintf( output, "%sac_instruction( char* name, char* mnemonic, unsigned min, unsigned max ){ ac_instr_name = name ; ac_instr_mnemonic = mnemonic; ac_instr_min_latency = min, ac_instr_max_latency =max;}\n", INDENT[1]);
  fprintf( output, "%sac_instruction( char* name, char* mnemonic ){ ac_instr_name = name ; ac_instr_mnemonic = mnemonic;}\n", INDENT[1]);
  fprintf( output, "%sac_instruction( char* name ){ ac_instr_name = name ;}\n", INDENT[1]);
  fprintf( output, "%sac_instruction( ){ ac_instr_name = \"NULL\";}\n", INDENT[1]);

  fprintf( output, "%svirtual void behavior(ac_stage_list  stage = (ac_stage_list)0, unsigned cycle=0);\n", INDENT[1]);

  fprintf( output, "%svoid set_cycles( unsigned c){ ac_instr_cycles = c;}\n", INDENT[1]);
  fprintf( output, "%sunsigned get_cycles(){ return ac_instr_cycles;}\n", INDENT[1]);

  fprintf( output, "%svoid set_min_latency( unsigned c){ ac_instr_min_latency = c;}\n", INDENT[1]);
  fprintf( output, "%sunsigned get_min_latency(){ return ac_instr_min_latency;}\n", INDENT[1]);

  fprintf( output, "%svoid set_max_latency( unsigned c){ ac_instr_max_latency = c;}\n", INDENT[1]);
  fprintf( output, "%sunsigned get_max_latency(){ return ac_instr_max_latency;}\n", INDENT[1]);

  fprintf( output, "%sunsigned get_size() {return ac_instr_size;}\n", INDENT[1]);
  fprintf( output, "%svoid set_size( unsigned s) {ac_instr_size = s;}\n", INDENT[1]);

  fprintf( output, "%schar* get_name() {return ac_instr_name;}\n", INDENT[1]);
  fprintf( output, "%svoid set_name( char* name) {ac_instr_name = name;}\n", INDENT[1]);

  fprintf( output, "%svirtual void set_fields( ac_instr instr ){\n",INDENT[1]);

  for( pfield = pgenfield; pfield != NULL; pfield = pfield->next)
    fprintf( output,"%s%s = instr.get(%d); \n", INDENT[2],pfield->name, pfield->id );
	
  //fprintf( output,"%sac_instr_size = %d; \n", INDENT[2], pformat->size );
  fprintf( output, "%s}\n", INDENT[1]);



  fprintf( output, "%svirtual  void print (ostream & os) const{};\n", INDENT[1]);

  fprintf( output, "%sfriend ostream& operator<< (ostream &os,const ac_instruction &ins){\n", INDENT[1]);
  fprintf( output, "%sins.print(os);\n", INDENT[1]);
  fprintf( output, "%sreturn os;\n", INDENT[1]);
  fprintf( output, "%s};\n", INDENT[1]);

  fprintf( output, "};\n\n");
	
}

/**************************************/
/*!Emit one class for each format declared.
  Used by CreateTypesHeader function. */
/***************************************/
void EmitFormatClasses(FILE *output) {
  extern ac_dec_format *format_ins_list;
  ac_dec_format *pformat;
  ac_dec_field *pfield;

  /* Emiting format class declaration */
  COMMENT(INDENT[0],"Instruction Format class declarations.\n");

  for( pformat = format_ins_list; pformat!= NULL; pformat=pformat->next){

    fprintf( output, "class ac_%s: public ac_instruction {\n", pformat->name);
    fprintf( output, "protected:\n");

    for( pfield = pformat->fields; pfield != NULL; pfield = pfield->next){
      if( pfield->sign )
        fprintf( output,"%sint %s;\n",INDENT[1],pfield->name );
      else
        fprintf( output,"%sunsigned %s;\n",INDENT[1],pfield->name );
    }

    fprintf( output, "public:\n");
    fprintf( output, "%sac_%s( char* name, char* mnemonic, unsigned min, unsigned max ):ac_instruction(name, mnemonic, min, max){};\n", INDENT[1], pformat->name);
    fprintf( output, "%sac_%s( char* name, char *mnemonic ):ac_instruction(name, mnemonic) {};\n", INDENT[1], pformat->name);
    fprintf( output, "%sac_%s( char* name ):ac_instruction(name) {};\n", INDENT[1], pformat->name);
    fprintf( output, "%sac_%s( ):ac_instruction() {};\n", INDENT[1], pformat->name);
    fprintf( output, "%svoid set_fields( ac_instr instr ){\n",INDENT[1]);

    for( pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
      fprintf( output,"%s%s = instr.get(%d); \n", INDENT[2],pfield->name, pfield->id );

    fprintf( output,"%sac_instr_size = %d; \n", INDENT[2], pformat->size );

    fprintf( output, "%s}\n", INDENT[1]);

    fprintf( output, "%svirtual void behavior( ac_stage_list stage=(ac_stage_list)0, unsigned cycle=0 );\n", INDENT[1]);


    //Print method
    fprintf( output, "%svirtual void print (ostream & os) const{\n", INDENT[1]);
    fprintf( output, "%sos ", INDENT[2]);

    for( pfield = pformat->fields; pfield != NULL; pfield = pfield->next)
      if(pfield->next)
        fprintf( output, " << \"%s: \" << %s << \", \" ",  pfield->name,  pfield->name);
      else
        fprintf( output, " << \"%s: \" << %s;", pfield->name,  pfield->name);

    fprintf( output, "\n}\n");

    fprintf( output, "};\n\n");
  }
}

/*!*************************************/
/*! Emit  instruction class declarations
  Used by CreateTypesHeader function. */
/***************************************/
void EmitInstrClasses( FILE *output){

  extern ac_dec_instr *instr_list;
  ac_dec_instr *pinstr;

  COMMENT(INDENT[0],"Instruction class declarations.\n");

  for( pinstr = instr_list; pinstr!= NULL; pinstr=pinstr->next){
    fprintf( output, "class ac_%s: public ac_%s {\n", pinstr->name, pinstr->format);
    fprintf( output, "%spublic:\n", INDENT[0]);
    fprintf( output, "%sac_%s( char* name, char* mnemonic, unsigned min, unsigned max ):ac_%s(name, mnemonic, min, max){};\n", INDENT[1], pinstr->name, pinstr->format);
    fprintf( output, "%sac_%s( char* name, char *mnemonic ):ac_%s(name, mnemonic) {};\n", INDENT[1], pinstr->name, pinstr->format);
    fprintf( output, "%sac_%s( char* name ):ac_%s(name) {};\n", INDENT[1], pinstr->name, pinstr->format);
    fprintf( output, "%sac_%s( ):ac_%s() {};\n", INDENT[1], pinstr->name, pinstr->format);
    fprintf( output, "%svoid behavior( ac_stage_list  stage = (ac_stage_list)0, unsigned cycle=0 );\n", INDENT[1]);

    //Print method
    fprintf( output, "%svoid print (ostream & os) const{\n", INDENT[1]);
    fprintf( output, "%sos << ac_instr_mnemonic << \"\t\";", INDENT[2]);
    fprintf( output, "}\n");

    fprintf( output, "};\n\n");
  }
}

/**************************************/
/*! Emit declaration of decoding structures
  Used by CreateISAHeader function.   */
/***************************************/
void EmitDecStruct( FILE* output){
  extern ac_dec_format *format_ins_list;
  extern ac_dec_instr *instr_list;
  extern int declist_num;
  extern int HaveMultiCycleIns;
  ac_dec_field *pdecfield;
  ac_dec_format *pformat;
  ac_dec_instr *pinstr;
  ac_dec_list *pdeclist;
  int i;
  int count_fields;

  //fprintf( output, "%sunsigned counter;\n", INDENT[2]);
  fprintf( output, "%sextern ac_dec_field *fields;\n", INDENT[2]);
  fprintf( output, "%sextern ac_dec_format *formats;\n", INDENT[2]);
  fprintf( output, "%sextern ac_dec_list *dec_list;\n", INDENT[2]);
  fprintf( output, "%sextern ac_dec_instr *instructions;\n\n", INDENT[2]);

  //Field Structure
  fprintf( output, "%sfields = (ac_dec_field*) malloc(sizeof(ac_dec_field)*AC_DEC_FIELD_NUMBER);\n", INDENT[2]);
  i = 0;
  for( pformat = format_ins_list; pformat!= NULL; pformat=pformat->next){

    for( pdecfield = pformat->fields; pdecfield!= NULL; pdecfield=pdecfield->next){
      fprintf( output, "%sfields[%d].name      = \"%s\";\n", INDENT[2], i, pdecfield->name);
      fprintf( output, "%sfields[%d].size      = %d;\n", INDENT[2], i, pdecfield->size);
      fprintf( output, "%sfields[%d].first_bit = %d;\n", INDENT[2], i, pdecfield->first_bit);
      fprintf( output, "%sfields[%d].id        = %d;\n", INDENT[2], i, pdecfield->id);
      fprintf( output, "%sfields[%d].val       = %ld;\n", INDENT[2], i, pdecfield->val);
      fprintf( output, "%sfields[%d].sign      = %d;\n", INDENT[2], i, pdecfield->sign);
      if(pdecfield->next)
        fprintf( output, "%sfields[%d].next      = &(fields[%d]);\n\n", INDENT[2], i, i+1);
      else
        fprintf( output, "%sfields[%d].next      = NULL;\n\n", INDENT[2], i);
      i++;
    }
  }

  fprintf( output, " \n");

  //Format Structure
  fprintf( output, "%sformats = (ac_dec_format*) malloc(sizeof(ac_dec_format)*AC_DEC_FORMAT_NUMBER);\n", INDENT[2]);
  i = 0;
  count_fields = 0;
  for( pformat = format_ins_list; pformat!= NULL; pformat=pformat->next){

    fprintf( output, "%sformats[%d].name      = \"%s\";\n", INDENT[2], i, pformat->name);
    fprintf( output, "%sformats[%d].fields    = &(fields[%d]);\n", INDENT[2], i, count_fields);
    if(pformat->next)
      fprintf( output, "%sformats[%d].next      = &(formats[%d]);\n\n", INDENT[2], i, i+1);
    else
      fprintf( output, "%sformats[%d].next      = NULL;\n\n", INDENT[2], i);
    i++;
    for( pdecfield = pformat->fields; pdecfield!= NULL; pdecfield=pdecfield->next)
      count_fields++;
  }

  fprintf( output, " \n");

  //Decode list Structure
  fprintf( output, "%sdec_list = (ac_dec_list*) malloc(sizeof(ac_dec_list)*AC_DEC_LIST_NUMBER);\n", INDENT[2]);
  i = 0;
  for( pinstr = instr_list; pinstr!= NULL; pinstr=pinstr->next){

    for( pdeclist = pinstr->dec_list; pdeclist!= NULL; pdeclist=pdeclist->next){
      fprintf( output, "%sdec_list[%d].name      = \"%s\";\n", INDENT[2], i, pdeclist->name);
      fprintf( output, "%sdec_list[%d].id        = %d;\n", INDENT[2], i, pdeclist->id);
      fprintf( output, "%sdec_list[%d].value     = %d;\n", INDENT[2], i, pdeclist->value);
      if(pdeclist->next)
        fprintf( output, "%sdec_list[%d].next      = &(dec_list[%d]);\n\n", INDENT[2], i, i+1);
      else
        fprintf( output, "%sdec_list[%d].next      = NULL;\n\n", INDENT[2], i);
      i++;
    }
  }
  declist_num = i;

  fprintf( output, " \n");

  //Instruction Structure
  fprintf( output, "%sinstructions = (ac_dec_instr*) malloc(sizeof(ac_dec_instr)*AC_DEC_INSTR_NUMBER);\n", INDENT[2]);
  i = 0;
  count_fields = 0;
  for( pinstr = instr_list; pinstr!= NULL; pinstr=pinstr->next){
    fprintf( output, "%sinstructions[%d].name      = \"%s\";\n", INDENT[2], i, pinstr->name);
    fprintf( output, "%sinstructions[%d].mnemonic  = \"%s\";\n", INDENT[2], i, pinstr->mnemonic);
    fprintf( output, "%sinstructions[%d].asm_str   = \"%s\";\n", INDENT[2], i, pinstr->asm_str);
    fprintf( output, "%sinstructions[%d].format    = \"%s\";\n", INDENT[2], i, pinstr->format);
    fprintf( output, "%sinstructions[%d].id        = %d;\n", INDENT[2], i, pinstr->id);
    if(HaveMultiCycleIns)
      fprintf( output, "%sinstructions[%d].cycles    = %d;\n", INDENT[2], i, pinstr->cycles);

    fprintf( output, "%sinstructions[%d].dec_list  = &(dec_list[%d]);\n", INDENT[2], i, count_fields);
    if(pinstr->next)
      fprintf( output, "%sinstructions[%d].next      = &(instructions[%d]);\n\n", INDENT[2], i, i+1);
    else
      fprintf( output, "%sinstructions[%d].next      = NULL;\n\n", INDENT[2], i);

    for( pdeclist = pinstr->dec_list; pdeclist!= NULL; pdeclist=pdeclist->next)
      count_fields++;
    i++;
  }

}



/**************************************/
/*! Emits a method to update pipe regs
  Used by CreateArchImpl function     */
/***************************************/
void EmitPipeUpdateMethod( FILE *output){
  extern ac_stg_list *stage_list;
  extern char *project_name;
  ac_stg_list *pstage;
  extern ac_sto_list *storage_list;
  ac_sto_list *pstorage;

  //Emiting Update Method.
  COMMENT(INDENT[0],"Updating Pipe Regs for behavioral simulation.");
  fprintf( output, "%svoid %s_arch::ac_update_regs(){\n", INDENT[0], project_name);
  fprintf( output, "%sstatic ac_instr nop;\n\n", INDENT[1]);

  for( pstage = stage_list; pstage->next != NULL; pstage=pstage->next){

    fprintf( output, "%sif( !%s_stall )\n", INDENT[1], pstage->name);

    fprintf( output, "%sif( %s_flush ){\n", INDENT[2], pstage->name);
    fprintf( output, "%s%s_regin.write( nop );\n", INDENT[3], pstage->next->name);
    fprintf( output, "%s%s_flush = 0;\n", INDENT[3], pstage->name);
    fprintf( output, "%s}\n", INDENT[2]);

    fprintf( output, "%selse\n", INDENT[2]);
    fprintf( output, "%s%s_regin.write( %s_regout.read() );\n", INDENT[3], pstage->next->name, pstage->name);

    fprintf( output, "%selse\n", INDENT[1]);
    fprintf( output, "%s%s_stall = 0;\n", INDENT[2], pstage->name);
    fprintf( output, "\n");

  }

  if( ACDelayFlag ){
    for( pstorage = storage_list; pstorage!= NULL; pstorage = pstorage->next )
      //TODO: Support Delayed assignment for formatted regs
      if( pstorage->format == NULL )
        fprintf( output, "%s%s.commit_delays( sc_simulation_time() );\n", INDENT[1], pstorage->name);

    fprintf( output, "%sac_pc.commit_delays( sc_simulation_time() );\n", INDENT[1]);
  }

  fprintf( output, "%sbhv_pc = ac_pc;\n", INDENT[1]);

  fprintf( output, "%s}\n", INDENT[0]);
}

/**************************************/
/*! Emits a method to update pipe regs
  Used by CreateArchImpl function     */
/***************************************/
void EmitMultiPipeUpdateMethod( FILE *output){

  extern ac_pipe_list *pipe_list;
  extern char *project_name;
  ac_stg_list *pstage;
  ac_pipe_list *ppipe;
  extern ac_sto_list *storage_list;
  ac_sto_list *pstorage;

  //Emiting Update Method.
  COMMENT(INDENT[0],"Updating Pipe Regs for behavioral simulation.");
  fprintf( output, "%svoid %s_arch::ac_update_regs(){\n", INDENT[0], project_name);
  fprintf( output, "%sstatic ac_instr nop;\n\n", INDENT[1]);

  for ( ppipe = pipe_list; ppipe != NULL; ppipe=ppipe->next ){

    for( pstage = ppipe->stages; pstage->next != NULL; pstage=pstage->next){

      fprintf( output, "%sif( !%s_%s_stall )\n", INDENT[1], ppipe->name, pstage->name);

      fprintf( output, "%sif( %s_%s_flush ){\n", INDENT[2], ppipe->name, pstage->name);
      fprintf( output, "%s%s_%s_regin.write( nop );\n", INDENT[3], ppipe->name, pstage->next->name);
      fprintf( output, "%s%s_%s_flush = 0;\n", INDENT[3], ppipe->name, pstage->name);
      fprintf( output, "%s}\n", INDENT[2]);
			
      fprintf( output, "%selse\n", INDENT[2]);
      fprintf( output, "%s%s_%s_regin.write( %s_%s_regout.read() );\n", INDENT[3], ppipe->name, pstage->next->name, ppipe->name, pstage->name);
			
      fprintf( output, "%selse\n", INDENT[1]);
      fprintf( output, "%s%s_%s_stall = 0;\n", INDENT[2], ppipe->name, pstage->name);
      fprintf( output, "\n");
			
    }
  }

  if( ACDelayFlag ){
    for( pstorage = storage_list; pstorage!= NULL; pstorage = pstorage->next )
      //TODO: Support Delayed assignment for formatted regs
      if( pstorage->format == NULL )
        fprintf( output, "%s%s.commit_delays( sc_simulation_time() );\n", INDENT[1], pstorage->name);
		
    fprintf( output, "%sac_pc.commit_delays( sc_simulation_time() );\n", INDENT[1]);
  }
	
  fprintf( output, "%sbhv_pc = ac_pc;\n", INDENT[1]);
	
  fprintf( output, "%s}\n", INDENT[0]);
}

/**************************************/
/*!  Emits a method to update pipe regs
  \brief Used by CreateArchImpl function      */
/***************************************/
void EmitUpdateMethod( FILE *output){

  extern char *project_name;
  extern int HaveMultiCycleIns, HaveMemHier;
  extern ac_sto_list *storage_list;
  ac_sto_list *pstorage;

  //Emiting Update Method.
  COMMENT(INDENT[0],"Updating Regs for behavioral simulation.");
//   fprintf( output, "%svoid %s::ac_update_regs(){\n\n", INDENT[0], project_name);
//
//   fprintf(output, "%sfor (;;) {\n\n", INDENT[1]);

  fprintf( output, "%sif(!ac_wait_sig){\n", INDENT[1]);
  if( ACDelayFlag ){
    for( pstorage = storage_list; pstorage!= NULL; pstorage = pstorage->next )
      fprintf( output, "%s%s.commit_delays( (double)ac_cycle_counter );\n", INDENT[2], pstorage->name);
    fprintf( output, "%sac_pc.commit_delays(  (double)ac_cycle_counter );\n", INDENT[2]);

    fprintf( output, "%sif(!ac_parallel_sig)\n", INDENT[2]);
    fprintf( output, "%sac_cycle_counter+=1;\n", INDENT[3]);
    fprintf( output, "%selse\n", INDENT[2]);
    fprintf( output, "%sac_parallel_sig = 0;\n\n", INDENT[3]);

  }

  fprintf( output, "%sbhv_pc = ac_pc;\n", INDENT[2]);
  if( HaveMultiCycleIns)
    fprintf( output, "%sbhv_cycle.write( ac_cycle );\n", INDENT[2]);
  fprintf( output, "%s}\n", INDENT[1]);
  /*   fprintf( output, "%selse{\n", INDENT[1]); */
  /*   fprintf( output, "%sdo_it = do_it.read()^1;\n", INDENT[2]); */
  /*   fprintf( output, "%s}\n", INDENT[1]); */

  if( HaveMemHier ){
    for( pstorage = storage_list; pstorage!= NULL; pstorage = pstorage->next ) {
      if( pstorage->type == CACHE || pstorage->type == ICACHE || pstorage->type == DCACHE || pstorage->type == MEM )
        fprintf( output, "%s%s.process_request( );\n", INDENT[1], pstorage->name);
    }
  }
  fprintf(output, "%sif (ac_stop_flag) {\n", INDENT[1]);
  fprintf( output, "%sreturn;\n", INDENT[2]);
  fprintf( output, "%s}\n", INDENT[1]);

  if (ACWaitFlag) {
    fprintf( output, "%selse {\n", INDENT[1]);

    fprintf( output, "%sif (instr_in_batch < instr_batch_size) {\n", INDENT[2]);
    fprintf( output, "%sinstr_in_batch++;\n", INDENT[3]);
    fprintf( output, "%s}\n", INDENT[2]);

    fprintf( output, "%selse {\n", INDENT[2]);
    fprintf( output, "%sinstr_in_batch = 0;\n", INDENT[3]);
    fprintf( output, "%swait(1, SC_NS);\n", INDENT[3]);
    fprintf( output, "%s}\n", INDENT[2]);

    fprintf(output, "%s}\n\n", INDENT[1]);
  }

  fprintf( output, "%s} // for (;;)\n", INDENT[0]);
  fprintf( output, "%s} // behavior()\n\n", INDENT[0]);
}

/**************************************/
/*!  Emits the if statement that handles instruction decodification
  \brief Used by EmitProcessorBhv, EmitMultCycleProcessorBhv and CreateStgImpl functions      */
/***************************************/
void EmitDecodification( FILE *output, int base_indent){

  extern int wordsize, fetchsize, HaveMemHier;
  extern char* project_name;

  base_indent++;
  if( HaveMemHier ){

    if (fetchsize == wordsize)
      fprintf( output, "%s*((ac_fetch*)(fetch)) = IM->read( decode_pc );\n\n", INDENT[base_indent]);
    else if (fetchsize == wordsize/2)
      fprintf( output, "%s*((ac_fetch*)(fetch)) = IM->read_half( decode_pc );\n\n", INDENT[base_indent]);
    else if (fetchsize == 8)
      fprintf( output, "%s*((ac_fetch*)(fetch)) = IM->read_byte( decode_pc );\n\n", INDENT[base_indent]);
    else {
      AC_ERROR("Fetchsize differs from wordsize or (wordsize/2) or 8: not implemented.");
      exit(EXIT_FAILURE);
    }

    fprintf( output, "%sif( ac_wait_sig ) {\n", INDENT[base_indent]);
    fprintf( output, "%sreturn;\n", INDENT[base_indent+1]);
    fprintf( output, "%s}\n", INDENT[base_indent]);
  }

  if( ACDecCacheFlag ){
    fprintf( output, "%sins_cache = (DEC_CACHE+decode_pc);\n", INDENT[base_indent]);
    fprintf( output, "%sif ( !ins_cache->valid ){\n", INDENT[base_indent]);
  }

  if( !HaveMemHier ){

    /*     if (fetchsize == wordsize) */
    /*       fprintf( output, "%s*((ac_fetch*)(fetch)) = IM->read( decode_pc );\n\n", INDENT[base_indent+1]); */
    /*     else if (fetchsize == wordsize/2) */
    /*       fprintf( output, "%s*((ac_fetch*)(fetch)) = IM->read_half( decode_pc );\n\n", INDENT[base_indent+1]); */
    /*     else if (fetchsize == 8) */
    /*       fprintf( output, "%s*((ac_fetch*)(fetch)) = IM->read_byte( decode_pc );\n\n", INDENT[base_indent+1]); */
    /*     else { */
    /*       AC_ERROR("Fetchsize differs from wordsize or (wordsize/2) or 8: not implemented."); */
    /*       exit(EXIT_FAILURE); */
    /*     } */
  }

    /*   fprintf( output, "%squant = AC_FETCHSIZE/8;\n", INDENT[base_indent+1]); */
  fprintf( output, "%squant = 0;\n", INDENT[base_indent+1]);

    //The Decoder uses a big endian bit stream. So if the host is little endian, convert it!
    /*   if( ac_host_endian == 0 ){ */
    /*     fprintf( output, "%sfor (i=0; i< AC_FETCHSIZE/8; i++) {\n", INDENT[base_indent+1]); */
    /*     fprintf( output, "%sbuffer[quant - 1 - i] = fetch[i];\n", INDENT[base_indent+2]); */
    /*     fprintf( output, "%s}\n", INDENT[base_indent+1]); */
    /*   } */

  if( ACDecCacheFlag ){
    fprintf( output, "%sins_cache->instr_p = new ac_instr<%s_parms::AC_DEC_FIELD_NUMBER>((ISA.decoder)->Decode(reinterpret_cast<unsigned char*>(buffer), quant));\n", INDENT[base_indent+1], project_name);
    fprintf( output, "%sins_cache->valid = 1;\n", INDENT[base_indent+1]);
    fprintf( output, "%s}\n", INDENT[base_indent]);
    fprintf( output, "%sinstr_vec = ins_cache->instr_p;\n", INDENT[base_indent]);
  }
  else{
    fprintf( output, "%sinstr_dec = (ISA.decoder)->Decode(reinterpret_cast<unsigned char*>(buffer), quant);\n", INDENT[base_indent]);
    fprintf( output, "%sinstr_vec = new ac_instr<%s_parms::AC_DEC_FIELD_NUMBER>( instr_dec);\n", INDENT[base_indent], project_name);
  }

  //Checking if it is a valid instruction
  fprintf( output, "%sins_id = instr_vec->get(IDENT);\n\n", INDENT[base_indent]);
  fprintf( output, "%sif( ins_id == 0 ) {\n", INDENT[base_indent]);
  fprintf( output, "%scerr << \"ArchC Error: Unidentified instruction. \" << endl;\n", INDENT[base_indent+1]);
  fprintf( output, "%scerr << \"PC = \" << hex << decode_pc << dec << endl;\n", INDENT[base_indent+1]);
  fprintf( output, "%sstop();\n", INDENT[base_indent+1]);
  fprintf( output, "%sreturn;\n", INDENT[base_indent+1]);
  fprintf( output, "%s}\n", INDENT[base_indent]);

  fprintf( output, "\n");

}

/**************************************/
/*!  Emit code for executing instructions
  \brief Used by EmitProcessorBhv, EmitMultCycleProcessorBhv and CreateStgImpl functions      */
/***************************************/
void EmitInstrExec( FILE *output, int base_indent){
  extern ac_stg_list *stage_list;
  extern ac_pipe_list *pipe_list;
  extern ac_dec_instr *instr_list;
  extern ac_dec_format *format_ins_list;
  extern ac_dec_field *common_instr_field_list;
  extern int HaveCycleRange;

  extern char* project_name;

  ac_dec_format *pformat;
  ac_dec_instr *pinstr;
  ac_dec_field *pfield, *pf;

  if( ACGDBIntegrationFlag )
    fprintf( output, "%sif (gdbstub && gdbstub->stop(decode_pc)) gdbstub->process_bp();\n\n", INDENT[base_indent]);

  fprintf( output, "%sac_pc = decode_pc;\n\n", INDENT[base_indent]);

  fprintf(output, "%sISA.cur_instr_id = ins_id;\n", INDENT[base_indent]);
  fprintf(output, "%sif (!ac_annul_sig) ", INDENT[base_indent]);

  //Pipelined archs can annul an instruction through pipelining flushing.
  if(stage_list || pipe_list ){
    fprintf( output, "ISA._behavior_instruction( (ac_stage_list) id );\n");
/*     fprintf( output, "%s(ISA.*(%s_parms::%s_isa::instr_table[ins_id].ac_instr_type_behavior))((ac_stage_list) id);\n", INDENT[base_indent], project_name, project_name); */
/*     fprintf( output, "%s(ISA.*(%s_parms::%s_isa::instr_table[ins_id].ac_instr_behavior))((ac_stage_list) id);\n", INDENT[base_indent], project_name, project_name); */
  }
  else{
    fprintf(output, "ISA._behavior_instruction(");
    /* common_instr_field_list has the list of fields for the generic instruction. */
    for( pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next){
      fprintf(output, "instr_vec->get(%d)", pfield->id);
      if (pfield->next != NULL)
        fprintf(output, ", ");
    }
    fprintf(output, ");\n");

    /*     fprintf( output, "%sif(!ac_annul_sig) (ISA.*(%s_parms::%s_isa::instr_table[ins_id].ac_instr_type_behavior))();\n", INDENT[base_indent], project_name, project_name); */
    /*     fprintf( output, "%sif(!ac_annul_sig) (ISA.*(%s_parms::%s_isa::instr_table[ins_id].ac_instr_behavior))();\n", INDENT[base_indent], project_name, project_name); */
  }

  /* Switch statement for instruction selection */
  fprintf(output, "%sswitch (ins_id) {\n", INDENT[base_indent]);
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    /* opens case statement */
    fprintf(output, "%scase %d: // Instruction %s\n", INDENT[base_indent], pinstr->id, pinstr->name);
    /* emits format behavior method call */
    for (pformat = format_ins_list;
         (pformat != NULL) && strcmp(pinstr->format, pformat->name);
         pformat = pformat->next);
    fprintf(output, "%sif (!ac_annul_sig) ISA._behavior_%s_%s(", INDENT[base_indent + 1],
            project_name, pformat->name);
    for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
      fprintf(output, "instr_vec->get(%d)", pfield->id);
      if (pfield->next != NULL)
        fprintf(output, ", ");
    }
    fprintf(output, ");\n");
    /* emits instruction behavior method call */
    fprintf(output, "%sif (!ac_annul_sig) ISA.behavior_%s(", INDENT[base_indent + 1],
            pinstr->name);
    for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
      fprintf(output, "instr_vec->get(%d)", pfield->id);
      if (pfield->next != NULL)
        fprintf(output, ", ");
    }
    fprintf(output, ");\n");
    fprintf(output, "%sbreak;\n", INDENT[base_indent + 1]);
  }
  fprintf(output, "%s} // switch (ins_id)\n", INDENT[base_indent]);

  if( ACStatsFlag ){
    fprintf( output, "%sif((!ac_annul_sig) && (!ac_wait_sig)) {\n", INDENT[base_indent]);
    fprintf( output, "%sISA.stats[%s_stat_ids::INSTRUCTIONS]++;\n", INDENT[base_indent+1], project_name);
    fprintf( output, "%s(*(ISA.instr_stats[ins_id]))[%s_instr_stat_ids::COUNT]++;\n", INDENT[base_indent+1], project_name);

    //If cycle range for instructions were declared, include them on the statistics.
/*    if( HaveCycleRange ){
      fprintf( output, "%sac_sim_stats.ac_min_cycle_count += instr->get_min_latency();\n", INDENT[base_indent+1]);
      fprintf( output, "%sac_sim_stats.ac_max_cycle_count += instr->get_max_latency();\n", INDENT[base_indent+1]);
    }*/

    fprintf( output, "%s}\n", INDENT[base_indent]);
  }

  if( ACDebugFlag ){
    fprintf( output, "%sif( ac_do_trace != 0 ) \n", INDENT[base_indent]);
    fprintf( output, PRINT_TRACE, INDENT[base_indent+1]);
  }

  if( stage_list || pipe_list )
    fprintf( output, "%sregout.write( *instr_vec);\n", INDENT[base_indent]);

  if(!ACDecCacheFlag){
    fprintf( output, "%sdelete instr_vec;\n", INDENT[base_indent]);
    //    fprintf( output, "%sfree(instr_dec);\n", INDENT[base_indent]);
  }
  //  fprintf( output, "%s}\n", INDENT[base_indent-1]);

}


/**************************************/
/*!  Emits the if statement executed before
  fetches are performed.
  \brief Used by EmitProcessorBhv functions and CreateStgImpl      */
/***************************************/
void EmitFetchInit( FILE *output, int base_indent){
  extern int HaveMultiCycleIns;

  fprintf(output, "%sbhv_pc = ac_pc;\n", INDENT[base_indent]);

  if (!ACDecCacheFlag){
    fprintf( output, "%sif( bhv_pc >= APP_MEM->get_size()){\n", INDENT[base_indent]);
  }
  else
    fprintf( output, "%sif( bhv_pc >= dec_cache_size){\n", INDENT[base_indent]);

  fprintf( output, "%scerr << \"ArchC: Address out of bounds (pc=0x\" << hex << bhv_pc << \").\" << endl;\n", INDENT[base_indent+1]);
	//  fprintf( output, "%scout = cerr;\n", INDENT[base_indent+1]);

  if( ACVerifyFlag ){
    fprintf( output, "%send_log.mtype = 1;\n", INDENT[base_indent+1]);
    fprintf( output, "%send_log.log.time = -1;\n", INDENT[base_indent+1]);
    fprintf( output, "%sif(msgsnd(msqid, (struct log_msgbuf *)&end_log, sizeof(end_log), 0) == -1)\n", INDENT[base_indent+1]);
    fprintf( output, "%sperror(\"msgsnd\");\n", INDENT[base_indent+2]);
  }
/*   fprintf( output, "%sac_stop();\n", INDENT[base_indent+1]); */
  fprintf( output, "%sstop();\n", INDENT[base_indent+1]);
  fprintf( output, "%sreturn;\n", INDENT[base_indent+1]);
  fprintf( output, "%s}\n", INDENT[base_indent]);

  fprintf( output, "%selse {\n", INDENT[base_indent]);

  fprintf( output, "%sif( start_up ){\n", INDENT[base_indent+1]);
  fprintf( output, "%sdecode_pc = ac_pc;\n", INDENT[base_indent+2]);
  if(ACABIFlag)
    fprintf( output, "%sISA.syscall.set_prog_args(argc, argv);\n", INDENT[3]);
  fprintf( output, "%sstart_up=0;\n", INDENT[base_indent+2]);
  if( ACDecCacheFlag )
    fprintf( output, "%sinit_dec_cache();\n", INDENT[base_indent+2]);
  fprintf( output, "%s}\n", INDENT[base_indent+1]);

  fprintf( output, "%selse{ \n", INDENT[base_indent+1]);
  if(HaveMultiCycleIns && !ACDecCacheFlag ){
    //fprintf( output, "%sfree(instr_dec);\n", INDENT[base_indent+2]);
    fprintf( output, "%sdelete(instr_vec);\n", INDENT[base_indent+2]);
  }
  fprintf( output, "%sdecode_pc = bhv_pc;\n", INDENT[base_indent+2]);
  fprintf( output, "%s}\n \n", INDENT[base_indent+1]);

}

/**************************************/
/*!  Emits the body of a processor implementation for
  a processor without pipeline and with single cycle instruction.
  \brief Used by CreateProcessorImpl function      */
/***************************************/
void EmitProcessorBhv( FILE *output){

  fprintf(output, "%sfor (;;) {\n\n", INDENT[1]);

  EmitFetchInit(output, 1);
  EmitDecodification(output, 2);
  EmitInstrExec(output, 2);

  fprintf( output, "%sif ((!ac_wait_sig) && (!ac_annul_sig)) ac_instr_counter+=1;\n", INDENT[2]);
  fprintf( output, "%sac_annul_sig = 0;\n", INDENT[2]);
  if (ACVerboseFlag || ACVerifyFlag || ACVerifyTimedFlag)
    fprintf( output, "%sbhv_done.write(1);\n", INDENT[2]);
  fprintf( output, "%s}\n", INDENT[1]);

//   fprintf(output, "%swait();\n\n", INDENT[1]);
//
//   fprintf( output, "%s}\n\n", INDENT[1]);
//
//   fprintf( output, "}\n\n");

}

/**************************************/
/*!  Emits the body of a processor implementation for
  a processor without pipeline, with single cycle instruction and
  with an ABI provided.
  \brief Used by CreateProcessorImpl function      */
/***************************************/
void EmitProcessorBhv_ABI( FILE *output){

  fprintf(output, "%sfor (;;) {\n\n", INDENT[1]);

  EmitFetchInit(output, 1);

  //Emiting system calls handler.
  COMMENT(INDENT[2],"Handling System calls.")
    fprintf( output, "%sswitch( decode_pc ){\n\n", INDENT[2]);

  EmitABIDefine(output);
  fprintf( output, "\n\n");
  EmitABIAddrList(output,2);

  fprintf( output, "%sdefault:\n\n", INDENT[2]);

  EmitDecodification(output, 2);
  EmitInstrExec(output, 3);

  //Closing default case.
  fprintf( output, "%sbreak;\n", INDENT[3]);

  //Closing switch.
  fprintf( output, "%s}\n", INDENT[2]);

  fprintf( output, "%sif ((!ac_wait_sig) && (!ac_annul_sig)) ac_instr_counter+=1;\n", INDENT[2]);
  fprintf( output, "%sac_annul_sig = 0;\n", INDENT[2]);
  if (ACVerboseFlag || ACVerifyFlag || ACVerifyTimedFlag)
    fprintf( output, "%sdone.write(1);\n", INDENT[2]);

  //Closing for.
  fprintf( output, "%s}\n", INDENT[1]);

//   fprintf(output, "%swait();\n\n", INDENT[1]);
//
//   fprintf( output, "%s}\n\n", INDENT[1]);
//
//   fprintf( output, "}\n\n");

}


/**************************************/
/*!  Emits the body of a processor implementation for
  a multicycle processor
  \brief Used by CreateProcessorImpl function      */
/***************************************/
void EmitMultiCycleProcessorBhv( FILE *output){


  fprintf( output, "%sif( ac_cycle == 1 ){\n\n", INDENT[1]);

  EmitFetchInit(output, 2);
  EmitDecodification(output, 3);

  //Multicycle execution demands a different control. Do not use EmitInstrExec.
  fprintf( output, "%sac_pc = decode_pc;\n", INDENT[3]);
  fprintf( output, "%sac_cycle = 1;\n", INDENT[3]);
  fprintf( output, "%sinstr = (ac_instruction *)ISA.instr_table[ins_id][1];\n", INDENT[3]);
  fprintf( output, "%sformat = (ac_instruction *)ISA.instr_table[ins_id][2];\n", INDENT[3]);
  fprintf( output, "%sformat->set_fields( *instr_vec  );\n", INDENT[3]);
  fprintf( output, "%sinstr->set_fields( *instr_vec  );\n", INDENT[3]);

  if( ACDebugFlag ){
    fprintf( output, "%sif( ac_do_trace != 0 ) \n", INDENT[3]);
    fprintf( output, PRINT_TRACE, INDENT[4]);
  }

  if( ACStatsFlag ){
    fprintf( output, "%sac_sim_stats.instr_executed++;;\n", INDENT[3]);
    fprintf( output, "%sac_sim_stats.instr_table[ins_id].count++;\n", INDENT[3]);
    fprintf( output, "%sac_sim_stats.ac_min_cycle_count += instr->get_min_latency();\n", INDENT[3]);
    fprintf( output, "%sac_sim_stats.ac_max_cycle_count += instr->get_max_latency();\n", INDENT[3]);
  }

  fprintf( output, "%sISA.instruction.set_cycles( instr->get_cycles());\n", INDENT[3]);
  fprintf( output, "%sISA.instruction.set_size( instr->get_size());\n", INDENT[3]);

  fprintf( output, "%s}\n", INDENT[2]);
  fprintf( output, "%sac_instr_counter+= 1;\n", INDENT[2]);

  fprintf( output, "%s}\n", INDENT[1]);

  fprintf( output, "%sISA.instruction.behavior( (ac_stage_list)0, ac_cycle );\n", INDENT[1]);
  fprintf( output, "%sformat->behavior((ac_stage_list)0, ac_cycle );\n", INDENT[1]);
  fprintf( output, "%sinstr->behavior((ac_stage_list)0, ac_cycle );\n", INDENT[1]);

  fprintf( output, "%sif( ac_cycle > instr->get_cycles())\n", INDENT[1]);
  fprintf( output, "%sac_cycle=1;\n\n", INDENT[2]);

  fprintf( output, "%sbhv_done.write(1);\n", INDENT[1]);
  fprintf( output, "}\n\n");
}

/**************************************/
/*!  Emits the define that implements the ABI control
  for pipelined architectures
  \brief Used by CreateStgImpl function      */
/***************************************/
void EmitPipeABIDefine( FILE *output){

  fprintf( output, "%s#define AC_SYSC(NAME,LOCATION) \\\n", INDENT[0]);
  fprintf( output, "%scase LOCATION: \\\n", INDENT[2]);

  fprintf( output, "%sif (flushes_left) { \\\n", INDENT[3]);
  fprintf( output, "%sfflush(0);\\\n", INDENT[4]);
  fprintf( output, "%sflushes_left--;\\\n", INDENT[4]);
  fprintf( output, "%s} \\\n", INDENT[3]);

  fprintf( output, "%selse { \\\n", INDENT[3]);
  fprintf( output, "%sfflush(0);\\\n", INDENT[4]);

  if( ACDebugFlag ){
    fprintf( output, "%sif( ac_do_trace != 0 )\\\n", INDENT[4]);
    fprintf( output, "%strace_file << hex << decode_pc << dec << endl; \\\n", INDENT[5]);
  }

  fprintf( output, "%sISA.syscall.NAME(); \\\n", INDENT[4]);
  fprintf( output, "%sac_instr_counter++; \\\n", INDENT[4]);
  fprintf( output, "%sflushes_left = 7; \\\n", INDENT[4]);
  fprintf( output, "%s} \\\n", INDENT[3]);

  fprintf( output, "%sregout.write( *the_nop); \\\n", INDENT[3]);
  fprintf( output, "%sif (! (ac_pc.read() %% 2)) ac_pc.write(ac_pc.read() + 1); \\\n", INDENT[3]);
  fprintf( output, "%selse ac_pc.write(ac_pc.read() - 1); \\\n", INDENT[3]);
  fprintf( output, "%sbreak;  \\\n", INDENT[3]);

}

/**************************************/
/*!  Emits the define that implements the ABI control
  for non-pipelined architectures
  \brief Used by CreateStgImpl function      */
/***************************************/
void EmitABIDefine( FILE *output){
  extern char* project_name;

  fprintf( output, "%s#define AC_SYSC(NAME,LOCATION) \\\n", INDENT[0]);
  fprintf( output, "%scase LOCATION: \\\n", INDENT[2]);

  if( ACStatsFlag ){
    fprintf( output, "%sISA.stats[%s_stat_ids::SYSCALLS]++; \\\n", INDENT[4],
	project_name);
  }

  if( ACDebugFlag ){
    fprintf( output, "%sif( ac_do_trace != 0 )\\\n", INDENT[4]);
    fprintf( output, "%strace_file << hex << decode_pc << dec << endl; \\\n", INDENT[5]);
  }

  fprintf( output, "%sISA.syscall.NAME(); \\\n", INDENT[4]);
  fprintf( output, "%sbreak;  \\\n", INDENT[3]);
}


/**************************************/
/*!  Emits the ABI special address list
  to be used inside the ABI switchs
  \brief Used by CreateStgImpl function      */
/***************************************/
void EmitABIAddrList( FILE *output, int base_indent){

  fprintf( output, "#include <ac_syscall.def>\n", INDENT[base_indent]);
  fprintf( output, "\n");
  fprintf( output, "#undef AC_SYSC\n\n");
}

/**************************************/
/*!  Emits a ac_cache instantiation.
  \brief Used by CreateResourcesImpl function      */
/***************************************/
void EmitCacheDeclaration( FILE *output, ac_sto_list* pstorage, int base_indent){

  //Parameter 1 will be the pstorage->name string
  //Parameters passed to the ac_cache constructor. They are not exactly in the same
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

  for( pparms = pstorage->parms; pparms != NULL; pparms = pparms->next ){

    switch( i ){

    case 1: /* First parameter must be a valid associativity */

      if( !pparms->str ){
        AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
        printf("The first parameter must be a valid associativity: \"dm\", \"2w\", \"4w\", ... \n");
        exit(1);
      }

#ifdef DEBUG_STORAGE
      printf("CacheDeclaration: Processing parameter: %d, which is: %s\n", i, pparms->str);
#endif

      if( !strcmp(pparms->str, "dm") || !strcmp(pparms->str, "DM") ){ //It is a direct-mapped cache
        is_dm = 1;
        sprintf( parm4, "1");  //Set size will be the 4th parameter for ac_cache constructor.
        sprintf( parm5, "DEFAULT");  //DM caches do not need a replacement strategy, use a default value in this parameter.
      }
      else if( !strcmp(pparms->str, "fully") || !strcmp(pparms->str, "FULLY") ){ //It is a fully associative cache
        is_fully =1;
      }
      else{  //It is a n-way cache
        aux = strchr( pparms->str,'w');
        if(  !aux ){   // Checking if the string has a 'w'
          AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
          printf("The first parameter must be a valid associativity: \"dm\", \"2w\", \"4w\", ..., \"fully\" \n");
          exit(1);
        }
        aux = (char*) malloc( strlen(pparms->str) );
        strncpy(aux, pparms->str,  strlen(pparms->str)-1);
        aux[ strlen(pparms->str)-1]='\0';

        sprintf(parm4, "%s", aux);  //Set size will be the 4th parameter for ac_cache constructor.
        free(aux);
      }
      break;

    case 2: /* Second parameter is the number of blocks (lines) */

      if( !(pparms->value > 0 ) ){
        AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
        printf("The second parameter must be a valid (>0) number of blocks (lines).\n");
        exit(1);
      }

      if( is_fully )
        sprintf( parm4, "%d", pparms->value);  //Set size will be the number of blocks (lines) for fully associative caches

      sprintf(parm3, "%d", pparms->value);
      break;

    case 3: /* Third parameter is the block (line) size */

      if( !(pparms->value > 0 ) ){
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

      if( is_dm ){ //This value is set when the first parameter is being processed.
        /* So this is a write-policy */
        if( !strcmp( pparms->str, "wt") || !strcmp( pparms->str, "WT") ){
          //One will tell that a wt cache was declared
          wp = WRITE_THROUGH;
        }
        else if( !strcmp( pparms->str, "wb") || !strcmp( pparms->str, "WB") ) {
          //Zero will tell that a wb cache was declared
          wp = WRITE_BACK;
        }
        else{
          AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
          printf("For direct-mapped caches, the fourth parameter must be a valid write policy: \"wt\" or \"wb\".\n");
          exit(1);
        }
      }
      else{
        /* So, this is a replacement strategy */
        if( !strcmp( pparms->str, "lru") || !strcmp( pparms->str, "LRU") ){
          sprintf( parm5, "LRU");  //Including parameter
        }
        else if( !strcmp( pparms->str, "random") || !strcmp( pparms->str, "RANDOM") ) {
          sprintf( parm5, "RANDOM");  //Including parameter
        }
        else{
          AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
          printf("For non-direct-mapped caches, the fourth parameter must be a valid replacement strategy: \"lru\" or \"random\".\n");
          exit(1);
        }
      }
      break;

    case 5: /* The fifth parameter is a write policy. */

      if( !is_dm ){ //This value is set when the first parameter is being processed.

        if( !strcmp( pparms->str, "wt") || !strcmp( pparms->str, "WT") ){

          wp = WRITE_THROUGH;
        }
        else if( !strcmp( pparms->str, "wb") || !strcmp( pparms->str, "WB") ) {

          wp = WRITE_BACK;
        }
        else{
          AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
          printf("For non-direct-mapped caches, the fourth parameter must be a valid replacement strategy: \"lru\" or \"random\".\n");
          exit(1);
        }
      }

      else{ //This value is "war" for write-around or "wal" for "write-allocate"

        if( !strcmp( pparms->str, "war") || !strcmp( pparms->str, "WAR") ){
          wp = wp | WRITE_AROUND;
        }
        else if( !strcmp( pparms->str, "wal") || !strcmp( pparms->str, "WAL") ) {
          wp = wp | WRITE_ALLOCATE;
        }
        else{
          AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
          printf("For non-direct-mapped caches, the fourth parameter must be a valid replacement strategy: \"lru\" or \"random\".\n");
          exit(1);
        }

      }
      break;

    case 6: /* The sixth parameter, if it is present, is a write policy.
               It must not be present for direct-mapped caches.*/

      if( !is_dm ){ //This value is set when the first parameter is being processed.

        if( !strcmp( pparms->str, "war") || !strcmp( pparms->str, "WAR") ){
          wp = wp | WRITE_AROUND;
        }
        else if( !strcmp( pparms->str, "wal") || !strcmp( pparms->str, "WAL") ) {
          wp = wp | WRITE_ALLOCATE;
        }
        else{
          AC_ERROR("Invalid parameter in cache declaration: %s\n", pstorage->name);
          printf("For non-direct-mapped caches, the fifth parameter must be  \"war\" or \"wal\".\n");
          exit(1);
        }

      }
      else{
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

  //Printing cache declaration.
  fprintf( output, "%sac_cache ac_resources::%s(\"%s\", %s, %s, %s, %s, 0x%x);\n",
           INDENT[base_indent], pstorage->name, pstorage->name, parm2, parm3, parm4, parm5, wp);
}

////////////////////////////////////
// Utility Functions              //
////////////////////////////////////

//!Read the archc.conf configuration file
void ReadConfFile(){

  char *user_homedir;
  char *conf_filename_local;
  char *conf_filename_global;
//  extern char *ARCHC_PATH;
  extern char *SYSTEMC_PATH;
  extern char *TLM_PATH;
  extern char *CC_PATH;
  extern char *OPT_FLAGS;
  extern char *DEBUG_FLAGS;
  extern char *OTHER_FLAGS;

  FILE *conf_file;
  char line[CONF_MAX_LINE];
  char var[CONF_MAX_LINE];
  char value[CONF_MAX_LINE];

/*  ARCHC_PATH = getenv("ARCHC_PATH"); */

/*  if(!ARCHC_PATH){ */
/*    AC_ERROR("You should set the ARCHC_PATH environment variable.\n"); */
/*    exit(1); */
/*  } */

  user_homedir = getenv("HOME");
  conf_filename_local = malloc(strlen(user_homedir) + 19);
  strcpy(conf_filename_local, user_homedir);
  strcat(conf_filename_local, "/.archc/archc.conf");

  conf_filename_global = malloc(strlen(SYSCONFDIR) + 12);
  strcpy(conf_filename_global, SYSCONFDIR);
  strcat(conf_filename_global, "/archc.conf");

  conf_file = fopen(conf_filename_local, "r");

  if (!conf_file)
    conf_file = fopen(conf_filename_global, "r");

  if( !conf_file ){
    //ERROR
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

        if( !strcmp(var, "TLM_PATH") ){
          TLM_PATH =  (char*) malloc(strlen(value)+1);
          TLM_PATH = strcpy(TLM_PATH, value);
        }
        if( !strcmp(var, "SYSTEMC_PATH") ){
          SYSTEMC_PATH = (char*) malloc(strlen(value)+1);
          SYSTEMC_PATH = strcpy(SYSTEMC_PATH, value);
          if (strlen(value) <= 2) {
            AC_ERROR("Please configure a SystemC path running install.sh script in ArchC directory.\n");
            exit(1);
          }
        }
        else if( !strcmp(var, "CC") ){
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
        else if( !strcmp(var, "TARGET_ARCH") ){
          TARGET_ARCH = (char*) malloc(strlen(value)+1);
          TARGET_ARCH = strcpy(TARGET_ARCH, value);
        }

      }
    }
  }

  fclose(conf_file);

  free(conf_filename_local);
  free(conf_filename_global);
}
