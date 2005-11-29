//////////////////////////////////////////////////////////
/*!\file ac_msgbuf.H
  \brief The ArchC structures for IPC

  This class contains  structures for IPC during co-verification
*/
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// ArchC co-verification protocol:
//
// 1th MSG          : Number of devices being checked (N)
// 2nd ... (N+1) MSG: Names of each devices
// (N+2) ...        : Update logs
//
// That's why we have three different structures for 
// messages
//////////////////////////////////////////////////////////
#ifndef _AC_MSGBUF_H
#define _AC_MSGBUF_H

#include "ac_log.H"

#define AC_ERROR( msg )     cerr<< "ArchC ERROR: " << msg  <<'\n'

struct start_msgbuf {
	long mtype;
	int  ndevice;
};

struct dev_msgbuf {
	long mtype;
	char name[256];
};

struct log_msgbuf {
	long mtype;
	change_log log;
};

struct dev_list {

	struct dev_msgbuf dbuf;
	struct dev_list *next;
};

struct dev_loglist{
	long type;
	char name[256];
	log_list ref_log;
	log_list duv_log;
	struct dev_loglist *next;
};

#endif
