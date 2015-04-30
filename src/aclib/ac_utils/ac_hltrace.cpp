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

#define CACHE_SIZE 300 // size in MB

namespace {

//-------------------------------------------------------------
// Bucket
//-------------------------------------------------------------

template<class K, class V>
struct LRUCacheH4Value
{
	typedef std::pair<const K, LRUCacheH4Value<K, V> > Val;
	
	LRUCacheH4Value()
		: _v(), _older(NULL), _newer(NULL) { }
	
	LRUCacheH4Value(const V & v, Val * older, Val * newer)
		: _v(v), _older(older), _newer(newer) { } 
	
	V _v;
	Val * _older;
	Val * _newer;
};


//-------------------------------------------------------------
// Const Iterator
//-------------------------------------------------------------

template<class K, class V>
class LRUCacheH4ConstIterator
{
public:
	typedef std::pair<const K, LRUCacheH4Value<K, V> > Val;
	typedef LRUCacheH4ConstIterator<K, V> const_iterator;
	typedef Val & reference;
	typedef Val * pointer;
	
	enum DIRECTION {
		MRU_TO_LRU = 0,
		LRU_TO_MRU
	};
	
	LRUCacheH4ConstIterator(const Val * ptr = NULL, DIRECTION dir = MRU_TO_LRU);
	
	const_iterator & operator++();
    const_iterator operator++(int);
	
	bool operator==(const const_iterator & other);
	bool operator!=(const const_iterator & other);
	
	const K & key() const;
	const V & value() const;

private:
	const Val * _ptr;
	DIRECTION _dir;
};


template<class K, class V>
LRUCacheH4ConstIterator<K, V>::LRUCacheH4ConstIterator(
	const LRUCacheH4ConstIterator<K, V>::Val * ptr,
	LRUCacheH4ConstIterator<K, V>::DIRECTION dir)
		: _ptr(ptr), _dir(dir)
{
}


template<class K, class V>
LRUCacheH4ConstIterator<K, V> & LRUCacheH4ConstIterator<K, V>::operator++()
{
	assert(_ptr);
	_ptr = (_dir == LRUCacheH4ConstIterator<K, V>::MRU_TO_LRU ? _ptr->second._older : _ptr->second._newer);
	return *this;
}
	

template<class K, class V>
LRUCacheH4ConstIterator<K, V> LRUCacheH4ConstIterator<K, V>::operator++(int)
{
	const_iterator ret = *this;
	++*this;
	return ret;
}


template<class K, class V>
bool LRUCacheH4ConstIterator<K, V>::operator==(const const_iterator & other)
{
	return _ptr == other._ptr;
}
	

template<class K, class V>
bool LRUCacheH4ConstIterator<K, V>::operator!=(const const_iterator & other)
{
	return _ptr != other._ptr;
}


template<class K, class V>
const K & LRUCacheH4ConstIterator<K, V>::key() const
{
	assert(_ptr);
	return _ptr->first;
}


template<class K, class V>
const V & LRUCacheH4ConstIterator<K, V>::value() const
{
	assert(_ptr); 
	return _ptr->second._v;
}
	
	
} // file scope


namespace plb {

//-------------------------------------------------------------
// LRU Cache
//-------------------------------------------------------------

template<class K, class V>
class LRUCacheH4
{
public:
	typedef LRUCacheH4ConstIterator<K, V> const_iterator;
	
public:
	LRUCacheH4(int maxsize);                    // Pre-condition: maxsize >= 1
	LRUCacheH4(const LRUCacheH4 & other);
	
	V & operator[](const K & key);
	void insert(const K & key, const V & value);
	
	int size() const;
	int maxsize() const;
	bool empty() const;
	
	const_iterator find(const K & key);         // updates the MRU
	const_iterator find(const K & key) const;   // does not update the MRU
	const_iterator mru_begin() const;           // from MRU to LRU
	const_iterator lru_begin() const;           // from LRU to MRU
	const_iterator end() const;
	
