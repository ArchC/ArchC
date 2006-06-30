/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_cache.cpp
 * @author    Pablo Viana <pvs@cin.ufpe.br>
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            CIn - UFPE
 *
 * @version   1.0
 * @date      Thu Sep 22 11:47:36 2005
 *
 * @brief     The ArchC Cache device class. Implementation file.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include "ac_cache.H"
#include "ac_resources.H"

//!Private method for the generation of trace files to be utilized with DineroIV
  void ac_cache::tracing(unsigned address, unsigned type)
  {
#ifdef  AC_TRACE  //! Trace files generation
      access_type = type;
      size_ref = 1;
      char * buffer;
      buffer = new char [8];
      for (int b = 0; b < 8; b++)
          buffer[b]=0;
      buffer[0] = address;
      buffer[4] = size_ref;
      buffer[6] = access_type;
      buffer[7] = padding;
//      trace.write(buffer, 8);
      trace << address << endl;
      delete[] buffer;
#endif
  }

//!Private method used for the reference slicing into tag, index and offset fields
  void ac_cache::addressing(unsigned address)
  {
      //!Define the position information
      unsigned offset_field = (AC_WORDSIZE/8) * this->block_size;
      unsigned index_field = this->num_sets;
      unsigned tag_field = offset_field*index_field;
      this->offset = address % offset_field;
      this->set = (address/offset_field) % index_field;
      this->address_tag = address/tag_field;
//      cout << "ADDRESS TAG: " << address_tag << endl;
      //!Point to the 1st element of the set in question
      slot_tag = (unsigned *)(this->tag + (this->set * this->set_size));
      slot_valid = (bool *)(this->valid + (this->set * this->set_size));
      hit = -1;                           // reset the hit event
      unsigned *test_tag;                 // tag to compare
      bool *test_valid;                   // bit_valid to compare

      //Scan the set searching for the tag desired
      for(this->element = 0; this->element < this->set_size ; this->element++)
      {
        test_tag = (unsigned *)(slot_tag + element);
        test_valid = (bool *)(slot_valid + element);

      	if((*test_tag == address_tag)&&(*test_valid))
      	{
      	  hit = element;   //It has found the reference in the set. Gets the line number.
//      	  cout << "Hit at TAG: " << *test_tag << endl;
      	}
      }
      //Points to the correct mapped cache line
      if(hit!=-1) //Whether had got a hit.
      {
         slot_data = (char *)(this->Data + (hit + this->set * this->set_size) *this->block_size * AC_WORDSIZE/8);
         slot_tag = (unsigned *)(this->tag + (hit + this->set * this->set_size));
         slot_valid = (bool *)(this->valid + (hit + this->set * this->set_size));
         slot_dirty = (bool *)(this->dirty + (hit + this->set * this->set_size));
      }
      else    //The data was not found in the set
      {
      	 this->element = get_chosen(set);  //chooses the element to be replaced
         slot_data = (char *)(this->Data + (this->element + set * this->set_size) *this->block_size * AC_WORDSIZE/8);
         slot_tag = (unsigned *)(this->tag + (this->element + set * this->set_size));
         slot_valid = (bool *)(this->valid + (this->element + set * this->set_size));
         slot_dirty = (bool *)(this->dirty + (this->element + set * this->set_size));
      }
  }


