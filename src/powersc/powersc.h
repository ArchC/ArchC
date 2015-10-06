/*****************************************************************************
                             PowerSC Library

                           Copyright 2004-2006
                       Computer Systems Laboratory
                          All Rights Reserved

 PERMISSION IS GRANTED TO USE, COPY AND REDISTRIBUTE THIS SOFTWARE FOR
 NONCOMMERCIAL EDUCATION AND RESEARCH PURPOSES, SO LONG AS NO FEE IS CHARGED,
 AND SO LONG AS THE COPYRIGHT NOTICE ABOVE, THIS GRANT OF PERMISSION, AND THE
 DISCLAIMER BELOW APPEAR IN ALL COPIES MADE; AND SO LONG AS THE NAME OF THE
 COMPUTER SYSTEMS LABORATORY IS NOT USED IN ANY ADVERTISING OR PUBLICITY
 PERTAINING TO THE USE OR DISTRIBUTION OF THIS SOFTWARE WITHOUT SPECIFIC,
 WRITTEN PRIOR AUTHORIZATION. PERMISSION TO MODIFY OR OTHERWISE CREATE
 DERIVATIVE WORKS OF THIS SOFTWARE IS NOT GRANTED.

 THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION AS TO ITS FITNESS
 FOR ANY PURPOSE, AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR
 IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE STATE UNIVERSITY
 OF CAMPINAS, THE INSTITUTE OF COMPUTING AND THE COMPUTER SYSTEMS LABORATORY
 SHALL NOT BE LIABLE FOR ANY DAMAGES, INCLUDING SPECIAL, INDIRECT, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM ARISING OUT OF OR IN
 CONNECTION WITH THE USE OF THE SOFTWARE, EVEN IF IT HAS BEEN OR IS HEREAFTER
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *****************************************************************************/

/*****************************************************************************

  <powersc.h> -- Include this file to use the PowerSC library.

  Original Author: Felipe Klein <klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef POWERSC_H
#define POWERSC_H

// make the deprecated class sc_string
// be the standard library string class
#define SC_USE_STD_STRING

#include <systemc.h>

#include "config.h"
#include "debug_power.h"

#include "psc_uint.h"
#include "psc_int.h"
#include "psc_bit.h"
#include "psc_logic.h"
#include "psc_bv.h"
#include "psc_lv.h"
#include "psc_signal.h"
#include "psc_signal_bool.h"
#include "psc_clock.h"

//#include "base/psc_sampler.h"
//#include "base/psc_techlib.h"
//#include "base/psc_macromodel.h"

//#include "utils/psc_randgen.h"
//#include "utils/psc_misc.h"
//#include "utils/psc_interp.h"


#include "psc_sampler.h"
#include "psc_techlib.h"
#include "psc_macromodel.h"

#include "psc_randgen.h"
#include "psc_misc.h"
#include "psc_interp.h"

using psc_dt::psc_uint;
using psc_dt::psc_int;
using psc_dt::psc_bit;
using psc_dt::psc_logic;
using psc_dt::psc_bv;
using psc_dt::psc_lv;

using namespace psc_util;
using namespace psc_power_base;

#ifdef POWER_SIM

#define	sc_uint psc_uint
#define	sc_int psc_int
#define	sc_signal psc_signal
#define	sc_bit psc_bit
#define	sc_logic psc_logic
#define	sc_bv psc_bv
#define  sc_lv psc_lv
#define	sc_clock psc_clock


#ifdef PSC_DONT_IGNORE_COND
#define if(x) if (psc_objinfo_base::repository.conditional_hit( "(" #x ")" ":" __FILE__ ":" , __LINE__ , (x) ))
#endif

#define PSC_TECHLIB_INIT \
	psc_objinfo_base::techlib.set_voltage( LibOperCond::voltage ); \
	psc_objinfo_base::techlib.set_volt_unit( LibUnits::voltage ); \
	psc_objinfo_base::techlib.set_cap_unit( LibUnits::capacitive ); \
	psc_objinfo_base::techlib.set_time_unit( LibUnits::time ); \
	psc_objinfo_base::techlib.set_leak_unit( LibUnits::leakage_power ); \
	psc_objinfo_base::techlib.set_dynpwr_unit( LibUnits::dynamic_power ); \
	LibUnits::use_lib_time()

#define PSC_TECHLIB_INFO \
	psc_objinfo_base::techlib.print()

#define PSC_INSERT_CELL(info) \
	psc_objinfo_base::cells.add(info)

#define PSC_BLOCK( code ) code

#define PSC_WRITE_CSV_FILE( file ) \
   psc_objinfo_base::repository.write_entries_to_csv_file( file );

#define PSC_SET_WLOAD( obj, load ) \
	obj.set_wire_load( load )

#define PSC_SET_NET_LOAD( obj, load ) \
	obj.set_net_load( load )

#define PSC_SET_NET_DELAY( obj, delay ) \
	obj.set_net_delay( delay )

#define PSC_REPORT_SWITCHING_ACTIVITY \
	psc_objinfo_base::repository.print_entries()

#define PSC_REPORT_SWITCHING_POWER \
	psc_objinfo_base::repository.switching_power_report()

#define PSC_REPORT_POWER \
	psc_objinfo_base::cells.power_report()
	//psc_objinfo_base::cells.power_report("pw_out.txt")

#define PSC_REPORT_CELLS_INFO \
	psc_objinfo_base::cells.print_cells()
	
#define PSC_OBJ_ALIAS(obj, str) obj.set_alias(str)
#define PSC_IGNORE(obj) obj.dont_update()
#define PSC_FULL_SAMPLING psc_objinfo_base::repository.full_sampling()
#define PSC_STATUS_QUO(interval, stream) \
   psc_objinfo_base::repository.enable_status_quo( interval, stream )
   
#define PSC_SAMPLES_PRED_RANGE(min, max) \
   psc_objinfo_base::repository.set_samples_pred_range( min, max )

#define PSC_SAMPLES_MON_RANGE(min, max) \
   psc_objinfo_base::repository.set_samples_mon_range( min, max )

#define PSC_NUM_FIRST_SAMPLES(num) \
   psc_objinfo_base::repository.set_num_first_samples( num )

#define PSC_SAMPLING_THRESHOLD(val) \
   psc_objinfo_base::repository.set_sampling_threshold( val )

#else

#define	psc_signal_bool	sc_signal<bool>

#define PSC_TECHLIB_INIT
#define PSC_TECHLIB_INFO
#define PSC_INSERT_CELL(info)
#define PSC_BLOCK( code )
#define PSC_WRITE_CSV_FILE( file )
#define PSC_SET_WLOAD( obj, load )
#define PSC_SET_NET_LOAD( obj, load )
#define PSC_SET_NET_DELAY( obj, delay )
#define PSC_REPORT_SWITCHING_ACTIVITY
#define PSC_REPORT_SWITCHING_POWER
#define PSC_REPORT_POWER
#define PSC_REPORT_CELLS_INFO
#define PSC_OBJ_ALIAS(obj, str) 
#define PSC_IGNORE(obj)
#define PSC_FULL_SAMPLING
#define PSC_STATUS_QUO(interval) 

#define PSC_SAMPLES_PRED_RANGE(min, max)
#define PSC_SAMPLES_MON_RANGE(min, max)
#define PSC_NUM_FIRST_SAMPLES(num)
#define PSC_SAMPLING_THRESHOLD(val)

#endif

#endif
