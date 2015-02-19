#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <utility>
#include <string.h>
#include <iostream>
#include "ac_hltrace.H"

struct LineInfo
{
  int colNumber ;
  int lineNumber ;
  std::string sourcePath;
  std::string functionName;
};

static char *debuginfo_path;

static  Dwfl_Callbacks offline_callbacks; 


static const char * get_diename (Dwarf_Die *die)
{
  Dwarf_Attribute attr;
  const char *name;

  name = dwarf_formstring (dwarf_attr_integrate (die, DW_AT_MIPS_linkage_name,
        &attr)
      ?: dwarf_attr_integrate (die, DW_AT_linkage_name,
        &attr));

  if (name == NULL)
    name = dwarf_diename (die) ?: "??";

  return name;
}



static bool print_dwarf_function (Dwfl_Module *mod, Dwarf_Addr addr, LineInfo* lineInfo)
{
  Dwarf_Addr bias = 0;
  Dwarf_Die *cudie = dwfl_module_addrdie (mod, addr, &bias);

  Dwarf_Die *scopes;
  int nscopes = dwarf_getscopes (cudie, addr - bias, &scopes);
  if (nscopes <= 0)
    return false;

  for (int i = 0; i < nscopes; ++i)
    switch (dwarf_tag (&scopes[i]))
    {
      case DW_TAG_subprogram:
        {
          const char *name = get_diename (&scopes[i]);
          if (name == NULL)
            return false;
          //get function name 
          lineInfo->functionName = std::string(name);
          return true;
        }

      case DW_TAG_inlined_subroutine:
        {
          const char *name = get_diename (&scopes[i]);
          if (name == NULL)
            return false;
          //get function name
          lineInfo->functionName = std::string(name);
        }
    }

  return false;
}




static int find_symbol (Dwfl_Module *mod,
    void **userdata __attribute__ ((unused)),
    const char *name __attribute__ ((unused)),
    Dwarf_Addr start __attribute__ ((unused)),
    void *arg)
{
  const char *looking_for = (const char *)((void **) arg)[0];
  GElf_Sym *symbol = (GElf_Sym *)((void **) arg)[1];
  GElf_Addr *value = (GElf_Addr *)((void **) arg)[2];

  int n = dwfl_module_getsymtab (mod);
  for (int i = 1; i < n; ++i)
  {
    const char *symbol_name = dwfl_module_getsym_info (mod, i, symbol,
        value, NULL, NULL,
        NULL);
    if (symbol_name == NULL || symbol_name[0] == '\0')
      continue;
    switch (GELF_ST_TYPE (symbol->st_info))
    {
      case STT_SECTION:
      case STT_FILE:
      case STT_TLS:
        break;
      default:
        if (!strcmp (symbol_name, looking_for))
        {
          ((void **) arg)[0] = NULL;
          return DWARF_CB_ABORT;
        }
    }
  }

  return DWARF_CB_OK;
}

static bool adjust_to_section (const char *name, uintmax_t *addr, Dwfl *dwfl)
{
  /* It was (section)+offset.  This makes sense if there is
     only one module to look in for a section.  */
  Dwfl_Module *mod = NULL;

  int nscn = dwfl_module_relocations (mod);
  for (int i = 0; i < nscn; ++i)
  {
    GElf_Word shndx;
    const char *scn = dwfl_module_relocation_info (mod, i, &shndx);
    if (scn == NULL)
      break;
    if (!strcmp (scn, name))
    {
      /* Found the section.  */
      GElf_Shdr shdr_mem;
      GElf_Addr shdr_bias;
      GElf_Shdr *shdr = gelf_getshdr
        (elf_getscn (dwfl_module_getelf (mod, &shdr_bias), shndx),
         &shdr_mem);
      if (shdr == NULL)
        break;


      *addr += shdr->sh_addr + shdr_bias;
      return true;
    }
  }

  return false;
}

static void print_src (const char *src, int lineno, int linecol, Dwarf_Die *cu, LineInfo* lineInfo)
{
  const char *comp_dir = "";
  const char *comp_dir_sep = "";

  if (src[0] != '/')
  {
    Dwarf_Attribute attr;
    comp_dir = dwarf_formstring (dwarf_attr (cu, DW_AT_comp_dir, &attr));
    if (comp_dir != NULL)
      comp_dir_sep = "/";
  }

  //here we have the source path and the line number/ col number
  lineInfo->lineNumber = lineno;
  lineInfo->colNumber = linecol;
  char completePath[1000];
  sprintf (completePath,"%s%s%s",comp_dir, comp_dir_sep, src);
  lineInfo->sourcePath = std::string (completePath);
}


