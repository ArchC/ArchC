/**
 * @file      ac_tlm2_nb_port.cpp
 * @author    Liana Duenha
 *
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 * 
 * @date      22, October, 2012
 * @brief     Defines the ArchC TLM 2.0  port.

 *
 * @attention Copyright (C) 2002-2012 --- The ArchC Team
 *
 */

//////////////////////////////////////////////////////////////////////////////


// Standard includes

// SystemC includes

// ArchC includes
#include "ac_tlm2_nb_port.H"
#include "ac_tlm2_payload.H"


// If you want to debug TLM 2.0, please uncomment the next line

//#define debugTLM2 

// Constructors

/** 
 * Default constructor.
 * 
 * @param size Size or address range of the element to be attached.
 * 
 */
ac_tlm2_nb_port::ac_tlm2_nb_port(char const* nm, uint32_t sz) : name(nm), size(sz), LOCAL_init_socket() {

  LOCAL_init_socket.register_nb_transport_bw(this, &ac_tlm2_nb_port::nb_transport_bw);

}


tlm::tlm_sync_enum  ac_tlm2_nb_port::nb_transport_bw(ac_tlm2_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &time)
{

	#ifdef debugTLM2
	printf("\n\nNB_TRANSPORT_BW --> Processor is receiving a package");
	#endif
 	
	unsigned char* data_pointer = payload.get_data_ptr();		 
	uint32_t addr = (uint32_t) payload.get_address();

	payload_global->set_data_ptr(data_pointer);

	payload_global->deep_copy_from(payload);
        payload_global->free_all_extensions();	

	#ifdef debugTLM2
	printf("\nAC_TLM2_NB_PORT NB_TRANSPORT_BW: command-->%d  data-->%d payload_global data->%d address-->%ld",payload_global->get_command(),*data_pointer,*(payload_global->get_data_ptr()),addr);        
	printf("\nNotifying a event in BW TRANSPORT");
        #endif

	this->wake_up.notify();

	phase = tlm::END_RESP;
	tlm::tlm_sync_enum status = tlm::TLM_COMPLETED;

	return status;


}


//////////////////////////////////////////////////////////////////////////////
/** 
 * Reads a single word.
 * 
 * @param buf Buffer into which the word will be copied.
 * @param address Address from where the word will be read.
 * @param wordsize Word size in bits.
 * 
 */
void ac_tlm2_nb_port::read(ac_ptr buf, uint32_t address, int wordsize,sc_core::sc_time &time_info)
{

	payload_global = new ac_tlm2_payload();

	tlm::tlm_phase phase = tlm::BEGIN_REQ;
	tlm::tlm_sync_enum status;

  	//sc_core::sc_time time_info = sc_core::sc_time(0, SC_NS);
	unsigned char p[32];

	payload_global->set_command(tlm::TLM_READ_COMMAND);
	payload_global->set_address((sc_dt::uint64)address);
	payload_global->set_data_ptr(p);
	
        if (wordsize == 8) payload_global->set_data_length(sizeof(uint8_t));
        else if (wordsize == 16) payload_global->set_data_length(sizeof(uint16_t));
        else if (wordsize == 32) payload_global->set_data_length(sizeof(uint32_t));
	else {
		printf("\n\nAC_TLM2_NB_PORT READ: wordsize not implemented");
		exit(0);
	}

	#ifdef debugTLM2 
	printf("\n\n*******AC_TLM2_NB_PORT READ: command-->%d address-->%ld",tlm::TLM_READ_COMMAND, address);
	#endif

	status = LOCAL_init_socket->nb_transport_fw(*payload_global, phase, time_info);
	if(status != tlm::TLM_UPDATED)
	{
		printf("\nAC_TLM2_NB_PORT READ ERROR");
		exit(0);
	}

	wait(this->wake_up);
	
	
	uint8_t data8;
	uint16_t data16;
	uint32_t data32;

	   switch (wordsize) 
	   {
	      case 8:
	       	data8 = *((uint8_t*)payload_global->get_data_ptr());
		*(buf.ptr8) = ((uint8_t*)&data8)[0];
	        #ifdef debugTLM2 
		printf("\nAC_TLM2_NB_PORT READ: wordsize-->%d  data8-->%d data-->%d, address-->%ld",wordsize,data8,*(payload_global->get_data_ptr()),payload_global->get_address());
		#endif
	       	break;

	      case 16:
	      
		data16 = *((uint16_t*)payload_global->get_data_ptr());
		*(buf.ptr16) = ((uint16_t*)&data16)[0];

	  	#ifdef debugTLM2 
                printf("\nAC_TLM2_NB_PORT READ: wordsize-->%d  data16-->%d data-->%d, address-->%ld",wordsize,data16,*(payload_global->get_data_ptr()),payload_global->get_address());
		#endif
                break;
	      case 32:
	       
		data32 = *((uint32_t*)payload_global->get_data_ptr());

		*(buf.ptr32) = ((uint32_t*)&data32)[0];

        	#ifdef debugTLM2 
		printf("\nAC_TLM2_NB_PORT READ: wordsize-->%d  data32-->%d data-->%d, address-->%ld",wordsize,data32,*(payload_global->get_data_ptr()),payload_global->get_address());
		#endif
        	break;
	      case 64:
	      default:
        	printf("*** AC_TLM2_NB_PORT READ: wordsize-->%d not supported ****", wordsize);
		exit(0);
	        break;
	   }

	
	delete payload_global;
 	
}

