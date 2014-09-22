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

  psc_signal_bool.h -- The psc_signal_bool channel class.

  Original Author: Felipe Klein <klein@ic.unicamp.br>, IC/UNICAMP, 2006.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_SIGNAL_BOOL_H
#define PSC_SIGNAL_BOOL_H

#include <systemc.h>

#include "psc_objinfo.h"
//#include "base/psc_objinfo.h"
#include "psc_bit.h"

using namespace psc_dt;

// ----------------------------------------------------------------------------
//  boolEMPLATE: psc_signal_bool
//
//  The psc_signal_bool class.
// ----------------------------------------------------------------------------
class psc_signal_bool : public sc_signal<bool>, public psc_objinfo_if
{
public:
	
	// constructors
	
	psc_signal_bool()
		{
			m_n_connec = 0;
			init_info();
		}

	explicit psc_signal_bool( const char *name_ )
		: sc_signal<bool>( name_ )
		{
			m_n_connec = 0;
			init_info(name_);
		}

   void set_alias( const char *new_alias );

	void  set_wire_load( const double load );

	double get_wire_load() const;

	void set_net_load( const double load );

	void add_to_net_load( const double load );

	double get_net_load() const;

	void set_net_delay( const double delay );

	double get_net_delay() const;

	void dont_update( bool dont = true );

	void
	register_port( sc_port_base &port_, const char* if_typename_);

	void end_of_simulation();

	inline
	int get_num_conn() const
	{ return( m_n_connec ); }

	inline
	int get_fanout() const
	{ return( m_sig_val.get_fanout() ); }

   inline
   unsigned int get_toggle_count() const
	{ return( m_sig_val.get_toggle_count() ); }

   inline
   string & get_id()
	{ 
		return( m_sig_val.get_id() ); 
	}

   inline
   double get_toggle_rate_u() const
	{ return( m_sig_val.get_toggle_rate_u() ); }

   inline
   double get_toggle_rate() const
	{ return( m_sig_val.get_toggle_rate() ); }

   inline
   vector<double> get_split_toggle_rate() const
	{ return( m_sig_val.get_split_toggle_rate() ); }

   double get_sp0() const;		// static probaility at 0
   double get_sp1() const;		// static probaility at 1
	void end_of_elaboration();

   // operators

   psc_signal_bool& operator = ( bool a )
	{ write( a ); return *this; }

   psc_signal_bool& operator = ( const psc_signal_bool& a )
	{ write( a.read() ); return *this; }

	inline
	void write( const bool & value_ )
	{
		m_sig_val = value_;
		sc_signal<bool>::write( value_ );
	}

protected:

	// number of connected ports to this signal
	int m_n_connec;
	psc_bit m_sig_val;
	
private:
	void init_info(const char *name_ = NULL);
};

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

#endif
