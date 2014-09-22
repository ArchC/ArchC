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

  psc_clock.cpp -- The psc_clock class.

  Original Author: Felipe Klein <klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#include "psc_clock.h"

// ----------------------------------------------------------------------------
// Specialized versions of the virtual methods (inherited from psc_objinfo):
//  - on_bits
//  - bit_diff
//  - update_toggle_count
// ----------------------------------------------------------------------------
unsigned short int psc_clock::on_bits( const sc_bit & v )
{
	return( bool(v) );
}

inline sc_bit psc_clock::bit_diff(const sc_bit & v1, const sc_bit & v2)
{
	bool _v1 = (bool)v1, _v2 = (bool)v2;

	if ( psc_clock::must_update() && (_v1 ^ _v2) )
		psc_objinfo<1, sc_bit>::inc_bit_toggle_count( 0, 1 );
	
	return( sc_bit(_v1 ^_v2) );
}

void psc_clock::update_toggle_count(const sc_bit & cur_val, const sc_bit & new_val)
{
	psc_objinfo<1, sc_bit>::update_toggle_count(cur_val, new_val);
}

uint64 psc_clock::uint64value( const sc_bit & v ) const
{
	uint64 value = v.to_bool();
	return( value );
}

void psc_clock::register_port( sc_port_base &port_, const char* if_typename_)
{
	// increment the number of connected ports
	m_n_connec++;

	// increments the fanout for this net
	// if the port type is 'sc_in'
	string ptype = port_.kind();
	if ( ptype == "sc_in" )
		this->inc_fanout();

	// invoke the overriden method
	sc_clock::register_port( port_, if_typename_ );
}

void psc_clock::end_of_simulation()
{
	this->finish_pending();
}

// constructors

psc_clock::psc_clock()
: sc_clock(),
	psc_objinfo<1, sc_bit>("psc_clock")
{
   init_clock();
}

psc_clock::psc_clock( const char * name_ )
: sc_clock( name_ ),
	psc_objinfo<1, sc_bit>("psc_clock")
{
   init_clock();
}

psc_clock::psc_clock( const char * name_,
		    const sc_time& period_,
		    double         duty_cycle_,
		    const sc_time& start_time_,
		    bool           posedge_first_ )
: sc_clock( name_, period_, duty_cycle_, start_time_, posedge_first_ ),
	psc_objinfo<1, sc_bit>("psc_clock")
{
   init_clock();
}

psc_clock::psc_clock( const char * name_,
		    double         period_v_,
		    sc_time_unit   period_tu_,
		    double         duty_cycle_ )
: sc_clock( name_, period_v_, period_tu_, duty_cycle_ ),
	psc_objinfo<1, sc_bit>("psc_clock")
{
   init_clock();
}

psc_clock::psc_clock( const char * name_,
		    double         period_v_,
		    sc_time_unit   period_tu_,
		    double         duty_cycle_,
		    double         start_time_v_,
		    sc_time_unit   start_time_tu_,
		    bool           posedge_first_ )
: sc_clock( name_, period_v_, period_tu_, duty_cycle_, start_time_v_,
      start_time_tu_, posedge_first_ ),
	psc_objinfo<1, sc_bit>("psc_clock")
{
   init_clock();
}

// for backward compatibility with 1.0
psc_clock::psc_clock( const char * name_,
		    double         period_,      // in default time units
		    double         duty_cycle_,
		    double         start_time_,  // in default time units
		    bool           posedge_first_ )
: sc_clock( name_, period_, duty_cycle_, start_time_, posedge_first_ ),
	psc_objinfo<1, sc_bit>("psc_clock")
{
   init_clock();
}


// destructor (do nothing)

psc_clock::~psc_clock()
{}

void psc_clock::posedge_count()
{
   m_clk_val = true;
	update_toggle_count( sc_bit(false), sc_bit(true) );
}

void psc_clock::negedge_count()
{
   m_clk_val = false;
	update_toggle_count( sc_bit(true), sc_bit(false) );
}


void psc_clock::init_clock()
{
	// set the alias for the clock
   this->set_alias( this->name() );

	std::string gen_base;

	// options for the processes counting the
	// clock switching
	sc_spawn_options posedge_count_options;
	sc_spawn_options negedge_count_options;

	// create the posedge_count process
	posedge_count_options.spawn_method();
	posedge_count_options.dont_initialize();
	posedge_count_options.set_sensitivity( 
			& this->m_next_posedge_event 
			);
	gen_base = basename();
	gen_base += "_posedge_count";
	sc_spawn( sc_clock_posedge_count_callback(this),
			sc_gen_unique_name(gen_base.c_str()),
			& posedge_count_options
			);

	// create the negedge_count process
	negedge_count_options.spawn_method();
	negedge_count_options.dont_initialize();
	negedge_count_options.set_sensitivity(
			& this->m_next_negedge_event
			);
	gen_base = basename();
	gen_base += "_negedge_count";
	sc_spawn( sc_clock_negedge_count_callback(this),
			sc_gen_unique_name(gen_base.c_str()),
			& negedge_count_options
			);
}