static int handle_address (const char *string, Dwfl *dwfl, LineInfo *lineInfo )
{
  char *endp;
  uintmax_t addr = strtoumax (string, &endp, 0);

  static std::vector<std::string> sourcePathCache; 
  static std::map<uintmax_t,std::tuple<int,int,int>> address2lineCache;   // tuple: 1st = lineNum, 2nd = colNum, 3rd = sourcePathNum

  auto ita = address2lineCache.find(addr);
  if (ita != address2lineCache.end())
  {
    lineInfo->lineNumber = std::get<0>(ita->second);
    lineInfo->colNumber = std::get<1>(ita->second);
    lineInfo->sourcePath = sourcePathCache[std::get<2>(ita->second)];
    return 0;
    
  }

   

  if (endp == string)
  {
    bool parsed = false;
    int i, j;
    char *name = NULL;
    if (sscanf (string, "(%m[^)])%" PRIiMAX "%n", &name, &addr, &i) == 2
        && string[i] == '\0')
      parsed = adjust_to_section (name, &addr, dwfl);
    switch (sscanf (string, "%m[^-+]%n%" PRIiMAX "%n", &name, &i, &addr, &j))
    {
      default:
        break;
      case 1:
        addr = 0;
        j = i;
      case 2:
        if (string[j] != '\0')
          break;

        GElf_Sym sym;
        GElf_Addr value = 0;
        void *arg[3] = { name, &sym, &value };
        (void) dwfl_getmodules (dwfl, &find_symbol, arg, 0);
        addr += value;
        parsed = true;
        break;
    }

    free (name);
    if (!parsed)
      return 1;
  }

  Dwfl_Module *mod = dwfl_addrmodule (dwfl, addr);

  print_dwarf_function (mod, addr, lineInfo) ;


  Dwfl_Line *line = dwfl_module_getsrc (mod, addr);

  const char *src;
  int lineno, linecol;

  if (line != NULL && (src = dwfl_lineinfo (line, &addr, &lineno, &linecol,
          NULL, NULL)) != NULL)
  {
    print_src (src, lineno, linecol, dwfl_linecu (line), lineInfo);
  }
  else
  {
    //could not find line and column number
    lineInfo->lineNumber = -1;
    lineInfo->colNumber = -1;
  }

  
  auto it_sourcePath = std::find(sourcePathCache.begin(), sourcePathCache.end(), lineInfo->sourcePath);
  int sourcePathIndex;
  if (it_sourcePath != sourcePathCache.end())
  {
    sourcePathIndex = it_sourcePath - sourcePathCache.begin();
  }
  else
  {
    sourcePathIndex = sourcePathCache.size();
    sourcePathCache.push_back(lineInfo->sourcePath);
  }


  address2lineCache.insert(std::pair<uintmax_t,std::tuple<int,int,int>>(addr,std::tuple<int,int,int>(lineInfo->lineNumber,lineInfo->colNumber,sourcePathIndex)));

  return 0;
}


LineInfo* getLineInfoFromAddr(std::string exec_name ,std::string addr)
{
  offline_callbacks.find_debuginfo = dwfl_standard_find_debuginfo;
  offline_callbacks.debuginfo_path = &debuginfo_path;
  offline_callbacks.section_address = dwfl_offline_section_address;
  offline_callbacks.find_elf = dwfl_build_id_find_elf;

    
  //Dwfl *dwfl = NULL;
  //dwfl = dwfl_begin (&offline_callbacks);
  //dwfl_report_offline (dwfl, "", exec_name.c_str(), -1); 
  //dwfl_report_end (dwfl, NULL, NULL);
  
  static Dwfl *dwfl = NULL;
  if (dwfl == NULL)
  {
    dwfl = dwfl_begin (&offline_callbacks);
    dwfl_report_offline (dwfl, "", exec_name.c_str(), -1); 
    dwfl_report_end (dwfl, NULL, NULL);
  }
  

  LineInfo * lineInfo = new LineInfo();

  handle_address (addr.c_str(), dwfl,lineInfo);


  //dwfl_end (dwfl);
  return lineInfo;
}

void generate_trace_for_address(unsigned long long int addr)
{
  extern char* appfilename;
  extern const char *project_name;
  static std::string appNameString (appfilename);
  static std::string projectNameString (project_name);

  static std::string hltrace_file_name = projectNameString + "_" + appNameString + ".hltrace";
  static FILE* hltrace_file  = fopen(hltrace_file_name.c_str(),"w+");
  static int last_trace_line = -1;
  static std::string last_trace_file ("");


  if(hltrace_file)
  {
    LineInfo* lineInfo = getLineInfoFromAddr(appfilename,std::to_string(addr));
    if(lineInfo != NULL)
    {
      int new_trace_line = lineInfo->lineNumber;
      std::string new_trace_file = lineInfo->sourcePath; 

      if((new_trace_line != last_trace_line || new_trace_file.compare(last_trace_file) != 0) && new_trace_file.compare("") != 0 && new_trace_line != -1)
      {
        if(new_trace_file.compare(last_trace_file) != 0)
        {
          last_trace_file = new_trace_file;
          fprintf(hltrace_file, "%s\n",new_trace_file.c_str());
        }
        last_trace_line = new_trace_line;
        fprintf(hltrace_file, "%d\n", new_trace_line);
      }
      delete (lineInfo);
    }
  }

}