/* read n_words */

void ac_tlm2_nb_port::read(ac_ptr buf, uint32_t address,
                         int wordsize, int n_words,sc_core::sc_time &time_info) {

	payload_global = new ac_tlm2_payload();

	tlm::tlm_phase phase = tlm::BEGIN_REQ;
	tlm::tlm_sync_enum status;

	//sc_core::sc_time time_info = sc_core::sc_time(0, SC_NS);
	payload_global->set_command(tlm::TLM_READ_COMMAND);
	
	unsigned char p[32];
	unsigned int i;
	
	#ifdef debugTLM2 
	printf("\n\n*******AC_TLM2_NB_PORT READ N_WORDS: wordsize--> %d command-->%d address-->%ld",wordsize,tlm::TLM_READ_COMMAND, address);
	#endif

	switch (wordsize) 
	{
	    case 8:
   	       
		for( i=0; i<n_words; i++)
  		{	
			payload_global->set_address(address +i);
			payload_global->set_data_length(sizeof(uint8_t));
			payload_global->set_data_ptr(p);

			status = LOCAL_init_socket->nb_transport_fw(*payload_global, phase, time_info); 
			if(status != tlm::TLM_UPDATED)
			{
				printf("\nAC_TLM2_NB_PORT n_words READ ERROR");
				exit(0);
			}
	
			wait(this->wake_up);
			
	
			unsigned char* data_pointer = payload_global->get_data_ptr();		

			for (int j = 0; (i < n_words) && (j < 4); j++, i++) {
				(buf.ptr8)[i] = ((uint8_t*)data_pointer)[j];
			}
			i--;
			
   		}
		break;
   
 	  case 16:
		for( i=0; i<n_words; i++)
  		{
			payload_global->set_address(address + (i * sizeof(uint16_t)));
			payload_global->set_data_length(sizeof(uint16_t));
			payload_global->set_data_ptr(p);

			status = LOCAL_init_socket->nb_transport_fw(*payload_global, phase, time_info); 
			if(status != tlm::TLM_UPDATED)
			{
				printf("\nAC_TLM2_NB_PORT n_words READ ERROR");
				exit(0);
			}
	
			wait(this->wake_up);

    		        unsigned char* data_pointer = payload_global->get_data_ptr();


			for (int j = 0; (i < n_words) && (j < 4); j++, i++) {
				buf.ptr16[i] = ((uint16_t*)data_pointer)[j];
			}
			i--;

		}
       		break;
	  case 32:
		
		for( i=0; i<n_words; i++)
  		{	
			payload_global->set_address(address + (i * sizeof(uint32_t)));
			payload_global->set_data_length(sizeof(uint32_t));
		        payload_global->set_data_ptr(p);

			status = LOCAL_init_socket->nb_transport_fw(*payload_global, phase, time_info); 
			if(status != tlm::TLM_UPDATED)
			{
				printf("\nAC_TLM2_NB_PORT n_words READ ERROR");
				exit(0);
			}
	
			wait(this->wake_up);

			unsigned char* data_pointer = payload_global->get_data_ptr();
			
			buf.ptr32[i] = *((uint32_t*)data_pointer);

   		}	
	
	        break;
  	  case 64:
	  default:
		printf("\n\nAC_TLM2_NB_PORT WRITE: wordsize not implemented");
		exit(0);	
		break;
        }
	 
	delete payload_global;
 		
}

/** 
 * Writes a single word.
 * 
 * @param buf Buffer from which the word will be copied.
 * @param address Address to where the word will be written.
 * @param wordsize Word size in bits.
 *
 */