//!Private method utilized to replace the cache with data from lower level
  void ac_cache::replaceBlockWrite()
  {

  	   unsigned base_address, f;
  	   switch(replace_status)
  	   {
  	   	    case 0:

  	   	         if((isWriteBack())&&(isWriteAllocate())&&(this->next_level != NULL)&&(*slot_valid==true)&&(*slot_dirty==true))
  	   	         {
//  	   	                cout << "writingback" << endl;
                        this->writingBack();
                        break;
  	   	         }
  	   	         else
  	   	         {
//  	   	         	    cout << "nao precisa wb" << endl;
  	   	                replace_status++;
  	   	         }



  	   	    case 1:
//  	   	         cout << "entrando no CASE 1 com status = " << (int)replace_status << endl;

                 if(isWriteAllocate())
                 {
                        f = ~(0);  //0xFFFFFFF...FFF
                        f = (unsigned) (f * block_size * AC_WORDSIZE/8); //0xFFFFFFF...F00 //works as a shift << LOG(blocksize*word size in Bytes)
                        base_address = requested_address & f; //cut off the least significant (offset) bits
//                        cout << "Requested address: " << requested_address << endl;
                        if(this->next_level != NULL)
                        {
//                            cout << "address em replaceWrite: " << hex << base_address << endl;
                            this->next_level->request_block(this, base_address, block_size*AC_WORDSIZE/8);
                            break;
                        }
                 }
                else
                {
                	    replace_status++;
                }

            case 2:
//  	   	         cout << "entrando no CASE 2 com status = " << (int)replace_status << endl;

                 //WRITE LOCAL
                 if(isWriteAllocate())
                 {
                 	    if(write_size == W_WORD)
                            this->ac_storage::write(slot_data + offset - Data, *(ac_word*)datum_ref);
                        else if(write_size == W_HALF)
                            this->ac_storage::write_half(slot_data + offset - Data, *(unsigned short*)datum_ref);
                        else if(write_size == W_BYTE)
                            this->ac_storage::write_byte(slot_data + offset - Data, *(char*)datum_ref);
                        //DIRTY SUJO
                        if(isWriteBack())
                        {
                            *(slot_dirty) = true;
                            this->ready();
//                            replace_status++;
                            break;
                        }
                        else
                        {
                            replace_status++;
                        }
                 }
                 else
                 {
                     replace_status++;
                 }

            case 3:
//  	   	         cout << "entrando no CASE 3 com status = " << (int)replace_status << endl;

                 if(this->next_level != NULL)
                 {
                 	    if(write_size == W_WORD)
                 	    {
                            this->next_level->request_write(this, requested_address, *(ac_word*)datum_ref);
                            break;
                        }
                        else if(write_size == W_HALF)
                        {
                            this->next_level->request_write_half(this, requested_address, *(unsigned short*)datum_ref);
                            break;
                        }
                        else if(write_size == W_BYTE)
                        {
                            this->next_level->request_write_byte(this, requested_address, *(char*)datum_ref);
                            break;
                        }
                 }

         }
  }

//!Private method utilized to replace the cache with data from lower level
  void ac_cache::replaceBlockRead(unsigned address)
  {

  	   unsigned base_address, f;
  	   switch(replace_status)
  	   {
  	   	    case 0:
  	   	         if((isWriteBack())&&(this->next_level != NULL)&&(*slot_valid==true)&&(*slot_dirty==true))
  	   	         {
//  	   	         	    cout << "address before WB: " << address << endl;
                         this->writingBack();
//  	   	         	    cout << "address after WB: " << address << endl;
                         break;
                 }
  	   	         else
  	   	         {
  	   	                replace_status++;
  	   	         }

  	   	    case 1:
//  	   	         cout << "entrei no case 1 com status = " << (int)replace_status << endl;
                 f = ~(0);  //0xFFFFFFF...FFF
                 f = (unsigned) (f * block_size * AC_WORDSIZE/8); //0xFFFFFFF...F00 //works as a shift << LOG(blocksize*word size in Bytes)
//                 cout << "f: " << f << endl;
//                 base_address = address & f; //cut off the least significant (offset) bits
                 base_address = requested_address & f; //cut off the least significant (offset) bits
                 if(this->next_level != NULL)
                 {
//                       cout << "address_base em replaceRead: " << hex << base_address << "address: " << address << endl;
//                       cout << "requested address: " << requested_address << endl;
                       this->next_level->request_block(this, base_address, block_size*AC_WORDSIZE/8);
                       break;
                 }

            case 2:
                 this->ready();
  	   }
  }

//!Private method utilized to store back to the lower level the modified data cache
  void ac_cache::writingBack()
  {
              //rebuilding the reference to be stored back in the lower level
              unsigned index_shift  = ((AC_WORDSIZE/8) * this->block_size);
              unsigned tag_shift = index_shift*this->num_sets;
              unsigned base_address = (*slot_tag)*tag_shift + set*index_shift;
//              cout << "tag_shift: " << tag_shift << "index_shift: " << index_shift << endl;
//              cout << "buffer enviado: " << (int)slot_data << endl;
              this->next_level->request_write_block(this, base_address, this->slot_data, this->block_size*AC_WORDSIZE/8);
//              cout << "data sent: " << hex << *(unsigned *)(slot_data)  << " Address: " << base_address << endl;
              *slot_dirty = false;

  }


  bool ac_cache::isWriteThrough()
  {
       if(write_policy & 0x01)
           return true;
       else
           return false;
  }

  bool ac_cache::isWriteBack()
  {
       if(write_policy & 0x02)
           return true;
       else
           return false;
  }

  bool ac_cache::isWriteAllocate()
  {
       if(write_policy & 0x10)
           return true;
       else
           return false;
  }

  bool ac_cache::isWriteAround()
  {
       if(write_policy & 0x20)
           return true;
       else
           return false;
  }


