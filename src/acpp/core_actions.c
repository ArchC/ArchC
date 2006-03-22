/* ex: set tabstop=2 expandtab:
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/

#include "core_actions.h"

static ac_dec_instr *instr_list_tail;
static ac_sto_list *storage_list_tail;
static ac_pipe_list *pipe_list_tail;
static ac_cache_parms *parms_list_tail;
static ac_dec_field *field_list;     
static ac_cache_parms *parms_list;


static int parse_format(char **fieldstr, int sum_size, int size_limit, ac_dec_field **field_list_head, ac_dec_field **field_list_tail, char *error_msg);



ac_dec_format *format_ins_list;    
ac_dec_format *format_ins_list_tail;
ac_dec_format *format_reg_list;    
ac_dec_format *format_reg_list_tail;
ac_dec_instr *instr_list;          
ac_pipe_list *pipe_list;           
ac_stg_list  *stage_list;          
ac_sto_list  *storage_list;        

int HaveFormattedRegs, HaveMultiCycleIns, HaveMemHier, HaveCycleRange;
int ControlInstrInfoLevel;
int HaveTLMPorts;
int HaveTLMIntrPorts;

int instr_num;     
int declist_num;  
int format_num;   
int const_count;  
int stage_num;    
int pipe_num;     
int reg_width;   
int largest_format_size;

ac_sto_list *fetch_device; 






void init_core_actions() 
{
  /* module variables */
  instr_list_tail = NULL;
  storage_list_tail = NULL;
  pipe_list_tail = NULL;
  parms_list_tail = NULL;
  field_list = NULL;
  parms_list = NULL;

  /* interface variables */
  format_ins_list = NULL; 
  format_ins_list_tail = NULL;
  format_reg_list = NULL; 
  format_reg_list_tail = NULL;
  instr_list = NULL;
  pipe_list = NULL;
  stage_list = NULL;
  storage_list = NULL;

  HaveFormattedRegs = 0; 
  HaveMultiCycleIns = 0;
  HaveMemHier = 0;
  HaveCycleRange = 0;
  ControlInstrInfoLevel = 0;
  HaveTLMPorts = 0;
  HaveTLMIntrPorts = 0;   

  instr_num = 0;     
  declist_num = 0;  
  format_num = 0;   
  const_count = 0;  
  stage_num = 0;    
  pipe_num = 0;     
  reg_width = 0;  
  largest_format_size = 0;

  fetch_device = NULL;
}


ac_dec_instr *find_instr(char *name)
{
  ac_dec_instr *pinstr = instr_list;
  
  while (pinstr != NULL) {
    if (!strcmp(pinstr->name, name)) break;
    pinstr = pinstr->next;
  }
  
  return pinstr;
}

ac_dec_format *find_format(char *name)
{
  ac_dec_format *pformat = format_ins_list;

  while (pformat != NULL) {
    if (!strcmp(pformat->name, name)) break;
    pformat = pformat->next;
  }

  return pformat;
}

ac_sto_list *find_storage(char *name)
{
  ac_sto_list *pstorage = storage_list;

  while (pstorage != NULL) {
    if (!strcmp(pstorage->name, name)) break;
    pstorage = pstorage->next;
  }

  return pstorage;
}

ac_dec_field *find_field(ac_dec_format *pformat, char *name)
{
  ac_dec_field *pfield = pformat->fields;
  
  while (pfield != NULL) {
    if (!strcmp(pfield->name, name)) break;

    pfield = pfield->next;
  }
  return pfield;
}


/***************************************/
/*!Add format to instr/reg format lists.
  \param head Head of the format list.
  \param tail Tail of the format list.
  \param name The name of the instruction to be added.
  \param str  String containg field declarations */