	void dump_mru_to_lru(std::ostream & os) const;

private:
	typedef std::pair<const K, LRUCacheH4Value<K, V> > Val;
	typedef __gnu_cxx::hashtable<Val, K, __gnu_cxx::hash<K>, std::_Select1st<Val>, std::equal_to<K> > MAP_TYPE;

private:
	Val * _update_or_insert(const K & key);
	Val * _update(typename MAP_TYPE::iterator it);
	Val * _insert(const K & key);

private:
	MAP_TYPE _map;
	Val * _mru;
	Val	* _lru;
	int _maxsize;
};


// Reserve enough space to avoid resizing later on and thus invalidate iterators
template<class K, class V>
LRUCacheH4<K, V>::LRUCacheH4(int maxsize)
	: _map(maxsize, __gnu_cxx::hash<K>(), std::equal_to<K>()),
	  _mru(NULL),
	  _lru(NULL),
	  _maxsize(maxsize)
{
	if (_maxsize <= 0)
		throw "LRUCacheH4: expecting cache size >= 1";
}


template<class K, class V>
LRUCacheH4<K, V>::LRUCacheH4(const LRUCacheH4<K, V> & other)
	: _map(other._map.bucket_count(), __gnu_cxx::hash<K>(), std::equal_to<K>()),
      _maxsize(other._maxsize),
	  _mru(NULL),
	  _lru(NULL)
{
	for (const_iterator it = other.lru_begin();  it != other.end();  ++it)
		this->insert(it.key(), it.value());
}


template<class K, class V>
V & LRUCacheH4<K, V>::operator[](const K & key)
{
	return _update_or_insert(key)->second._v;
}


template<class K, class V>
void LRUCacheH4<K, V>::insert(const K & key, const V & value)
{
	_update_or_insert(key)->second._v = value;
}


template<class K, class V>
int LRUCacheH4<K, V>::size() const
{
	return _map.size();
}
	
	
template<class K, class V>
int LRUCacheH4<K, V>::maxsize() const 
{
	return _maxsize;
}


template<class K, class V>
bool LRUCacheH4<K, V>::empty() const
{
	return size() > 0;
}


// updates MRU
template<class K, class V>
typename LRUCacheH4<K, V>::const_iterator LRUCacheH4<K, V>::find(const K & key)
{
	typename MAP_TYPE::iterator it = _map.find(key);

	if (it != _map.end())
		return const_iterator(_update(it), const_iterator::MRU_TO_LRU);
	else
		return end();
}


// does not update MRU
template<class K, class V>
typename LRUCacheH4<K, V>::const_iterator LRUCacheH4<K, V>::find(const K & key) const
{
	typename MAP_TYPE::iterator it = _map.find(key);
	
	if (it != _map.end())
		return const_iterator(&*it, const_iterator::MRU_TO_LRU);
	else
		return end();
}
	

template<class K, class V>
void LRUCacheH4<K, V>::dump_mru_to_lru(std::ostream & os) const
{
	os << "LRUCacheH4(" << size() << "/" << maxsize() << "): MRU --> LRU: " << std::endl;
	for (const_iterator it = mru_begin();  it != end();  ++it)
		os << it.key() << ": " << it.value() << std::endl;
}


template<class K, class V>
typename LRUCacheH4<K, V>::const_iterator LRUCacheH4<K, V>::mru_begin() const
{
	return const_iterator(_mru, const_iterator::MRU_TO_LRU);
}


template<class K, class V>
typename LRUCacheH4<K, V>::const_iterator LRUCacheH4<K, V>::lru_begin() const
{
	return const_iterator(_lru, const_iterator::LRU_TO_MRU);
}


template<class K, class V>
typename LRUCacheH4<K, V>::const_iterator LRUCacheH4<K, V>::end() const
{
	return const_iterator();
}


template<class K, class V>
typename LRUCacheH4<K, V>::Val * LRUCacheH4<K, V>::_update_or_insert(const K & key)
{
	typename MAP_TYPE::iterator it = _map.find(key);
	if (it != _map.end())
		return _update(it);
	else
		return _insert(key);
}


template<class K, class V>
typename LRUCacheH4<K, V>::Val * LRUCacheH4<K, V>::_update(typename MAP_TYPE::iterator it)
{
	LRUCacheH4Value<K, V> & v = it->second;
	Val * older = v._older;
	Val * newer = v._newer;
	Val * moved = &*it;
	
	// possibly update the LRU
	if (moved == _lru && _lru->second._newer)
		_lru = _lru->second._newer;
	
	if (moved != _mru) {
		// "remove" key from current position
		if (older)
			older->second._newer = newer;
		if (newer)
			newer->second._older = older;
		
		// "insert" key to MRU position
		v._older = _mru;
		v._newer = NULL;
		_mru->second._newer = moved;
		_mru = moved;
	}
	
	return moved;
}


template<class K, class V>
typename LRUCacheH4<K, V>::Val * LRUCacheH4<K, V>::_insert(const K & key)
{
	// if we have grown too large, remove LRU
	if (_map.size() >= _maxsize) {
		Val * old_lru = _lru;
		if (_lru->second._newer) {
			_lru = _lru->second._newer;
			_lru->second._older = NULL;
		}
		_map.erase(old_lru->first);
	}
	
	// insert key to MRU position
	std::pair<typename MAP_TYPE::iterator, bool> ret
		= _map.insert_unique(Val(key, LRUCacheH4Value<K, V>(V(), _mru, NULL)));
	Val * inserted = &*ret.first;
	if (_mru)
		_mru->second._newer = inserted;
	_mru = inserted;
	
	// possibly update the LRU
	if (!_lru)
		_lru = _mru;
	else if (!_lru->second._newer)
		_lru->second._newer = _mru;
	
	return inserted;
}


}  // namespace plb

