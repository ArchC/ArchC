#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <tuple>
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

static char *debuginfo_path;

static  Dwfl_Callbacks offline_callbacks; 

static std::map<Dwarf_Addr,std::tuple<int,int,int> > address_2_line_info_cache; //1st = lineNum, 2nd = colNum, 3rd = source_file_name_index */
static std::vector<std::string> source_file_names_cache;


static std::string hltrace_file_name;
static FILE* hltrace_file = NULL;


static Dwfl *dwfl = NULL;

void pre_process_lines_info()
{
  extern char* appfilename;
  extern const char *project_name;

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

  if(dwfl == NULL)
  {
    dwfl = dwfl_begin (&offline_callbacks);
    dwfl_report_offline (dwfl, "", appfilename, -1); 
  }
  

  Dwarf_Addr bias = 0;
  Dwarf_Die *lastcu = NULL;
  do 
  {
    lastcu = dwfl_nextcu (dwfl, lastcu, &bias);

    if(lastcu)
    {
      size_t numb_lines = 0;
      dwfl_getsrclines (lastcu, &numb_lines);
      for (int r = 0; r < numb_lines; ++r)
      {
        Dwfl_Line* myline =  dwfl_onesrcline (lastcu, r);

        

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

        address_2_line_info_cache[curaddr] = std::tuple<int,int,int>(linep,colp,source_file_names_cache_index);

      }

    }


  } while(lastcu != NULL);

  for (int i = 0; i < source_file_names_cache.size(); ++i)
  {
    fprintf(hltrace_file, "%s\n", source_file_names_cache[i].c_str() );
    
  }
  fprintf(hltrace_file, "---\n" );

}


void generate_trace_for_address(unsigned long long int addr)
{


  if(address_2_line_info_cache.empty())
  {
    pre_process_lines_info();

  }
  static int last_trace_line = -1;
  static int last_trace_file_index = -1;

  if(hltrace_file)
  {
    
    int new_trace_line;
    int new_trace_file_index;



    std::map<Dwarf_Addr,std::tuple<int,int,int> >::iterator it_num = address_2_line_info_cache.find(addr);
    if(it_num != address_2_line_info_cache.end())
    {
      std::tuple<int,int,int> line_info = it_num->second;
      new_trace_line = std::get<0>(line_info);
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

      address_2_line_info_cache[curaddr] = std::tuple<int,int,int>(linep,colp,source_file_names_cache_index);
      new_trace_line = linep;
      new_trace_file_index = source_file_names_cache_index;
    }



    if((new_trace_line != last_trace_line || new_trace_file_index != last_trace_file_index) && new_trace_line != -1 && new_trace_file_index != -1)
    {
      if(new_trace_file_index != last_trace_file_index)
      {
        last_trace_file_index = new_trace_file_index;
        fprintf(hltrace_file, "f_%d\n",new_trace_file_index);
      }
      last_trace_line = new_trace_line;
      fprintf(hltrace_file, "%d\n", new_trace_line);
    }
  }

}


