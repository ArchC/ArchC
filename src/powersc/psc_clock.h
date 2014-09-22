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

  psc_clock.h -- The psc_clock class.

  Original Author: Felipe Klein <klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_CLOCK_H
#define PSC_CLOCK_H

// the line below is to ensure that the sc_spawn
// can be used
#define	SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>

#include "psc_objinfo.h"
//#include "base/psc_objinfo.h"
#include "psc_bit.h"

using psc_dt::psc_bit;

class psc_clock : public sc_clock, public psc_objinfo<1, sc_bit>
{
	// support methods

   unsigned short int on_bits( const sc_bit & v );
   inline sc_bit bit_diff( const sc_bit & v1, const sc_bit & v2 ); // sets on '1' changed bits between v1 and v2
   void update_toggle_count( const sc_bit & cur_val, const sc_bit & new_val );
	uint64 uint64value( const sc_bit & v ) const;

public:

	friend class sc_clock_posedge_count_callback;
	friend class sc_clock_negedge_count_callback;

	void register_port( sc_port_base &port_, const char* if_typename_);
	void end_of_simulation();

   SC_HAS_PROCESS( psc_clock );

   // constructors

   psc_clock();


   explicit psc_clock( const char * name_ );

   psc_clock( const char * name_,
            const sc_time& period_,
            double         duty_cycle_ = 0.5,
            const sc_time& start_time_ = SC_ZERO_TIME,
            bool           posedge_first_ = true );

   psc_clock( const char * name_,
            double         period_v_,
            sc_time_unit   period_tu_,
            double         duty_cycle_ = 0.5 );

   psc_clock( const char * name_,
            double         period_v_,
            sc_time_unit   period_tu_,
            double         duty_cycle_,
            double         start_time_v_,
            sc_time_unit   start_time_tu_,
            bool           posedge_first_ = true );

   // for backward compatibility with 1.0
   psc_clock( const char * name_,
            double         period_,            // in default time units
            double         duty_cycle_ = 0.5,
            double         start_time_ = 0.0,  // in default time units
            bool           posedge_first_ = true );


   ~psc_clock();

protected:

   sc_bit m_clk_val;

	// number of connected ports to this signal
	int m_n_connec;
   
   void posedge_count();
   void negedge_count();

   void init_clock();
   
private:
   
   // disabled
   psc_clock( const psc_clock& );
   psc_clock& operator = ( const psc_clock& );
   
};

class sc_clock_posedge_count_callback {
public:
    sc_clock_posedge_count_callback(psc_clock* target_p) { m_target_p = target_p; }
    inline void operator () () { m_target_p->posedge_count(); }
  protected:
    psc_clock* m_target_p;
};

class sc_clock_negedge_count_callback {
  public:
    sc_clock_negedge_count_callback(psc_clock* target_p) { m_target_p = target_p; }
    inline void operator () () { m_target_p->negedge_count(); }
  protected:
    psc_clock* m_target_p;
};


#endif
