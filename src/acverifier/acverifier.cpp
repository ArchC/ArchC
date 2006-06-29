/**
 * @file      acverifier.cpp
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
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     The ArchC co-verification engine
 *            This file contains functions to control the ArchC 
 *            co-verification engine. This engine will supervise
 *            simulation of two ArchC models monitoring updates to the
 *            storage devices. This is accomplished through IPC messages
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#include "ac_msgbuf.H"

#define REFERENCE_MODEL 0
#define DUV_MODEL 1
#define AC_ERROR( msg )    cerr<< "ArchC ERROR: " << msg  <<'\n'
#define AC_MSG( msg )      cerr<< "ArchC: " << msg  <<'\n'

//#define DDEBUG
//#define DEBUG
using namespace std;

char ACVersion[] = "0.9-beta";
//This is the maximal number of unmatched logs accepted before aborting co-verification
static const unsigned int AC_MAX_UNMATCHED = 20;

//These variables will be used to control and access message queues
//and child processes
key_t ref_key, duv_key;
int ref_msqid, duv_msqid;
pid_t ref_pid, duv_pid;
struct msqid_ds ref_info, duv_info;

//Device list for both models
struct dev_list *ref_devlist = NULL, *duv_devlist = NULL;
struct start_msgbuf ref_sbuf, duv_sbuf;
struct dev_loglist *loglist = NULL;




//Debugging printf function
#ifdef DEBUG
#include <stdarg.h>
inline int dprintf(const char *format, ...)
{
  int ret;

	va_list args;
	va_start(args, format);
	ret = vfprintf(stderr, format, args);
	va_end(args);
  return ret;
}
#else
inline void dprintf(const char *format, ...) {}
#endif

//Defining DDEBUG will produce a real verbose verifier
#ifdef DDEBUG
#include <stdarg.h>
inline int ddprintf(const char *format, ...)
{
  int ret;

	va_list args;
	va_start(args, format);
	ret = vfprintf(stderr, format, args);
	va_end(args);
  return ret;
}
#else
inline void ddprintf(const char *format, ...) {}
#endif


//Function Prototypes
void CheckOptions(int model);
void ListInit(struct dev_list** model_devlist, int msqid, start_msgbuf *sbuf );
void CheckListConsistency(void);
void AddLog( long type, change_log log, int device );
void MatchLogs( void);

//void DoIt(void);
void DoItOntheFly(void);
void FinishIt(void);
void ChangeDump( struct dev_loglist *pllist );

/* Display the command line options accepted by ArchC verifier. */
static void DisplayHelp (){
  printf ("==================================================\n");
  printf (" This is ArchC Verifier for ArchC version %s\n", ACVersion);
  printf ("==================================================\n\n");
  printf ("Usage: ac_verifier REF_model DUV_model APP [--ref_args=arguments] [--duv_args=arguments]\n\n");
  printf ("    Where:\n\n");
  printf ("    --> \"REF_model\" and \"DUV_model\" stand for the path to the executable files\n");
  printf ("         of each model respectively.\n\n");
	printf ("    --> \"APP\" stands for the running application file. You must specify the application \n");
	printf ("         to be loaded using --load option, like in regular ArchC simulations.\n\n");        
	printf ("    --> \"arguments\" stands for the set of arguments to be passed to the running application. \n");
	printf ("         It must be specified both for the reference and duv models.\n");         
	printf ("         You may specify different arguments for the two models. Like different names for\n");         
	printf ("         output files that you want to compare after the simulation, for example.\n");         
  printf ("\n\n");
}

