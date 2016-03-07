#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <utility>
#include <string.h>
#include <string>
#include <iostream>
#include "ac_hltrace.H"
#include <hashtable.h>
#include <sstream>
#include <cassert>
/* #include <elfutils/libdwfl.h> */
/* #include <elfutils/libdw.h> */

static char *debuginfo_path;

static  Dwfl_Callbacks offline_callbacks; 

static std::map<Dwarf_Addr,std::tuple<int,int,int> > address_2_line_info_cache; //1st = lineNum, 2nd = colNum, 3rd = source_file_name_index */
static std::vector<std::string> source_file_names_cache;
/* static std::map<std::string */ 


static std::string hltrace_file_name;
static FILE* hltrace_file = NULL;


/* static const char * */
/* get_diename (Dwarf_Die *die) */
/* { */
/*   Dwarf_Attribute attr; */
/*   const char *name; */

/*   name = dwarf_formstring (dwarf_attr_integrate (die, DW_AT_MIPS_linkage_name, */
/* 						 &attr) */
/* 			   ?: dwarf_attr_integrate (die, DW_AT_linkage_name, */
/* 						    &attr)); */

/*   if (name == NULL) */
/*     name = dwarf_diename (die) ?: "??"; */

/*   return name; */
/* } */

/* static bool */
/* get_function_name (Dwarf_Die *cudie, Dwarf_Addr addr, std::string function_name) */
/* { */
/*   Dwarf_Addr bias = 0; */
/*   /1* Dwarf_Die *cudie = dwfl_module_addrdie (mod, addr, &bias); *1/ */

/*   Dwarf_Die *scopes; */
/*   int nscopes = dwarf_getscopes (cudie, addr - bias, &scopes); */
/*   if (nscopes <= 0) */
/*     return false; */

/*   bool res = false; */
/*   for (int i = 0; i < nscopes; ++i) */
/*     switch (dwarf_tag (&scopes[i])) */
/*     { */
/*       case DW_TAG_subprogram: */
/*         { */
/*           const char *name = get_diename (&scopes[i]); */
/*           if (name == NULL) */
/*             goto done; */
/*           fprintf (stderr,"addr: %d %s\n",addr, name); */
/*           res = true; */
/*           goto done; */
/*         } */

/*       case DW_TAG_inlined_subroutine: */
/*         { */
/*           const char *name = get_diename (&scopes[i]); */
/*           if (name == NULL) */
/*             goto done; */

/*           /1* When using --pretty-print we only show inlines on their */
/*              own line.  Just print the first subroutine name.  *1/ */
/*           fprintf (stderr,"addr inline %d %s\n",addr, name); */
/*           res = true; */
/*           goto done; */

/*           /1* printf (" in "); *1/ */
/*           /1* continue; *1/ */
/*         } */
/*     } */

/* done: */
/*   free (scopes); */
/*   return res; */
/* } */







static Dwfl *dwfl = NULL;

void pre_process_lines_info()
{
  extern char* appfilename;
  /* char* appfilename = "./qsort_small" ; */
  /* char* appfilename = "./bitcnts" ; */
  extern const char *project_name;
  /* const char* project_name = "project_name"; */


  if (hltrace_file_name.empty())
  {
    std::string appNameString (appfilename);
    std::string projectNameString (project_name);
    hltrace_file_name = projectNameString + "_" + appNameString.substr(appNameString.find_last_of("\\/") + 1) + ".hltrace";
  }

  if(hltrace_file == NULL)
  {
    hltrace_file  = fopen(hltrace_file_name.c_str(),"w+");
  }

  offline_callbacks.find_debuginfo = dwfl_standard_find_debuginfo;
  offline_callbacks.debuginfo_path = &debuginfo_path;
  offline_callbacks.section_address = dwfl_offline_section_address;
  offline_callbacks.find_elf = dwfl_build_id_find_elf;

  /* static Dwfl *dwfl = NULL; */
  if(dwfl == NULL)
  {
    dwfl = dwfl_begin (&offline_callbacks);
    dwfl_report_offline (dwfl, "", appfilename, -1); 
    /* dwfl_report_end (dwfl, NULL, NULL); */
  }
  

  Dwarf_Addr bias = 0;
  Dwarf_Die *lastcu = NULL;
  do 
  {
    lastcu = dwfl_nextcu (dwfl, lastcu, &bias);

    if(lastcu)
    {
      /* std::cout << "mais um cu! " << std::endl; */
      size_t numb_lines = 0;
      dwfl_getsrclines (lastcu, &numb_lines);
      /* std::cout << numb_lines << std::endl; */
      for (int r = 0; r < numb_lines; ++r)
      {
        /* std::cout << " o r eh " << r << std::endl; */
        Dwfl_Line* myline =  dwfl_onesrcline (lastcu, r);
        /* std::cout << myline << std::endl; */

        /* Dwarf_Addr HIghPC = 0; */
        /* dwarf_highpc(myline->die,&HIghPC); */
        

        int linep;
        int colp;
        Dwarf_Word mtime;
        Dwarf_Word length;
        Dwarf_Addr curaddr;
        const char * current_source_file_name =  dwfl_lineinfo (myline, &curaddr, &linep, &colp, &mtime, &length);
        std::vector<std::string>::iterator sourceIt = std::find(source_file_names_cache.begin(),source_file_names_cache.end(),std::string(current_source_file_name));
        int source_file_names_cache_index;
        if(sourceIt != source_file_names_cache.end())
        {
          source_file_names_cache_index = sourceIt - source_file_names_cache.begin();
        }
        else
        {
          source_file_names_cache_index = source_file_names_cache.size();
          source_file_names_cache.push_back(std::string(current_source_file_name));
        }
        /* get_function_name(lastcu,curaddr); */

        address_2_line_info_cache[curaddr] = std::tuple<int,int,int>(linep,colp,source_file_names_cache_index);
        /* std::tuple<int,int,int> test_abc; */
        /* test_abc = address_2_line_info_cache[curaddr]; */
        /* std::cout << "addr: " << curaddr << " line: " << std::get<0>(test_abc) << "col: " << std::get<1>(test_abc) << "source index: "  << std::get<2>(test_abc)<< std::endl; */
        /* printf("%s %d %d\n",retorno_legal,linep,colp); */

      }

    }


  } while(lastcu != NULL);

  for (int i = 0; i < source_file_names_cache.size(); ++i)
  {
    /* std::cout << source_file_names_cache[i] << std::endl; */
    fprintf(hltrace_file, "%s\n", source_file_names_cache[i].c_str() );
    
  }
  fprintf(hltrace_file, "---\n" );

  /* dwfl_report_end (dwfl, NULL, NULL); */
  /* dwfl_validate_address(NULL,0,0); */
}