/***************************************/
int add_format( ac_dec_format **head, ac_dec_format **tail, char *name, char* str, char *error_msg)
{
  ac_dec_field *field_list_tail = field_list;
  ac_dec_format *pformat;
  int sum_size = parse_format(&str, 0, -1, &field_list, &field_list_tail, error_msg);

  if (sum_size == -1) return 0;
  
  //Create new format
  pformat = (ac_dec_format*) malloc( sizeof(ac_dec_format));
  pformat->name  = name;
  pformat->size  = sum_size;
  pformat->fields = field_list;
  pformat->next = NULL;

  //Clear field_list pointer
  field_list = 0;

  //Keeping track of the largest format
  if( sum_size >largest_format_size ) 
    largest_format_size = sum_size;

  //Put new format in the formats list
  if( (*tail) ){
    (*tail)->next = pformat;
    (*tail) = pformat;
  }
  else  { /*  First format being added to the list */
    (*tail) = (*head) = pformat;
  }

  return 1;
}



/***************************************/
/*!Add instruction to instruction list.
  \param name The name of the instruction to be added. */
/***************************************/
int add_instr(char* name, char *typestr, ac_dec_instr **pinstr, char *error_msg) 
{
  ac_dec_instr *ppins;

  (*pinstr) = (ac_dec_instr*) malloc( sizeof(ac_dec_instr));
  (*pinstr)->name = (char*) malloc( strlen(name)+1);
  (*pinstr)->format = (char*) malloc( strlen(typestr)+1);
  strcpy((*pinstr)->name, name);
  strcpy((*pinstr)->format, typestr);
  (*pinstr)->mnemonic = NULL;
  (*pinstr)->asm_str = NULL;
  instr_num++;
  (*pinstr)->id = instr_num;
  (*pinstr)->cycles = 1;
  (*pinstr)->min_latency = 1;
  (*pinstr)->max_latency = 1;
  (*pinstr)->dec_list = NULL;
  (*pinstr)->cflow = NULL;
  (*pinstr)->next = NULL;

  if ( instr_list_tail ) {
    ppins = find_instr(name);
    if (ppins) {
      sprintf(error_msg, "Duplicated instruction name: %s", name);
      return 0;
    }
    instr_list_tail->next = *pinstr;
    instr_list_tail = *pinstr;
  }
  else  { /*  First instruction being added to the list */
    instr_list_tail = instr_list = *pinstr;
  }
  return 1;
}


/**************************************/
/*! Add a pipeline to pipe list.
  \param name The name of the pipeline to be added. */
/***************************************/
ac_pipe_list *add_pipe(char* name)
{
  ac_pipe_list *ppipe;
   
  pipe_num++;
  ppipe = (ac_pipe_list*) malloc( sizeof(ac_pipe_list));
  ppipe->name = (char*) malloc( strlen(name)+1);
  strcpy(ppipe->name, name);  
  ppipe->id = pipe_num;
  ppipe->stages = NULL;
  ppipe->next = NULL;

  if( pipe_list_tail ){
     pipe_list_tail->next = ppipe;
     pipe_list_tail = ppipe;
  }
  else  { /*  First pipe being added to the list */
     pipe_list_tail = pipe_list = ppipe;
  }

  return ppipe;
}



/**************************************/
/*! Add stage to stage list.
  \param name The name of the stage to be added. */
/***************************************/
void add_stage( char* name, ac_stg_list** listp )
{
  ac_stg_list *list, *pstage;  

  stage_num++;
  pstage = (ac_stg_list*) malloc( sizeof(ac_stg_list));
  pstage->name = (char*) malloc( strlen(name)+1);
  strcpy(pstage->name, name); 
  pstage->id = stage_num;
  pstage->next = NULL;

  list = *listp;
  /* Inserting at the tail */
  if( list ){

     /* finding list tail */
     for(; list->next != NULL; list= list->next ){}

     /* Adding stage */
     list->next = pstage;
  }
  else  { /*  First stage being added to the list */
     *listp = pstage;
  }

}



/**************************************/
/*! Add a new device to the storage list.
  \param name The name of the device to be added. 
  \param size The size of the device to be added.
  \param type The type of the device to be added. */
