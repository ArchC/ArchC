/* @file      ac_memport.H
 * @author    Thiago Massariolli Sigrist
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Thu Sep 22 11:47:36 2005
 *
 * @brief     Defines a wrapper template class for memory access.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 * 
 */

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_MEMPORT_H_
#define _AC_MEMPORT_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <stdint.h>
#include <list>
#include <fstream>

#if defined(__linux__) || defined(__CYGWIN__)
#include <endian.h>

#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#define __BYTE_ORDER    BYTE_ORDER
#define __BIG_ENDIAN    BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#define __PDP_ENDIAN    PDP_ENDIAN

#elif defined(__OpenBSD__)
#include <sys/endian.h>

#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
#include <sys/endian.h>
#define be16toh(x) betoh16(x)
#define le16toh(x) letoh16(x)
#define be32toh(x) betoh32(x)
#define le32toh(x) letoh32(x)
#define be64toh(x) betoh64(x)
#define le64toh(x) letoh64(x)

#else
#error platform not supported
#endif

// SystemC includes

// ArchC includes
#include "ac_inout_if.H"
#include "ac_log.H"
#include "ac_arch_ref.H"
#include "ac_utils.H"
//////////////////////////////////////////////////////////////////////////////

// 'using' statements
using std::string;
using std::list;
using std::istringstream;
using std::ifstream;

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// Template wrapper class for memory access.
template<typename ac_word, typename ac_Hword> class ac_memport :
  public ac_arch_ref<ac_word, ac_Hword> {

private:

  ac_inout_if* storage;

  ac_word aux_word;
  ac_Hword aux_Hword;
  uint8_t aux_byte;

  ac_ptr buf;
  int bytesPerBlock;

  sc_core::sc_time time_info;
  unsigned int procId;

 // Byte Swap functions
  inline uint16_t byte_swap(uint16_t value) {
  #ifdef AC_GUEST_BIG_ENDIAN
   return be16toh(value);
  #else
   return le16toh(value);
  #endif
  }

  inline uint32_t byte_swap(uint32_t value) {
  #ifdef AC_GUEST_BIG_ENDIAN
    return be32toh(value);
  #else
    return le32toh(value);
  #endif
  }

  inline uint64_t byte_swap(uint64_t value) {
  #ifdef AC_GUEST_BIG_ENDIAN
    return be64toh(value);
  #else
    return le64toh(value);
  #endif
  }

  template <typename T>
  inline T byte_swap(T value) {
  #if AC_MATCH_ENDIANNESS
    return value;
  #else 
    unsigned int aux;
    T result = 0;
    for (unsigned int i = 0; i < sizeof(T); i++) {
      aux = value & 0xff;
      value  >>= 8;
      result <<= 8;
      result |=  aux;
    }
    return result;
  #endif
  }  



  

protected:
  typedef list<change_log<ac_word> > log_list;
#ifdef AC_UPDATE_LOG
  log_list changes;                 //!< Update log.
  fstream update_file;              //!< Update log binary file.
#endif

#ifdef AC_DELAY
  log_list delays;                  //!< Delayed update queue.
#endif

public:

  ///Default constructor
  explicit ac_memport(ac_arch<ac_word, ac_Hword>& ref) : ac_arch_ref<ac_word, ac_Hword>(ref),time_info(0,SC_NS){
        bytesPerBlock = 0;
        buf.ptr8 = NULL;
  }

  ///Default constructor with initialization
  explicit ac_memport(ac_arch<ac_word, ac_Hword>& ref, ac_inout_if& stg) : ac_arch_ref<ac_word, ac_Hword>(ref), storage(&stg),time_info(0,SC_NS) {
        bytesPerBlock = 0;
        buf.ptr8 = NULL;
  }

  virtual ~ac_memport() { if (buf.ptr8 != NULL) delete [] buf.ptr8; }

  // initializeBuffer and setBlockSize are necessary for cache<->memory data transference
  // if there is a cache using ac_memport, the number os units of data per block is a necessary
  // parameter for using read_block and write_block (ac_memport methods)
  void initializeBuffer ()
  {
      buf.ptr8 = new uint8_t [bytesPerBlock];
  }

  void setBlockSize (int cacheBlockSize)
  {
      // cacheBlockSize is the number of units of data in a cache block
      this->bytesPerBlock = (cacheBlockSize * sizeof(ac_word));
      this->initializeBuffer();
  }
  //////////////////////////////////////////////////////////////////////////////////////////

  sc_core::sc_time getTimeInfo()
  {
     return time_info;
  }

  void setTimeInfo(sc_core::sc_time time)
  { 
      time_info = time;
  }

  void setProcId(unsigned int id)
  {  
      procId = id;  
  }


  uint32_t byte_to_word(uint32_t a) {
        return a/sizeof(ac_word);
      }

///Reads a word
  inline ac_word read(uint32_t address) {
  //printf("\n\nAC_MEMPORT::read-> address=%x", address);

  sc_core::sc_time time = sc_core::sc_time(0, SC_NS);

    storage->read(&aux_word, address, sizeof(ac_word) * 8,time,this->procId);
    if (!this->ac_mt_endian) {
      aux_word = byte_swap(aux_word);
    }
    setTimeInfo (time);
    return aux_word;
  }

  ///Reads a byte
  inline uint8_t read_byte(uint32_t address) {
    //printf("\n\nAC_MEMPORT::read_byte->address=%x", address);
    sc_core::sc_time time = sc_core::sc_time(0, SC_NS);
    storage->read(&aux_byte, address, 8,time,this->procId);
    setTimeInfo (time);
    return aux_byte;
  }

  ///Reads half word
  inline ac_Hword read_half(uint32_t address) {

    //printf("\n\nAC_MEMPORT::read_half address=%x", address);

    sc_core::sc_time time = sc_core::sc_time(0, SC_NS);

    storage->read(&aux_Hword, address, sizeof(ac_Hword) * 8,time,this->procId);

    if (!this->ac_mt_endian) {
      aux_Hword = convert_endian(sizeof(ac_Hword), aux_Hword, 0);
    }
    setTimeInfo (time);
    return aux_Hword;
  }
  

  const ac_word *read_block(uint32_t address, unsigned l) {
   
      sc_core::sc_time time = sc_core::sc_time(0, SC_NS);
      ac_word *p = (ac_word*) buf.ptr8;

      /*if (l % sizeof(ac_word))
      { 
        printf("Memory size not wordsize aligned\n");
        exit(1);
      }
          
      storage->read(p, address, sizeof(ac_word) * 8, l/sizeof(ac_word), time,this->procId);
      setTimeInfo (time);
      return p;
      */
    
      

      

      l = byte_to_word(l);

      for (unsigned i=0; i<l; i++)
      {
        storage->read(&(p[i]), address+i*sizeof(ac_word), sizeof(ac_word) * 8,time,this->procId);
        setTimeInfo (time);
      }
      

      return p;      

      
      
      
  }


  //!Writing a word
    inline void write(uint32_t address, ac_word datum) {

      //printf("\n\nAC_MEMPORT::write-> address=%x datum=%x", address, datum);

      sc_core::sc_time time = sc_core::sc_time(0, SC_NS);
      aux_word = datum;
      if (!this->ac_mt_endian) {
      aux_word = byte_swap(datum);

      }
      storage->write(&aux_word, address, sizeof(ac_word) * 8,time,this->procId);
      setTimeInfo (time);
    }

   //!Writing a byte
    inline void write_byte(uint32_t address, uint8_t datum) {

        //printf("\n\nAC_MEMPORT::write_byte->address=%x datum=%x", address, datum);

        sc_core::sc_time time = sc_core::sc_time(0, SC_NS);
        storage->write(&datum, address, 8,time,this->procId);
        setTimeInfo (time);
    }

    //!Writing a short int
    inline void write_half(uint32_t address, ac_Hword datum) {

       //printf("\n\nAC_MEMPORT::write_half-> address=%x datum=%x", address, datum);

       sc_core::sc_time time = sc_core::sc_time(0, SC_NS);

       aux_Hword = datum;

       if (!this->ac_mt_endian) {
          aux_Hword = convert_endian(sizeof(ac_Hword), datum, 0);
       }

       storage->write(&aux_Hword, address, sizeof(ac_Hword) * 8,time,this->procId);
       setTimeInfo (time);
    }

    void write_block(uint32_t address, const ac_word *d, unsigned length) {
        //printf("AC_MEMPORT::write_block-> address=%x length=%u bytes\n", address, length);

        sc_core::sc_time time = sc_core::sc_time(0, SC_NS);

        /*
        if (length % sizeof(ac_word))
        { 
          printf("Memory size not wordsize aligned\n"); 
          exit(1);
        }
        

        storage->write((uint32_t*) d, address, sizeof(ac_word) * 8, l/sizeof(ac_word), time,this->procId); //TODO Remove cast
        
        setTimeInfo (time);
        */

        /*This code works but is inneficient*/

        unsigned l = byte_to_word(length);

        for (unsigned i=0; i<l; i++)
        {
          aux_word = d[i];
          storage->write(&aux_word, address+i*sizeof(ac_word), sizeof(ac_word) * 8,time,this->procId);
          setTimeInfo (time);
        }
        
        

// read:  storage->read  (p,             address, sizeof(ac_word) * 8, l/sizeof(ac_word), time,this->procId);
// write: storage->write ((uint32_t*) d, address, sizeof(ac_word) * 8, length / (sizeof(ac_word) * 8), time,this->procId); //TODO Remove cast
    }







#ifdef AC_DELAY
  //!Writing a word
  inline void write(uint32_t address, ac_word datum, uint32_t time) {
    if (!this->ac_mt_endian)
      delays.push_back(change_log<ac_word>(address, byte_swap(datum), time));
    else
      delays.push_back(change_log<ac_word>(address, datum, time));
  }

  //!Writing a byte 
  inline void write_byte(uint32_t address, uint8_t datum,
       uint32_t time) {
    uint32_t oset_addr = address % sizeof(ac_word);
    uint32_t base_addr = address - oset_addr;

    storage->read(&aux_word, base_addr, sizeof(ac_word) * 8);

    ((uint8_t*)(&aux_word))[oset_addr] = datum;
    
    delays.push_back(change_log<ac_word>(base_addr, aux_word, time));
    
  }

  //!Writing a short int 
  inline void write_half(uint32_t address, ac_Hword datum, uint32_t time) {
    uint32_t oset_addr = address % sizeof(ac_word);
    uint32_t base_addr = address - oset_addr;

    storage->read(&aux_word, base_addr, sizeof(ac_word) * 8);

    aux_Hword = datum;
    if (!this->ac_mt_endian) {
      aux_Hword = convert_endian(sizeof(ac_Hword), datum, 0);
    }
    ((ac_Hword*)(&aux_word))[oset_addr] = aux_Hword;
    
    delays.push_back(change_log<ac_word>(base_addr, aux_word, time));
    
  }

#endif

  /** 
   * Locks the device.
   * 
   */
  inline void lock() {
    storage->lock();
  } 

  /** 
   * Unlocks the device.
   * 
   */
  inline void unlock() {
    storage->unlock();
  }

#ifdef AC_UPDATE_LOG
  //! Reset log lists.
  void reset_log() { changes.clear(); }
  
  //!Dump storage device log.
  int change_dump(ostream& output) {
    log_list::iterator itor;
    
    if (changes.size()) {
      output << endl << endl;
      output << "**************** ArchC Change log *****************\n";
      output << "* Device: "<< name << "\t\t" << "PC: " << hex << ios_ac_pc << dec << endl;
      output << "***************************************************\n";
      output << "*        Address         Value          Time      *\n";
      output << "***************************************************\n";
      
      for (itor = changes.begin(); itor != changes.end(); itor++)
  output << "*  " << *itor << "     *" << endl;
      
      output << "***************************************************\n";
    }
    return 0;
  }
  
  //!Save storage device log.
  void change_save() {
    log_list::iterator itor;
    
    if (changes.size()) {

    for (itor = changes.begin(); itor != changes.end(); itor++)
      itor->save(this->update_file);

  }
}
#endif

  //!Method to provide the name of the device.
  inline const char* get_name() {
    return "";
  }

  //!Method to provide the size of the device.
  uint32_t get_size() {
    return storage->get_size();
  }

#ifdef AC_UPDATE_LOG
  //!Method to provide the change list.
  log_list* get_changes() {
    return &changes;
  }
#endif

 /// ***** EDIT *****
  /// Method to load device content from a file.
  /// Use local Data array
  inline void load(char* file) {

    ifstream input;
    string read;
    string word;
    istringstream line;
    unsigned text_size=0;
    bool is_addr, is_text=0, first_addr=1;
    long long data;
    unsigned int  addr=0;
    unsigned char* Data;

    Data = new unsigned char[storage->get_size()];

    sc_core::sc_time time(0,SC_NS);

    //Try to read as ELF first
    if (ac_load_elf<ac_word, ac_Hword>(*this, file, Data, storage->get_size(), this->ac_heap_ptr, this->ac_start_addr, this->ac_mt_endian) == EXIT_SUCCESS) {
      //init decode cache and return
      if(!this->dec_cache_size)
        this->dec_cache_size = this->ac_heap_ptr;
      storage->write(Data, 0, 32, (this->ac_heap_ptr)/4,time);
      setTimeInfo (time);
      delete[] Data;
      return;
    }

    // Looking for initialization file.
    input.open(file);
    if(!input){
      AC_ERROR("Could not open input file:" << file);
      AC_ERROR("Simulation aborted.");
      exit(1);
    }
    else{

      while( !input.eof() ){

        line.clear();
        getline(input, read);
        line.str(read);

        is_addr = 1;

        //Processing line
        while(line >> word){

    //Processing word
    if( is_addr ){
      addr = strtol(word.c_str(), NULL, 16);
      is_addr = 0;
      if( is_text && first_addr ){
        first_addr = 0;
        this->ac_start_addr = addr;
      }
    }
    else{
                                        
      if(is_text)text_size++;
      data = strtoll( word.c_str(), NULL, 16);
      write(addr,(ac_word)data);
      addr+= sizeof(ac_word);
    }
  }
      }
    }
    if(!this->dec_cache_size)
      this->dec_cache_size = addr;

    input.close();
  }

  /// ***** EDIT *****
  //!Method to load device content from an array.
  void load_array(uint8_t* d, const uint32_t s) {
    unsigned char* Data;
    if (storage->get_size() < s) {
      fprintf(stderr, "Storage %s: trying to load an array bigger then storage size.\n", storage->get_name().c_str());
      exit(EXIT_FAILURE);
    }

    sc_core::sc_time time(0,SC_NS);
    storage->write((ac_ptr)d, 0, 8, s,time);
    setTimeInfo (time);
  }


#ifdef AC_DELAY

  //!Committing delayed updates
  virtual void commit_delays(double time) {
    typename log_list::iterator itor;

    itor = delays.begin();

    // Sometimes, when a memory hierarchy is present and the processor spends
    // some cycles in a wait status, we may have to commit changes for every
    // cycle <= current time.
    while (delays.size() && (itor->time <= time)) {
      storage->write(&(itor->value), itor->addr, sizeof(ac_word) * 8);
      itor = delays.erase(itor);
    }
  }

#endif

  //!Dump the entire contents of a storage device
  virtual void dump() {}

  ///Binding operator
  inline void operator ()(ac_inout_if& stg) {
    storage = &stg;
  }

};
#endif