void ac_tlm2_nb_port::write(ac_ptr buf, uint32_t address, int wordsize,sc_core::sc_time &time_info) {

  unsigned char p[32];
  unsigned int i;
  
  unsigned char *ptr;

  payload_global = new ac_tlm2_payload();

  tlm::tlm_phase phase = tlm::BEGIN_REQ;
  tlm::tlm_sync_enum status;

  //sc_core::sc_time time_info = sc_core::sc_time(0, SC_NS);


  #ifdef debugTLM2 
  printf("\n\n*******AC_TLM2_NB_PORT WRITE: wordsize--> %d command-->%d address-->%ld",wordsize,tlm::TLM_WRITE_COMMAND, address);
  #endif


  switch (wordsize) {
  case 8:
    payload_global->set_command(tlm::TLM_READ_COMMAND);
    payload_global->set_address((uint64_t)address);
    payload_global->set_data_length(sizeof(uint8_t));
    payload_global->set_data_ptr(p);
    
    status = LOCAL_init_socket->nb_transport_fw(*payload_global, phase, time_info); 
    if(status != tlm::TLM_UPDATED)
    {
	printf("\nAC_TLM2_NB_PORT  WRITE ERROR");
	exit(0);
    }
	
    #ifdef debugTLM2 
    printf("\n\nAC_TLM2_NB_PORT WRITE is waiting for wake_up event");
    #endif

    wait(this->wake_up);

    
    payload_global->set_command(tlm::TLM_WRITE_COMMAND);

    ptr = payload_global->get_data_ptr();
    ((uint8_t*)ptr)[0] = *(buf.ptr8);

    payload_global->set_address((uint64_t)address);
    payload_global->set_data_ptr(ptr);

    
    status =  LOCAL_init_socket->nb_transport_fw(*payload_global, phase, time_info); 
    if(status != tlm::TLM_UPDATED)
    {
	printf("\nAC_TLM2_NB_PORT  WRITE ERROR");
	exit(0);
    }
    wait(this->wake_up);
    

    break;
  case 16:
    payload_global->set_command(tlm::TLM_READ_COMMAND);
    payload_global->set_address((uint64_t)address);
    payload_global->set_data_length(sizeof(uint16_t));
    payload_global->set_data_ptr(p);

    status = LOCAL_init_socket->nb_transport_fw(*payload_global, phase, time_info); 
    if(status != tlm::TLM_UPDATED)
    {
	printf("\nAC_TLM2_NB_PORT  WRITE ERROR");
	exit(0);
    }
	
    wait(this->wake_up);
    
    payload_global->set_command(tlm::TLM_WRITE_COMMAND);
    ptr = payload_global->get_data_ptr();

    ((uint16_t*)ptr)[0] = *(buf.ptr16);
    payload_global->set_address((uint64_t)address);
    payload_global->set_data_ptr(ptr);

    status = LOCAL_init_socket->nb_transport_fw(*payload_global, phase, time_info); 
    if(status != tlm::TLM_UPDATED)
    {
	printf("\nAC_TLM2_NB_PORT  WRITE ERROR");
	exit(0);
    } 
    
    wait(this->wake_up);
    

    break;
 
 case 32:
    payload_global->set_address((uint64_t)address);
    payload_global->set_command(tlm::TLM_WRITE_COMMAND);
    payload_global->set_data_length(sizeof(uint32_t));

    ((uint32_t*)p)[0]=*(buf.ptr32);

    payload_global->set_data_ptr(p);    
    status = LOCAL_init_socket->nb_transport_fw(*payload_global, phase, time_info); 

    if(status != tlm::TLM_UPDATED)
    {
	printf("\nAC_TLM2_NB_PORT  WRITE ERROR");
	exit(0);
    } 

    wait(this->wake_up);
	

    break;
 
  case 64:
  default:
	printf("\n\nAC_TLM2_NB_PORT WRITE: wordsize not implemented");
	exit(0);
    break;
  }

  
  delete payload_global;

}

/** 
 * Writes multiple words.
 * 
 * @param buf Buffer from which the words will be copied.
 * @param address Address to where the words will be written.
 * @param wordsize Word size in bits.
 * @param n_words Number of words to be written.
 * 
 */
void ac_tlm2_nb_port::write(ac_ptr buf, uint32_t address,
                         int wordsize, int n_words,sc_core::sc_time &time_info) {

  printf("ac_tlm2_nb_port write isn't implemented");
  
}




string ac_tlm2_nb_port::get_name() const {
  return name;
}

uint32_t ac_tlm2_nb_port::get_size() const {
  return size;
}

/** 
 * Locks the device.
 * 
 */
void ac_tlm2_nb_port::lock()
{
	printf("\nAC_TLM2_NB_PORT::LOCK ERROR --> NOT IMPLEMENTED\n");  
	exit(0);
	
}

/** 
 * Unlocks the device.
 * 
 */
void ac_tlm2_nb_port::unlock()
{

	printf("\nAC_TLM2_NB_PORT::UNLOCK ERROR --> NOT IMPLEMENTED\n");  
	exit(0);
}

//////////////////////////////////////////////////////////////////////////////

// Destructors

/**
 * Default (virtual) destructor.
 * @return Nothing.
 */
ac_tlm2_nb_port::~ac_tlm2_nb_port() {


 
}