/***************************************/
int add_storage( char* name, unsigned size, ac_sto_types type, char *typestr, char *error_msg)
{
  ac_sto_list *pstorage;
   
  pstorage = (ac_sto_list*) malloc( sizeof(ac_sto_list));
  pstorage->name = (char*) malloc( strlen(name)+1);
  strcpy(pstorage->name,name);
  if (type == REG && typestr != NULL) {
    pstorage->format = (char*) malloc( strlen(typestr)+1);
    strcpy(pstorage->format, typestr);
  }
  else
    pstorage->format = NULL;
    
  pstorage->size = size;


  /* In this case we may have a cache declaration with a parameter list or a generic cache*/
  if( (type == CACHE || type == ICACHE || type == DCACHE)  ){

    if( parms_list ){
      /* Plugging paramenters for ac_cache instantiation. */
      pstorage->parms = parms_list;
      /* Reseting list for future declarations */
      parms_list = parms_list_tail = NULL;
    }
    else{ /* Generic cache object */
      if (pstorage->size==0){        
        sprintf(error_msg, "Invalid size in cache declaration: %s", name);
        return 0;
      }
      else
        pstorage->parms = NULL;
    }   
  }

  pstorage->type = type;
  pstorage->next = NULL;
  pstorage->higher = NULL;
  pstorage->level = 0;

  if(  type == ICACHE  ){
    fetch_device = pstorage;
  }

  //Checking if the user declared a specific register width
  if(  ((type == REGBANK) || (type == REG)) && reg_width != 0  ){
    pstorage->width = reg_width;
  }

  if( storage_list_tail ){
     storage_list_tail->next = pstorage;
     storage_list_tail = pstorage;
  }
  else  { /*  First device being added to the list */
     storage_list_tail = storage_list = pstorage;
  }  

  return 1;
}



/**************************************/
/*! Add a new field to the end of the decoding sequence list.
  \param pinstr Pointer to the current instruction.
  \param name   The name of the field to be added.
  \param value  The value of the field to be added. */
/***************************************/
int add_dec_list( ac_dec_instr *pinstr, char* name, int value, char *error_msg)
{
  ac_dec_list *pdec_list;
  ac_dec_format *pformat;
  ac_dec_field *pfield;

  /* Find format */
  pformat = find_format(pinstr->format);
  if( pformat == NULL ) {     
    sprintf(error_msg, "Instr %s: Format not found for this instruction", pinstr->name);
    return 0;
  }

  /* Find field */
  pfield = find_field(pformat, name);
  if( pfield == NULL ) {
    sprintf(error_msg, "Instr %s: Field '%s' not found in type %s", pinstr->name, name, pformat->name);
    return 0;
  }

  /* Check if value fits in field size */
  if (value > ((1<<pfield->size)-1)) {
    sprintf(error_msg, "Instr %s: Field '%s' value %d too big", pinstr->name, name, value);
    value = ((1<<pfield->size)-1);
    return 0;
  }

  /* Create new decoding list node */
  pdec_list = (ac_dec_list*) malloc( sizeof(ac_dec_list));
  pdec_list->name = (char*) malloc( strlen(name)+1);
  strcpy(pdec_list->name, name);
  pdec_list->value = value;
  pdec_list->next = NULL;

  if( pinstr->dec_list == NULL )
    pinstr->dec_list = pdec_list;
  else{
    ac_dec_list *aux;
    for(aux =pinstr->dec_list; aux->next != NULL; aux = aux->next);
    aux->next = pdec_list;      
  }

  return 1;
}



/**************************************/
/*! Get the existing structure for control flow instructions or create one.
  \param pinstr Pointer to the current instruction. */
/***************************************/
ac_control_flow *get_control_flow_struct(ac_dec_instr *pinstr)
{
  if (!pinstr->cflow)
    pinstr->cflow = calloc(sizeof(ac_control_flow),1);
  return pinstr->cflow;
}



void add_parms(char *name, int value)
{
  ac_cache_parms *pparms;

  pparms = (ac_cache_parms*) malloc( sizeof(ac_cache_parms));
  pparms->value = value;  
  if (name != NULL) {
    pparms->str = (char*) malloc(strlen(name)+1); 
    strcpy(pparms->str, name);
  }
  else
    pparms->str = NULL;

  pparms->next = NULL;
  
  if ( parms_list == NULL )
    parms_list = parms_list_tail= pparms;
  else {
    parms_list_tail->next = pparms;
    parms_list_tail = pparms;
  }         
}






   
/********************************************************/
/*!Parse format string generating field list.
  \param fieldstr Reference to string containing field declarations.
  \param sum_size Sum of bits of previous fields.
  \param size_limit Limit of size for this field group
  \param field_list_head Head of the field list.
  \param field_list_tail Tail of the field list.        */