typedef plb::LRUCacheH4<uintmax_t,std::tuple<int,int,int,int>>lru_cache; // tuple: 1st = lineNum, 2nd = colNum, 3rd = sourcePathNum, 4fh = functionNameNum

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
  {
    return false;
  }

  for (int i = 0; i < nscopes; ++i)
    switch (dwarf_tag (&scopes[i]))
    {
      case DW_TAG_subprogram:
        {
          const char *name = get_diename (&scopes[i]);
          if (name == NULL)
          {
            if(nscopes > 0)
            {
              free(scopes);
            }
            return false;
          }
          //get function name 
          lineInfo->functionName = std::string(name);
          if(nscopes > 0)
          {
            free(scopes);
          }
          return true;
        }

      case DW_TAG_inlined_subroutine:
        {
          const char *name = get_diename (&scopes[i]);
          if (name == NULL)
          {
            if(nscopes > 0)
            {
              free(scopes);
            }
            return false;
          }
          //get function name
          lineInfo->functionName = std::string(name);
        }
    }
  
  if(nscopes > 0)
  {
    free(scopes);
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
  lineInfo->sourcePath = std::string(comp_dir) + std::string(comp_dir_sep) + std::string(src);
}


static int handle_address (const char *string, Dwfl *dwfl, LineInfo *lineInfo )
{
  char *endp;
  uintmax_t addr = strtoumax (string, &endp, 0);

  static std::vector<std::string> functionNameCache; 
  static std::vector<std::string> sourcePathCache; 
  static lru_cache address2lineCache(CACHE_SIZE * 100000);   // tuple: 1st = lineNum, 2nd = colNum, 3rd = sourcePathNum, 4fh = functionNameNum

  auto ita = address2lineCache.find(addr);
  if (ita != address2lineCache.end())
  {
    lineInfo->lineNumber = std::get<0>(ita.value());
    lineInfo->colNumber = std::get<1>(ita.value());
    lineInfo->sourcePath = sourcePathCache[std::get<2>(ita.value())];
    lineInfo->functionName = functionNameCache[std::get<3>(ita.value())];
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
//  print_dwarf_function (mod, addr, lineInfo) ;
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

  auto it_functionName = std::find(functionNameCache.begin(), functionNameCache.end(), lineInfo->functionName);
  int functionNameIndex;
  if (it_functionName != functionNameCache.end())
  {
    functionNameIndex = it_functionName - functionNameCache.begin();
  }
  else
  {
    functionNameIndex = functionNameCache.size();
    functionNameCache.push_back(lineInfo->functionName);
  }

  address2lineCache[addr] = std::tuple<int,int,int,int>(lineInfo->lineNumber,lineInfo->colNumber,sourcePathIndex,functionNameIndex);


  return 0;
}


LineInfo* getLineInfoFromAddr(std::string exec_name ,std::string addr)
{
  offline_callbacks.find_debuginfo = dwfl_standard_find_debuginfo;
  offline_callbacks.debuginfo_path = &debuginfo_path;
  offline_callbacks.section_address = dwfl_offline_section_address;
  offline_callbacks.find_elf = dwfl_build_id_find_elf;

  static Dwfl *dwfl = NULL;
  if(dwfl == NULL)
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

  static std::string hltrace_file_name;

  if (hltrace_file_name.empty())
  {
    std::string appNameString (appfilename);
    std::string projectNameString (project_name);
    hltrace_file_name = projectNameString + "_" + appNameString.substr(appNameString.find_last_of("\\/") + 1) + ".hltrace";
  }

  static FILE* hltrace_file = NULL;
  if(hltrace_file == NULL)
  {
    hltrace_file  = fopen(hltrace_file_name.c_str(),"w+");
  }
  static int last_trace_line = -1;
  static std::string last_trace_file ;

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
    }
    delete lineInfo;
  }

}

