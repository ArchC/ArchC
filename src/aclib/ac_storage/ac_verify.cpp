/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_verify.cpp
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include "ac_verify.H"


/*!Logging structural model changes for a given device.
  \param pdevchg A pointer to the change list of the device.
  \param address The address being modified.
  \param datum   The new value being stored at address.
  \param time    Simulation time of this modification.
*/
void ac_verify::add_log( log_list  *pdevchg, unsigned address, unsigned datum, double time  ) { 

  log_list::iterator itor;


  if( pdevchg->size() ) {

    //Looking for another change on the same address.
    //Its necessary to be sure that we keep only the last
    //change made during a cycle to avoid being tricked by
    //signal oscilation.
    itor = pdevchg->begin();
    while( (itor != pdevchg->end()) && 
           !(itor->addr == address && itor->time == time) ){
      itor++;
    }

    if(itor->addr == address && itor->time == time){
      itor->value = datum;
    }
    else{
      pdevchg->push_back( change_log(address, datum , time));
    }
  }
  //List was empty ...
  else{
    pdevchg->push_back( change_log(address, datum , time));
  }
}
  

    
/*!Match device's behavioral and structural logs at a given simulation time. 
  Modifications ocurred to the same address, with the same value at the same
  time can be discarded. Used for both timed and untimed  verification.
  
  \param pdevice A pointer to the device object being checked.
  \param pdevice A pointer to the structural change list for this device.
  \param time    The simulation time being checked.
*/
void ac_verify::match_logs( ac_storage *pbhvdevice,
                            log_list *pdevchange,
                            double time ) {

  log_list *pbhvchange;
  log_list::iterator bhvitor;
  log_list::iterator devitor;
  bool flag = false;

  pbhvchange = pbhvdevice->get_changes();
  bhvitor = pbhvchange->begin();

  if( pbhvchange->size() && pdevchange->size())

    while( bhvitor != pbhvchange->end() ){
    
      devitor = pdevchange->begin();
      flag = false;

      while( (bhvitor->time == devitor->time) && (devitor != pdevchange->end())){

        if( bhvitor->addr == devitor->addr &&
            bhvitor->value == devitor->value ){

          bhvitor = pbhvchange->erase( bhvitor );
          devitor = pdevchange->erase( devitor );
          flag = true;
          break;
        }
        devitor++;
      }
      if(!flag)
        bhvitor++;
     
    }
}

/*!Check a device behavioral and structural logs in the end of simulation. 
  Used for untimed  verification. In this case, the logs need to have
  modifications in the same order, but not at the same time.
  
  \param pdevice A pointer to the device object being checked.
  \param pdevice A pointer to the structural change list for this device.
*/
void ac_verify::check_final( ac_storage *pbhvdevice,
                             log_list *pdevchange){

  log_list *pbhvchange;
  log_list::iterator bhvitor;
  log_list::iterator devitor;
  bool flag;

  pbhvchange = pbhvdevice->get_changes();

  //Test if both logs have elements.
  if( pbhvchange->size() && pdevchange->size()){

    //Sorting vectors.
    pbhvchange->sort();
    pdevchange->sort();

    bhvitor = pbhvchange->begin();
    devitor = pdevchange->begin();

    //Checking consistence.    
    while( (bhvitor !=  pbhvchange->end()) &&  (devitor !=  pdevchange->end()) ){
      
      flag = false;
      if( bhvitor->addr == devitor->addr &&
          bhvitor->value == devitor->value ){
      
        bhvitor = pbhvchange->erase( bhvitor );
        devitor = pdevchange->erase( devitor );
        flag = true;
      }
      if(!flag){
        bhvitor++;
        devitor++;
      }
    }
  }

  if( pbhvchange->size() || pdevchange->size()){
    bhvitor = pbhvchange->begin();
    devitor = pdevchange->begin();

    AC_ERROR( "Verification failed for device " << pbhvdevice->get_name() << "! See logs in ac_verification.log");

    output << "************************************************************"<<endl;
    output << "*      ArchC Storage Based Co-Verification ERROR Log       *"<<endl;
    output << "************************************************************"<<endl;
    output << endl << "The following updates are inconsistent..."<<endl<<endl;

    output << "------------------------------------------------------------\n";
    output << "=> Device: "<< pbhvdevice->get_name() <<endl;
    output << "------------------------------------------------------------\n";
    output<< "                    Address \t\tValue \t\tTime"<<endl; 
    output<<"Behavioral: "<<endl;
    while(bhvitor !=  pbhvchange->end()){
      output << "\t\t"<< *bhvitor<< endl;
      bhvitor++;
    }
    output << "Structural: "<<endl;
    while(devitor !=  pdevchange->end()){
      output << "\t\t"<< *devitor<< endl;
      devitor++;
    }
    output << endl << endl;
  }
  
}