/********************************************************/
static int parse_format(char **fieldstr, int sum_size, int size_limit, ac_dec_field **field_list_head, ac_dec_field **field_list_tail, char *error_msg)
{
  char *str = *fieldstr;
  int first_bit = sum_size;
  char *fieldend;
  
  //Eat spaces
  for (; **fieldstr && isspace(**fieldstr); (*fieldstr)++);

  /* Building field_list */
  while (**fieldstr != 0) {

    //Start group?
    if (**fieldstr == '[') {
      (*fieldstr)++;
      sum_size = parse_format(fieldstr, sum_size, -1, field_list_head, field_list_tail, error_msg);
      if (sum_size == -1) return -1;
    }

    //Close group?
    else if (**fieldstr == ']') {
      (*fieldstr)++;
      if ((size_limit != -1) && (size_limit != sum_size)) {
        sprintf(error_msg, "Size mismatch in format choices for \"%s\"", str);
        return -1; // -1 = error
      }
      return sum_size;
    }

    //New choice?
    else if (**fieldstr == '|') {
      (*fieldstr)++;
      if (size_limit != -1) {
        if (size_limit != sum_size) {
          sprintf(error_msg, "Size mismatch in format choices for \"%s\"", str);
          return -1;
        }
      }
      else {
        size_limit = sum_size;
      }
      sum_size = first_bit;
    }

    //This must be a kind of field
    else {
      int value;
      ac_dec_field *pfield = (ac_dec_field*) malloc( sizeof(ac_dec_field));

      //Is it a constant number?
      value == strtol(*fieldstr, &fieldend, 0);
      if (fieldend != *fieldstr) {
        //Set name
        pfield->name = (char*) calloc(10,sizeof(char));
        snprintf( pfield->name, 10, "CONST_%d", const_count);
        const_count++;
        //Set value
        pfield->val = value;
      }

      //It is a regular field
      else {
        if (**fieldstr == '%') (*fieldstr)++;
        for (fieldend = *fieldstr; *fieldend && *fieldend!=':'; fieldend++);
        //Set name
        pfield->name = (char*) calloc((fieldend-(*fieldstr)+1),sizeof(char));
        strncpy(pfield->name, *fieldstr, (fieldend-(*fieldstr)));
        //Set value
        pfield->val = 0;
      }
      
      //Set size
      if (*fieldend != ':') {
        sprintf(error_msg, "Size not given in \"%s\"", *fieldstr);
        return -1;
      }
      *fieldstr = fieldend+1;
      value = strtol(*fieldstr, &fieldend, 0);
      if (fieldend == *fieldstr) {
        sprintf(error_msg, "Invalid size in \"%s\", character %d", str, (*fieldstr)-str);        
        return -1;
      }
      pfield->size = value;
      //Set sign
      pfield->sign=0;
      *fieldstr = fieldend;
      if (**fieldstr == ':') {
        (*fieldstr)++;
        if ((**fieldstr == 's') || (**fieldstr == 'S'))
          pfield->sign=1;
        else if ((**fieldstr != 'u') && (**fieldstr != 'U')) {          
          sprintf(error_msg, "Invalid sign specification in \"%s\", character %d", str, (*fieldstr)-str);
          return -1;
        }
        (*fieldstr)++;
      }
      //Set first bit
      sum_size += pfield->size;
      pfield->first_bit = sum_size-1;
      //Set id
      pfield->id = 0;
    
      //Put the new field in the list
      pfield->next = NULL;
      if(*field_list_head){
        (*field_list_tail)->next = pfield;
        (*field_list_tail) = pfield;
      }
      else{
        *field_list_tail = *field_list_head = pfield;
      }
    }
    
    //Eat spaces
    for (; **fieldstr && isspace(**fieldstr); (*fieldstr)++);
  }
    
  return sum_size;
}