/* int dwfl_validate_address (Dwfl *dwfl, Dwarf_Addr address) */
/* { */ 
/*   Dwfl_Module *mod = dwfl_addrmodule(dwfl, address); */                                        
/*   if (mod == NULL) */                                                                                                                          
/*     return -1; */                                                                                                                                    

/*   Dwarf_Addr relative = address; */                                                                                                          
/*   int idx = dwfl_module_relocate_address (mod, &relative); */                                                                              
/*   if (idx < 0) */                                                                                                                                       
/*     return -1; */                                                                                                                                       


/*   return 0; */                                                                                                                                           
/* } */   

void generate_trace_for_address(unsigned long long int addr)
{


  if(address_2_line_info_cache.empty())
  {
    pre_process_lines_info();
    /* for(std::map<Dwarf_Addr,std::tuple<int,int,int> >::const_iterator it = address_2_line_info_cache.begin(); */
    /*         it != address_2_line_info_cache.end(); ++it) */
    /* { */
    /*       std::cerr << it->first << " " << std::get<0>(it->second) << " " << std::get<1>(it->second) << "\n"; */
    /* } */

  }
  static int last_trace_line = -1;
  static int last_trace_file_index = -1;
  /* static std::string last_trace_file ; */

  if(hltrace_file)
  {
    
    int new_trace_line;
    int new_trace_file_index;



    /* std::map<Dwarf_Addr,std::tuple<int,int,int> >::iterator it_num = address_2_line_info_cache.lower_bound(addr); */ 
    std::map<Dwarf_Addr,std::tuple<int,int,int> >::iterator it_num = address_2_line_info_cache.find(addr);
    if(it_num != address_2_line_info_cache.end())
    {
      /* std::tuple<int,int,int> line_info = address_2_line_info_cache[addr]; */
      std::tuple<int,int,int> line_info = it_num->second;
      new_trace_line = std::get<0>(line_info);
      /* std::string new_trace_file = source_file_names_cache[std::get<2>(line_info)]; */ 
      new_trace_file_index = std::get<2>(line_info);
    }
    else
    {
      Dwfl_Line* myline = dwfl_getsrc(dwfl,addr);
      if(myline == NULL)
        return;
      int linep;
      int colp;
      Dwarf_Word mtime;
      Dwarf_Word length;
      Dwarf_Addr curaddr;
      const char * current_source_file_name =  dwfl_lineinfo (myline, &curaddr, &linep, &colp, &mtime, &length);
      if(current_source_file_name == NULL)
        return;
      std::vector<std::string>::iterator sourceIt = std::find(source_file_names_cache.begin(),source_file_names_cache.end(),std::string(current_source_file_name));
      int source_file_names_cache_index;
      if(sourceIt != source_file_names_cache.end())
      {
        source_file_names_cache_index = sourceIt - source_file_names_cache.begin();
      }
      else
      {
        source_file_names_cache_index = source_file_names_cache.size();
        source_file_names_cache.push_back(std::string(current_source_file_name));
      }
      /* get_function_name(lastcu,curaddr); */

      address_2_line_info_cache[curaddr] = std::tuple<int,int,int>(linep,colp,source_file_names_cache_index);
      new_trace_line = linep;
      new_trace_file_index = source_file_names_cache_index;
    }



    if((new_trace_line != last_trace_line || new_trace_file_index != last_trace_file_index) && new_trace_line != -1 && new_trace_file_index != -1)
    {
      /* if(new_trace_file.compare(last_trace_file) != 0) */
      if(new_trace_file_index != last_trace_file_index)
      {
        last_trace_file_index = new_trace_file_index;
        fprintf(hltrace_file, "f_%d\n",new_trace_file_index);
      }
      last_trace_line = new_trace_line;
      fprintf(hltrace_file, "%d\n", new_trace_line);
    }
    /* } */
    /* delete lineInfo; */
  }

}



/* int main(int argc, char *argv[]) */
/* { */
/*   /1* generate_trace_for_address(796); *1/ */
/*   generate_trace_for_address(1100000); */
/*   return 0; */
/* } */