////////////////////////////////////////////////
// This function is called when we need
// to force termination of all three processes
// ac_verifier, ref and duv
////////////////////////////////////////////////
void ABORT(){

	cerr << "Aborting co-verification ..." << endl;

	//Deleting message queues
	if (msgctl(ref_msqid, IPC_RMID, NULL) == -1) {
		perror("msgctl");
		cerr << "Could not delete Ref model msg queue."<<endl;
	}

	if (msgctl(duv_msqid, IPC_RMID, NULL) == -1) {
		perror("msgctl");
		cerr << "Could not delete Ref model msg queue."<<endl;
	}

	if (kill(ref_pid, 15)== -1) {
		perror("kill");
		cerr << "Could not terminate Ref model process."<<endl;
	}

	if (kill(duv_pid, 15)== -1) {
		perror("kill");
		cerr << "Could not terminate DUV model process."<<endl;
	}

	exit(1);
}


/////////////////////////////////////////////
// This is the main function.
// It will handle command-line arguments and
// create message queues.
/////////////////////////////////////////////
int main(int argc, char *argv[])
{
		char *ref_argv[argc], *duv_argv[argc];
		char *ref_version_arg[2];
		char *duv_version_arg[2];
		int j,i, nargs; 
		int ref_status, duv_status;
		++argv, --argc;  /* skip over program name */


		ref_version_arg[0] =  "--version";
		ref_version_arg[1] = " 2>refout.tmp";
		duv_version_arg[0] =  "--version";
		duv_version_arg[1] = " 2>refout.tmp";

		//The user asked for help ...
		if( !argv[0] || !strcmp(argv[0], "--help") || !strcmp(argv[0], "-h")){
			DisplayHelp();
			return 0;
		}

		//We need at least 3 arguments:
		//1. Path for the reference model
		//2. Path for the duv model
		//3. Path for the running application .
		if( argc < 3 ){
			AC_ERROR("Invalid number of arguments.");
			cerr << "   Try running ac_verifier --help for more information." <<endl;
			exit(1);
		}

		if( (strncmp( argv[2], "--load_obj=", 11)) && (strncmp( argv[2], "--load=", 7)) ){
			AC_ERROR("Invalid argument! Third argument must be the running application file.");
			cerr << "   You must use --load option, just as it is done with any ArchC model." <<endl;
			cerr << "   Try running ac_verifier --help for more information." <<endl;
			exit(1);
		}



		//Creating message queues.
    if ((ref_key = ftok(argv[0], 'A')) == -1) {
        perror("ftok");
        exit(1);
    }
		dprintf("Reference model queue key: %d\n",ref_key);

    if ((duv_key = ftok(argv[1], 'A')) == -1) {
        perror("ftok");
        exit(1);
    }
		dprintf("DUV model queue key: %d\n",duv_key);

    if ((ref_msqid = msgget(ref_key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
    
    if ((duv_msqid = msgget(duv_key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }


		/* Preparing arguments to be passed for both models */

		//Storing program name and application to be runned
		ref_argv[0] = (char*) malloc( sizeof(char)*strlen(argv[0])+1 );
		duv_argv[0] = (char*) malloc( sizeof(char)*strlen(argv[1])+1 );
		ref_argv[1] = (char*) malloc( sizeof(char)*strlen(argv[2])+1 );
		duv_argv[1] = (char*) malloc( sizeof(char)*strlen(argv[2])+1 );

		ref_argv[0] = strcpy(ref_argv[0],argv[0]);
		duv_argv[0] = strcpy(duv_argv[0],argv[1]);
		ref_argv[1] = strcpy(ref_argv[1],argv[2]);
		duv_argv[1] = strcpy(duv_argv[1],argv[2]);

		dprintf("REF: %s  running %s\n", ref_argv[0], ref_argv[1]);
		dprintf("DUV: %s  running %s\n", duv_argv[0], duv_argv[1]);

		nargs=argc-3; //Number of remaining arguments to be processed

		if ( !nargs ){
				dprintf("Running app has no args\n");
				ref_argv[2] = NULL;
				duv_argv[2] = NULL;
		}				
			
		argv+=3;
		i=0;
		while( i <nargs ){ //This means that we have arguments to pass to the running app
			
			if( !strncmp( argv[i], "--ref_args=", 11) ){
				
				//Storing Reference model running app args
				dprintf("Storing Reference model running app args\n");
				ref_argv[2] = (char*)malloc(strlen(argv[i]) - 10); //1st arg is after the '=' signal
				strcpy(ref_argv[2], argv[i]+11);
				i++;
				j=3;
				dprintf("Storing  arg %s\n", ref_argv[2]);

				while( (i<nargs) && (strncmp( argv[i], "--duv_args=", 11) )){
					
					ref_argv[j] = (char*)malloc(strlen(argv[i])+1);
					strcpy(ref_argv[j], argv[i]);					
					dprintf("Storing  arg %s\n", ref_argv[j]);
					i++;
					j++;
				}
				dprintf("Reference running application will receive %d arguments\n", j-2);
				ref_argv[j] = NULL;
			}

			if( !strncmp( argv[i], "--duv_args=", 11) ){  //There are duv model args
				
				//Storing DUV model running app args
				dprintf("Storing DUV model running app args\n");
				duv_argv[2] = (char*)malloc(strlen(argv[i]) - 10); //1st arg is after the '=' signal
				strcpy(duv_argv[2], argv[i]+11);
				i++;
				j=3;
				dprintf("Storing  arg %s\n", duv_argv[2]);
				
				while( (i<nargs) && (strncmp( argv[i], "--ref_args=", 11) )){
					duv_argv[j] = (char*)malloc(strlen(argv[i])+1);
					strcpy(duv_argv[j], argv[i]);					
					dprintf("Storing  arg %s\n", duv_argv[j]);
					i++;
					j++;
				}
				dprintf("DUV running application will receive %d arguments\n", j-2);
				duv_argv[j] = NULL;
			}
		}		

		//Checking if both models were generated with the -v version.
		//Running  models with --version option.
		if( !(ref_pid = fork()) ){
			execv(ref_argv[0], ref_version_arg);
		}
		else{
			if( !(duv_pid = fork()) ){
				execv(duv_argv[0], duv_version_arg);
			}
			else{
				//Waiting for the ref model to terminate
				waitpid(ref_pid,&ref_status,0);

				CheckOptions( REFERENCE_MODEL );

				//Waiting for the duv model to terminate
				waitpid(duv_pid,&duv_status,0);

				CheckOptions( DUV_MODEL );
			}
		}


		//Now everything was checked. Let's start the co-verification process
		//

		/* Creating process for both models */
		if( !(ref_pid = fork()) ){
			execv(ref_argv[0], ref_argv);
		}
		else{
			if( !(duv_pid = fork()) ){
				execv(duv_argv[0], duv_argv);
			}
			else{

				//Initializing both device lists
				dprintf("REFERENCE MODEL QUEUE (%d) INITIALIZATION:\n\n",ref_msqid);
				ListInit(&ref_devlist, ref_msqid, (struct start_msgbuf*)&ref_sbuf);
				dprintf("DUV MODEL QUEUE (%d) INITIALIZATION:\n\n",duv_msqid);
				ListInit(&duv_devlist, duv_msqid, (struct start_msgbuf*)&duv_sbuf);

				//Device list of both models (ref and duv) must have the same devices (number and names)
				CheckListConsistency();

				DoItOntheFly();

				waitpid(ref_pid,&ref_status,0);

				//TODO:Padronizar saida de erro
				if(!WIFEXITED(ref_status)){
					cerr << "Reference model returned with error"  <<endl;
					if(WIFSIGNALED(ref_status))
						cerr << "Signal : " << WTERMSIG(ref_status)<<endl;
				}

				waitpid(duv_pid,&duv_status,0);

				if(!WIFEXITED(duv_status)){
					cerr << "DUV  model returned with error" << endl;
					if(WIFSIGNALED(duv_status))
						cerr << "Signal : " << WTERMSIG(duv_status)<<endl;
				}
			}
		}

		//Finalizing co-verification process
		FinishIt();

		//Run the co-verification....
    printf("Co-verification finished.\n");

		//Deleting message queues
    if (msgctl(ref_msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    if (msgctl(duv_msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    return 0;
}


///////////////////////////////////////////
// Initialize the device list
//
// This function will get the first 
// messages (see protocol bellow) incoming from a model and 
// initialize its device list.
///////////////////////////////////////////

//////////////////////////////////////////////////////////
// ArchC co-verification protocol:
//
// 1st MSG          : Number of devices being checked (N)
// 2nd ... (N+1) MSG: Name of each device
// (N+2) ...        : Update logs
//
// Type 1 are control messages
// Type 2 are devices names do be verified
// Type 3 ...  are types created for each device to be verified
//////////////////////////////////////////////////////////

void ListInit( struct dev_list** model_devlist, int msqid, start_msgbuf *sbuf ){

	struct dev_msgbuf dbuf;
	struct dev_list *pdevlist;
	int i;


	//Processing first message...
	if (msgrcv(msqid, sbuf, sizeof(struct start_msgbuf), 0, 0) == -1) {
		perror("msgrcv");
		ABORT();
	}
	
	//First message must tell the number of devices to be supervised
	if( sbuf->mtype != 1 ){
		AC_ERROR("Invalid initialization message: expecting type 1 (control), got type " << sbuf->mtype );
		ABORT();
	}
		
	dprintf("-->Number of devices: %d\n",sbuf->ndevice);

	//2nd ... (N+1) messages must tell the name of the devices to be supervised
	for( i=0; i< sbuf->ndevice;i++){
		if (msgrcv(msqid, &dbuf, sizeof(dbuf), 0, 0) == -1) {
			perror("msgrcv");
			ABORT();
		}

		if( dbuf.mtype != 2 ){
			AC_ERROR("Invalid initialization message: expecting type 2 (device), got type " << dbuf.mtype);
			ABORT();
		}

		//Appending to the ref model device list.
		pdevlist = new (struct dev_list);
		pdevlist->dbuf = dbuf;
		pdevlist->next = *model_devlist;
		*model_devlist = pdevlist;

		dprintf("-->Device (%d) name: %s\n", i+1,pdevlist->dbuf.name);
	}

}

////////////////////////////////////////////
// This function checks the consistency of//
// device lists. Both ref and duv must be //
// verifying the same storage devices     //
////////////////////////////////////////////
void CheckListConsistency( ){

	struct dev_list *p1;
	struct dev_list *p2;
	struct dev_loglist *pll;
	unsigned next_type = 2 + ref_sbuf.ndevice;

	if( !ref_devlist || !duv_devlist ){
		AC_ERROR("Uninitialized device lists.");
		ABORT();
	}

	//First check the number of devices
	if( ref_sbuf.ndevice != duv_sbuf.ndevice ){
		AC_ERROR("Uninitialized device lists.");
		ABORT();
	}

	//Now check device names.
	for (p1 = ref_devlist; p1; p1=p1->next ){

		for( p2 = duv_devlist; p2; p2=p2->next) {
			if( !strcmp(p1->dbuf.name,p2->dbuf.name))
				break;
		}
		if( !p2 ){
			//Didn't find p1 storage in duv's list
			AC_ERROR("Device lists are not consistent. DUV model does not have a "<< p1->dbuf.name <<" device.");
			ABORT();
		}

		//Everything is OK for this device, so create its log list
		pll = new (struct dev_loglist);
		pll->type = next_type--;  //Device list was created in inverted order, so first type to be processed is the last sent by the models
		strcpy(pll->name, p1->dbuf.name);
		pll->next = loglist;
		loglist = pll;
		dprintf("Adding device list for %s with type %d\n",pll->name,pll->type);
	}
		
	dprintf("CheckListConsistency passed successfully.\n");
	
}


//////////////////////////////////////
// The main co-verification loop
//////////////////////////////////////
void DoItOntheFly(){

	struct log_msgbuf lbuf;
	bool ref_finished=0, duv_finished=0;
	int ref_status, duv_status;
	int i;

	//Keep "listening" to the models and comparing update logs
	while( !ref_finished || !duv_finished ){

		if( !ref_finished ){
			
			if (msgctl(ref_msqid, IPC_STAT, &ref_info) == -1) {
        perror("msgctl");
        ABORT();
			}
			
			ddprintf("REF QUEUE INFO:\nr-w: %03o, cbytes= %1u, qnum= %1u, qbytes = %1u\n",
							ref_info.msg_perm.mode & 0777, ref_info.msg_cbytes, ref_info.msg_qnum, ref_info.msg_qbytes);
			
			i = ref_info.msg_qnum;

			//If there is nothing on the queue, the child may have exited. Check it
			if( !i){
				waitpid(ref_pid,&ref_status,WNOHANG);
				if(WIFEXITED(ref_status)){
					dprintf("Matching logs ... Empty queue! Reference model exited!");
					if(WIFSIGNALED(ref_status))
						dprintf("Signal :%d ", WTERMSIG(ref_status));
					ref_finished = 1;
				}
			}

			//Consuming ref queue msgs
			while (i) {

				//Listening to Reference model
				if (msgrcv(ref_msqid, &lbuf, sizeof(struct log_msgbuf), 0, 0) == -1) {
					perror("msgrcv");
					ABORT();
				}
				// 		cerr << "Log received from ref: ";
				// 		cerr << lbuf.log <<endl;
				if( lbuf.log.time == -1 ){
					ref_finished = 1;
					dprintf("Reference model has finished\n");
				}
				else{
					//Append to the device list
					AddLog(lbuf.mtype, lbuf.log, 0 );
				}
				i--;
			}
		}

		if(!duv_finished){

			//Gathering info about ref and duv queues
			if (msgctl(duv_msqid, IPC_STAT, &duv_info) == -1) {
        perror("msgctl");
        ABORT();
			}
		
			ddprintf("DUV QUEUE INFO:\nr-w: %03o, cbytes= %1u, qnum= %1u, qbytes = %1u\n",
							duv_info.msg_perm.mode & 0777, duv_info.msg_cbytes, duv_info.msg_qnum, duv_info.msg_qbytes);
			
			i = duv_info.msg_qnum;
			//If there is nothing on the queue, the child may have exited. Check it
			if( !i){
				waitpid(duv_pid,&duv_status,WNOHANG);
				
				if(WIFEXITED(duv_status)){
					dprintf("Matching logs ... Empty queue! DUV  model exited!");
					if(WIFSIGNALED(duv_status))
						dprintf("Signal : ", WTERMSIG(duv_status));
					
					duv_finished =1;
				}
			}

			//Consuming duv queue msgs
			while (i) {

				//Listening to DUV model
				if (msgrcv(duv_msqid, &lbuf, sizeof(struct log_msgbuf), 0, 0) == -1) {
					perror("msgrcv");
					ABORT();
				}
				// 		cerr << "Log received from DUV: ";
				// 		cerr << lbuf.log <<endl;
				if( lbuf.log.time == -1 ){
					duv_finished = 1;
					dprintf("DUV model has finished\n");
				}
				else{
					
					//Append to the device list
					AddLog(lbuf.mtype, lbuf.log, 1 );
				}
				i--;
			}
		}

		MatchLogs();
		
	}
}

// //////////////////////////////////////
// // The main co-verification loop
// //////////////////////////////////////
// void DoIt(){

// 	struct log_msgbuf lbuf;
// 	int received =0;
// 	int i;
// 	bool ref_finished;

// 	//Keep "listening" to the models and comparing update logs
// 	for(;;){

// 		//Gathering info about ref and duv queues
//     if (msgctl(duv_msqid, IPC_STAT, &duv_info) == -1) {
//         perror("msgctl");
//         ABORT();
//     }
		
//     if (msgctl(ref_msqid, IPC_STAT, &ref_info) == -1) {
//         perror("msgctl");
//         ABORT();
//     }
		
// 		dprintf("REF QUEUE INFO:\nr-w: %03o, cbytes= %1u, qnum= %1u, qbytes = %1u\n",
// 						ref_info.msg_perm.mode & 0777, ref_info.msg_cbytes, ref_info.msg_qnum, ref_info.msg_qbytes);

// 		dprintf("DUV QUEUE INFO:\nr-w: %03o, cbytes= %1u, qnum= %1u, qbytes = %1u\n",
// 						duv_info.msg_perm.mode & 0777, duv_info.msg_cbytes, duv_info.msg_qnum, duv_info.msg_qbytes);

// 		if( ref_info.msg_cbytes >= 0.9*ref_info.msg_qbytes ){
			
// 			i = ref_info.msg_cbytes;
			
// 			cerr << "Log received from ref: "<< i <<" messages"<<endl;
// 			while(i){
// 				//Listening to Reference model
// 				if (msgrcv(ref_msqid, &lbuf, sizeof(struct log_msgbuf), 0, 0) == -1) {
// 					perror("msgrcv");
// 					ABORT();
// 				}

// 				if( lbuf.log.time == -1 ){
// 					ref_finished = 1;
// 				 cerr << "Reference model has finished ";
// 				//						cerr << lbuf.log <<endl;
// 				}
// 				//Append to the device list
// 				AddLog(lbuf.mtype, lbuf.log, 0 );
// 				i--;
// 			}
// 			received = 1;
// 		}
		
// 		if( duv_info.msg_cbytes >= 0.9* duv_info.msg_qbytes ){
			
// 			i = duv_info.msg_cbytes;
			
// 			cerr << "Log received from duv: "<< i <<" messages"<<endl;
// 			while(i){
// 				//Listening to DUV model
// 				if (msgrcv(duv_msqid, &lbuf, sizeof(struct log_msgbuf), 0, 0) == -1) {
// 					perror("msgrcv");
// 					ABORT();
// 				}
// 				//	cerr << "Log received from DUV: ";
// 				//	cerr << lbuf.log <<endl;
				
// 				//Append to the device list
// 				AddLog(lbuf.mtype, lbuf.log, 1 );
// 				i--;
// 			}
// 			received = 1;
// 		}
		
// 		//Check consistency
// 		if( received ){
// 			MatchLogs();
// 			received =0;
// 		}
		
// 	}
// }

//////////////////////////////////////////
// Add an update log to the corresponding
// list.
// device = 0 indicates operation on ref log
// device = 1 indicates operation on duv log
//////////////////////////////////////////
void AddLog( long type, change_log log, int device ){

	struct dev_loglist *pllist;


	for( pllist = loglist; pllist; pllist=pllist->next){

		if( type == pllist->type ){

			if( device )
				pllist->duv_log.push_back(log);
			else
				pllist->ref_log.push_back(log);

			break;
		}
	}

	if( !pllist )
		AC_ERROR("Invalid type ("<<type<<") in log message. Update ignored");
	
}


/////////////////////////////////////////
// Match reference and duv update logs
/////////////////////////////////////////
void MatchLogs( ){

	struct dev_loglist *pllist;
  log_list::iterator refitor;
  log_list::iterator duvitor;
	bool flag,error=0;

	for( pllist = loglist; pllist; pllist=pllist->next){

		if( pllist->duv_log.size() &&  pllist->ref_log.size()){

			pllist->duv_log.sort();
			pllist->ref_log.sort();
			ddprintf("BEFORE MATCHING...\n");
			ddprintf("Device %s -> Log size:  %d ref and %d duv\n", pllist->name, pllist->ref_log.size(), pllist->duv_log.size());
			
#ifdef DDEBUG
			ChangeDump( pllist);
#endif
			refitor = pllist->ref_log.begin();
			while( refitor != pllist->ref_log.end() ){
				
				duvitor = pllist->duv_log.begin();
				flag = false;
				
				while( /*(refitor->time == duvitor->time) &&*/ (duvitor != pllist->duv_log.end())){
					if( refitor->addr == duvitor->addr &&
							refitor->value == duvitor->value ){
						
						refitor = pllist->ref_log.erase( refitor );
						duvitor = pllist->duv_log.erase( duvitor );
						flag = true;
						break;
					}
					duvitor++;
				}
				//When we had a match, the iterator was incremented by the erase method
				if(!flag)
					refitor++;
			}

			ddprintf("AFTER MATCHING...\n");
			ddprintf("Device %s -> Log size:  %d ref and %d duv\n", pllist->name, pllist->ref_log.size(), pllist->duv_log.size());
			//Test if we already had too many erros (unmatched logs)
			if( pllist->ref_log.size() >= AC_MAX_UNMATCHED || pllist->duv_log.size() >= AC_MAX_UNMATCHED )
				error = 1;
		}
	}
	if(error){
		dprintf("Too many erros founded. Aborting ...\n");
		FinishIt();
		ABORT();
	}
}


////////////////////////////////////////////////////
// Final Check to the reference and duv update logs
////////////////////////////////////////////////////
void FinishIt(){

	struct dev_loglist *pllist;

	for( pllist = loglist; pllist; pllist=pllist->next){

		if( pllist->duv_log.size() ||  pllist->ref_log.size()){
	
			AC_ERROR("Co-verification FAILED. Reference and DUV models have inconsistent update logs for device "<< pllist->name);
			cerr <<endl;
			ChangeDump( pllist );
		}
	}
}

/////////////////////////////////////////////////////////
// Dump logs on the err output. Used when co-verification
// finds inconsistencies between two models
/////////////////////////////////////////////////////////
void ChangeDump( struct dev_loglist *pllist ) {

  log_list::iterator itor;
	ofstream covfile;
    
	covfile.open("coverif.out");
  if(  pllist->ref_log.size() ){
		covfile <<endl << endl;
    covfile << "**************** ArchC Change log *****************\n";
    covfile << "* Reference Model         Device: "<< pllist->name << endl;
    covfile << "***************************************************\n";
    covfile << "*        Address         Value          Time      *\n";
    covfile << "***************************************************\n";
      
    for( itor = pllist->ref_log.begin(); itor != pllist->ref_log.end(); itor++)  
      covfile << "*  " << *itor << "     *" << endl;
      
    covfile << "***************************************************\n";
  }

  if(  pllist->duv_log.size() ){
		covfile <<endl << endl;
    covfile << "**************** ArchC Change log *****************\n";
    covfile << "* DUV Model         Device: "<< pllist->name << endl;
    covfile << "***************************************************\n";
    covfile << "*        Address         Value          Time      *\n";
    covfile << "***************************************************\n";
      
    for( itor = pllist->duv_log.begin(); itor != pllist->duv_log.end(); itor++)  
      covfile << "*  " << *itor << "     *" << endl;
      
    covfile << "***************************************************\n";
  }

	covfile.close();
}

/////////////////////////////////////////////////////////
// Check if both models were generated with the correct 
// command-line options. For now, the only obligatory
// option is -v
/////////////////////////////////////////////////////////
void CheckOptions( int model ) {

	ifstream input;
  string read;
	char *filename;
	char* p;

	if(model == REFERENCE_MODEL)
		filename = "refout.tmp";
	else
		filename = "duvout.tmp";

	input.open(filename);

  if(!input){
    AC_ERROR("Could not open input file: " << filename);
    AC_ERROR("Command-line option checker aborted.");
    exit(1);
  }
		
	getline(input, read);
	p = strstr( read.c_str(), "(");
	if(p ){
		
		if( strstr( p, "-v") ||strstr( p, "-v)") )
			return;
	}

	//If we got here, the command-line options were incorrect.	
	if(model == REFERENCE_MODEL)
		AC_ERROR("Reference Model is not prepared for co-verification.\n");
	else
		AC_ERROR("DUV Model is not prepared for co-verification.\n");

	AC_MSG("   Run acpp with the -v option for generating models on co-verification mode");
	exit(1);
				
}