//!Read a word from the address passed as parameter
  ac_word ac_cache::read( unsigned address ) /*const*/
  {
      read_access_type = true;
      ac_word data_out;                    //hold the requested Data
      this->ac_cache::addressing(address); //slicing the address field
#ifdef  AC_TRACE                           //! Trace files generating
      this->ac_cache::tracing(address, 0); //access trace file registering a read operation
#endif
      //Read hit
      if(hit != -1){
//         data_out = ac_storage::read(slot_data + offset - Data);
      }
      //Read Miss
      else if (this->next_level != NULL)
      {
#ifdef AC_STATS
         ac_resources::ac_sim_stats.add_miss(name);
#endif
         this->stall();    //Stalls the processor, while the data is being provided
//         replace_status = 0;
         requested_address = address;
         this->replaceBlockRead(requested_address); // imitando o write, original address
      }
      data_out = ac_storage::read(slot_data + offset - Data);
      //Updates the tracking for replacement policies
      this->update(set, element);
      return (data_out);
  }

  //!Reading a byte
  unsigned char ac_cache::read_byte( unsigned address )
  {
      read_access_type = true;
      unsigned char data_out;                    //hold the requested Data
      this->ac_cache::addressing(address); //slicing the address field
#ifdef  AC_TRACE                           //! Trace files generating
      this->ac_cache::tracing(address, 0); //access trace file registering a read operation
#endif
      //Read hit
      if(hit != -1){
      }
      //Read Miss
      else if (this->next_level != NULL)
      {
#ifdef AC_STATS
         ac_resources::ac_sim_stats.add_miss(name);
#endif
         this->stall();    //Stalls the processor, while the data is being provided
//         replace_status = 0;
         requested_address = address;
         this->replaceBlockRead(requested_address);
      }
      data_out = ac_storage::read_byte(slot_data + offset - Data);
      //Updates the tracking for replacement policies
      this->update(set, element);
      return (data_out);
  }

  //!Reading half word
  ac_Hword ac_cache::read_half( unsigned address )
  {
      read_access_type = true;
      ac_Hword data_out;                    //hold the requested Data
      this->ac_cache::addressing(address); //slicing the address field
#ifdef  AC_TRACE                           //! Trace files generating
      this->ac_cache::tracing(address, 0); //access trace file registering a read operation
#endif
      //Read hit
      if(hit != -1){
      }
      //Read Miss
      else if (this->next_level != NULL)
      {
#ifdef AC_STATS
         ac_resources::ac_sim_stats.add_miss(name);
#endif
         this->stall();    //Stalls the processor, while the data is being provided
//         replace_status = 0;
         requested_address = address;
         this->replaceBlockRead(requested_address);
      }
      data_out = ac_storage::read_half(slot_data + offset - Data);
      //Updates the tracking for replacement policies
      this->update(set, element);
      return (data_out);
  }


  //!Writing into the cache
  void ac_cache::write( unsigned address, ac_word datum )
  {
      read_access_type = false;
      this->ac_cache::addressing(address);        //slicing the address field
#ifdef  AC_TRACE                                  //! Trace files generating
      this->ac_cache::tracing(address, 1);        //access trace file registering a write operation
#endif
      //Write hit
      if(hit != -1){
        //DIRTY SUJO
         if(isWriteBack())
         {
            *(slot_dirty) = true;
         }
         else if(isWriteThrough())
         {
            if(this->next_level != NULL)
            {
                //WAIT
                this->stall();
                //REQUEST WRITE
//                cout << "datum: " << datum << " slot_data+offset: " << *(ac_word*)(slot_data + offset) << endl;
                if(datum != *(ac_word*)(slot_data + offset))
                    this->next_level->request_write(this, address, datum);
                else
                    this->ready();
            }
            //RESPONSE WRITE
            //QUANDO CHEGAR, DESATIVA O WAIT
         }
        //WRITE LOCAL
        this->ac_storage::write(slot_data + offset - Data, datum);
//        cout << "escrevendo em " << (slot_data + offset - Data) << endl;
      }
      //Write Miss
      else
      {
//      	cout << "Miss Write" << endl;
#ifdef AC_STATS
         ac_resources::ac_sim_stats.add_miss(name);
#endif
         if(this->next_level != NULL)
         {
            //WAIT
            if(!isWriteAround()) //Only if the data must be locally writen
                this->stall();
            //ENTRA NA FSM
//            replace_status = 0; //reset FSM
            write_size = W_WORD;
            *(ac_word*)datum_ref = datum;
  	        requested_address = address;
//  	        cout << "FSM" << endl;
            this->replaceBlockWrite(); //FSM
         }
         else
         {
             *(slot_tag) = address_tag;
             *(slot_valid) = true;
             *(slot_dirty) = true;
             this->ac_storage::write(slot_data + offset - Data, datum);
//             cout << "write local" << endl;
         }
      }
      this->update(set, element);
  }


  //!Writing a byte to an address.
  void ac_cache::write_byte( unsigned address, unsigned char datum )
  {
      read_access_type = false;
      this->ac_cache::addressing(address);        //slicing the address field
#ifdef  AC_TRACE                                  //! Trace files generating
      this->ac_cache::tracing(address, 1);        //access trace file registering a write operation
#endif
      //Write hit
      if(hit != -1){
        //DIRTY SUJO
         if(isWriteBack())
         {
            *(slot_dirty) = true;
         }
         else if(isWriteThrough())
         {
            if(this->next_level != NULL)
            {
                //WAIT
                this->stall();
                //REQUEST WRITE
                unsigned char tmp= ac_storage::read_byte(slot_data + offset - Data);
                if(datum != tmp)
                    this->next_level->request_write_byte(this, address, datum);
                else
                    this->ready();
            }
         }
        //WRITE LOCAL
        this->ac_storage::write_byte(slot_data + offset - Data, datum);
      }
      //Write Miss
      else
      {
#ifdef AC_STATS
         ac_resources::ac_sim_stats.add_miss(name);
#endif
         if(this->next_level != NULL)
         {
            //WAIT
            if(!isWriteAround()) //Only if the data must be locally writen
               this->stall();
            //ENTRA NA FSM
//            replace_status = 0; //reset FSM
            write_size = W_BYTE;
            *(unsigned char*)datum_ref = datum;
  	        requested_address = address;
            this->replaceBlockWrite(); //FSM
         }
         else
         {
             *(slot_tag) = address_tag;
             *(slot_valid) = true;
             *(slot_dirty) = true;
             this->ac_storage::write_byte(slot_data + offset - Data, datum);
         }
      }
      this->update(set, element);
  }


  //!Writing a halfword to an address.
  void ac_cache::write_half( unsigned address, unsigned short datum )
  {
      read_access_type = false;
      this->ac_cache::addressing(address);        //slicing the address field
#ifdef  AC_TRACE                                  //! Trace files generating
      this->ac_cache::tracing(address, 1);        //access trace file registering a write operation
#endif
      //Write hit
      if(hit != -1){
        //DIRTY SUJO
         if(isWriteBack())
         {
            *(slot_dirty) = true;
         }
         else if(isWriteThrough())
         {
            if(this->next_level != NULL)
            {
                //WAIT
                this->stall();
                //REQUEST WRITE
                unsigned short tmp= ac_storage::read_half(slot_data + offset - Data);
                if(datum != tmp)
                    this->next_level->request_write_half(this, address, datum);
                else
                    this->ready();
            }
            //RESPONSE WRITE
            //QUANDO CHEGAR, DESATIVA O WAIT
         }
        //WRITE LOCAL
        this->ac_storage::write_half(slot_data + offset - Data, datum);
      }
      //Write Miss
      else
      {
#ifdef AC_STATS
         ac_resources::ac_sim_stats.add_miss(name);
#endif
         if(this->next_level != NULL)
         {
            //WAIT
            if(!isWriteAround()) //Only if the data must be locally writen
                this->stall();
            //ENTRA NA FSM
//            replace_status = 0; //reset FSM
            write_size = W_HALF;
            *(unsigned short*)datum_ref = datum;
  	        requested_address = address;
            this->replaceBlockWrite(); //FSM
         }
         else
         {
             *(slot_tag) = address_tag;
             *(slot_valid) = true;
             *(slot_dirty) = true;
             this->ac_storage::write_half(slot_data + offset - Data, datum);
         }
      }
      this->update(set, element);
  }

  //!
  //!Constructor. Where 'n' is the Name, 'bs' is Block Size in number of words, 'nb' is the Number of Blocks,
  //!'ss' is the Set Size in blocks per set and 'st' is the Strategy used for replacement (LRU or RANDOM)
  //!'wp' is the write policiy
  //!
  ac_cache::ac_cache( char *n, unsigned bs, unsigned nb, unsigned ss, unsigned st, unsigned char wp) :
    ac_storage(n, bs*nb*(AC_WORDSIZE/8)),
    block_size(bs),
    num_blocks (nb),
    set_size (ss),
    num_sets (nb/ss),
    strategy (st),
    write_policy (wp)
  {
    request_block_event = false;
    request_write_block_event = false;
    request_write_event = false;
//  	SC_METHOD(process_request);
//    sensitive << *bhv_pc;

   	next_level = NULL;
   	previous_level = NULL;
    tag = new unsigned[this->num_blocks];
    valid = new bool[this->num_blocks];
    dirty = new bool[this->num_blocks];
    chosen = new unsigned[this->num_sets];
//    cout << "Chosen reference" << chosen << endl;
    tracker = new unsigned[this->num_blocks];


//    cout << "Valid reference" << valid << endl;
    *valid = false;

    *dirty = false;
//    request_buffer = new char[block_size*(AC_WORDSIZE/8)];
    datum_ref = new char[4];
#ifdef AC_TRACE
    ac_cache::trace.open(n, ofstream::out);
//        ac_cache::trace.open(n, ofstream::binary);
#endif
  }




  //!Destructor
  ac_cache::~ac_cache(){

//      fprintf(stderr, "Destruindo a memoria %s \n", this->get_name());
//      fprintf(stderr, "Antes de detonar next 0x%x \n", next_level);
//      next_level = NULL;
//      fprintf(stderr, "Depois de detonar next 0x%x \n", next_level);
//      fprintf(stderr, "Antes de detonar tag 0x%x \n", tag);
      delete[] tag;
//      fprintf(stderr, "Antes de detonar Valid 0x%x \n", valid);
      delete[] valid;
//      fprintf(stderr, "Antes de detonar dirty 0x%x \n", dirty);
      delete[] dirty;
//      fprintf(stderr, "Antes de detonar chosen 0x%x \n", chosen);
      delete[] chosen;
//      fprintf(stderr, "Antes de detonar tracker 0x%x \n", tracker);
      delete[] tracker;
#ifdef AC_TRACE
//      closing the trace file generated
      ac_cache::trace.close();
#endif
//      delete[] request_buffer;
//      fprintf(stderr, "Memoria %s Destruida \n", this->get_name());
      delete[] datum_ref;

  }

  void ac_cache::bindTo(ac_cache& lower){  //were ac_cache
       this->next_level = &lower;
//       lower.previous_level = this;
  }


  void ac_cache::set_codeSize( unsigned s){
    codeSize = s;
  }

  unsigned ac_cache::get_chosen(unsigned s){
      unsigned maximum = 0;    //initiates the maximum
      long double norm;
      switch(this->strategy)
      {
          case LRU:      //!Least-recently used (LRU)
            for (unsigned t = 0 ; t < (unsigned)this->set_size ; t = t + sizeof(unsigned)) //searches for the maximum age
            {
               if( *(tracker + s*this->set_size + t) > maximum )
               {
               	   maximum = *(tracker + s*this->set_size + t);
               	   *(chosen + s) = t;                  //select the entry to be replaced
               }
               if(*(chosen + s) > this->set_size)
                   *(chosen + s) = 0;
            }
            break;

          case RANDOM:   //!Spreads allocation uniformly
            srand((int)(chosen + s));
            norm = rand();
            norm = norm * (this->set_size - 1);
            *(chosen + s) = (unsigned)((set_size-1)*(rand()/(double)RAND_MAX));
            break;

          default:
            *(chosen + s) += 1;
            if((*(chosen + s))>=set_size)
                *(chosen + s) = 0;
      }
#ifdef AC_DETAIL
      printf("Strategia %d: escolhido o %d \n",this->strategy, *(chosen + s));
#endif
      return *(chosen + s); //!Returns the value in the chosen list at the suitable set
  }

  //!Method that implements the policies of replacement, based on the last access
  void ac_cache::update(unsigned s, unsigned e)
  {
      for (unsigned t = 0 ; t < (unsigned)this->set_size ; t++)
      {
          *(tracker + s*this->set_size + t) += 1; //incremets all Tracker's entries inside the set
      }
      *(tracker + s*this->set_size + e) = 0;     //registering the last access
  }

  unsigned ac_cache::get_codeSize(){
    return codeSize;
  }
/*
################################################################################
##############           PROCESSOR STALLING              #######################
################################################################################
*/
  void ac_cache::stall(){
      ac_resources::ac_wait();
//       cout << "Wait "<< this->get_name() << endl;
  }


  void ac_cache::ready(){
      ac_resources::ac_release();
      this->replace_status = 0;
//       cout << "Ready "<< this->get_name() << endl;
  }


/*
################################################################################
##############                 INTERFACE                 #######################
################################################################################
*/

  void ac_cache::request_block(ac_cache_if* client, unsigned address, unsigned size_bytes)
  {
//  	   cout << "requesting from" << this->get_name() << endl;
  	   // cout << "size in bytes: " << dec << size << endl;
       client_global = client;
       request_buffer = new char[size_bytes*(AC_WORDSIZE/8)];
       for (unsigned offset_word = 0; offset_word < size_bytes; offset_word+=AC_WORDSIZE/8)
       {
          *(ac_word *)(request_buffer + offset_word) = this->read(address + offset_word);
           //cout << "bloco requisitado" << *(ac_word *)(request_buffer + offset_word) << endl;
       }
       request_block_event = true;
       //client->response_block(request_buffer);

   }

  void ac_cache::process_request() {
       if (request_write_block_event) {
          request_write_block_event = false;
          client_global->response_write_block();
       }else if (request_block_event) {
          request_block_event = false;
          client_global->response_block(request_buffer);
       }else if (request_write_event) {
          request_write_event = false;
          client_global->response_write();
       }
       //while (true)
       //{
       //	     wait(request_block_event);
       //      client_global->response_block(request_buffer);
       //}
  }

  void ac_cache::request_write_byte(ac_cache_if* client, unsigned address, unsigned char datum)
  {
  	   client_global = client;
       ac_cache::write_byte(address, datum);
       request_write_event = true;
       //notify(SC_ZERO_TIME, request_write_event);
       //client->response_write();
  }

  void ac_cache::request_write_half(ac_cache_if* client, unsigned address, unsigned short datum)
  {
  	   client_global = client;
       ac_cache::write_half(address, datum);
       request_write_event = true;
       //notify(SC_ZERO_TIME, request_write_event);
       //client->response_write();
  }

  void ac_cache::request_write(ac_cache_if* client, unsigned address, ac_word datum)
  {
  	   client_global = client;
       ac_cache::write(address, datum);
       request_write_event = true;
       //client->response_write();
  }

  void ac_cache::request_write_block(ac_cache_if* client, unsigned address, char* datum, unsigned size_bytes)
  {
  	   client_global = client;
  	   for (unsigned offset_word = 0; offset_word < size_bytes; offset_word+=AC_WORDSIZE/8)
  	   {
           write(address + offset_word/**AC_WORDSIZE/8*/, *(ac_word*)(datum + offset_word/**AC_WORDSIZE/8*/));
//           cout << "Data stored: " << read(address + offset_word) << endl;
       }

       request_write_block_event = true;
       //client->response_write_block();
  }

  void ac_cache::response_block(char* block)
  {
       *(slot_tag) = address_tag;
//       cout << "address tag: " << address_tag << endl;
       *(slot_valid) = true;
       *(slot_dirty) = false;
       for (unsigned offset_word = 0; offset_word < block_size; offset_word++)
       {
           *(ac_word *)(slot_data + offset_word*AC_WORDSIZE/8) = *(ac_word *)(block + offset_word*(AC_WORDSIZE/8));
//           cout << "dado: " << hex << *(ac_word *)(slot_data + offset_word*AC_WORDSIZE/8) << endl;
       }
       replace_status++;
       if(read_access_type)
       {
            this->replaceBlockRead(requested_address);
       }
       else
       {
            this->replaceBlockWrite();
       }
       delete[] block;
  }


  void ac_cache::response_write_byte()
  {
//       this->ready(requested_address);
  }

  void ac_cache::response_write_half()
  {
//       this->ready(requested_address);
  }

  void ac_cache::response_write()
  {
       this->ready();
  }

  void ac_cache::response_write_block()
  {
  	replace_status++;
       if(read_access_type)
            this->replaceBlockRead(requested_address);
       else
            this->replaceBlockWrite();
  }





  void ac_cache::bindToNext(ac_cache_if& next)
  {
       this->next_level = &next;
  }

  void ac_cache::bindToPrevious(ac_cache_if& previous)
  {
//       this->previous_level = (ac_cache&)&previous;
  }



