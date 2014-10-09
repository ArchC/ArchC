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
#include "psc_signal_bool.h"

void psc_signal_bool::init_info(const char *name_)
{
	::std::string cur_str, new_str;

   // to guarantee that the ID have been set
   this->m_sig_val.force_init_info();
//   this->m_new_val.force_init_info();

	if (name_) {
		cur_str = name_;
		//cur_str += ".";
		new_str = name_;
		new_str += ".";
	} else {
      cur_str = this->name();
      //cur_str += ".";
      new_str = this->name();
      new_str += ".";
   }
	
	//cur_str += "m_sig_val";
//	new_str += "m_new_val";
	
	this->m_sig_val.set_alias(cur_str);
	this->m_sig_val.set_is_net();

//	this->m_new_val.set_alias(new_str);
//	this->m_new_val.dont_update();
}


void psc_signal_bool::set_alias( const char *new_alias )
{
#ifdef DEBUG_POWER_L3
   cerr << "\t[psc_signal]: Setting alias \"" << new_alias << "\"" << endl;
#endif

   init_info( new_alias );
}


void  psc_signal_bool::set_wire_load( const double load )
{
	this->m_sig_val.set_wire_load( load );
}

double psc_signal_bool::get_wire_load() const
{
	return( this->m_sig_val.get_wire_load() );
}

void psc_signal_bool::set_net_load( const double load )
{
	this->m_sig_val.set_net_load( load );
}

void psc_signal_bool::add_to_net_load( const double load )
{
	this->m_sig_val.add_to_net_load( load );
}

double psc_signal_bool::get_net_load() const
{
	return( this->m_sig_val.get_net_load() );
}

void psc_signal_bool::set_net_delay( const double delay )
{
	this->m_sig_val.set_net_delay( delay );
}

double psc_signal_bool::get_net_delay() const
{
	return( this->m_sig_val.get_net_delay() );
}

void psc_signal_bool::dont_update( bool dont )
{
	this->m_sig_val.dont_update( dont );
//	this->m_new_val.dont_update( dont );
}

void psc_signal_bool::register_port( sc_port_base &port_, const char* if_typename_)
{
	// increment the number of connected ports
	m_n_connec++;

	// incrments the fanout for this net
	// if the port type is 'sc_in'
	string ptype = port_.kind();
	if ( ptype == "sc_in" )
		this->m_sig_val.inc_fanout();

	// invoke the overriden method
	sc_signal<bool>::register_port( port_, if_typename_ );
}

void psc_signal_bool::end_of_elaboration()
{
	// this case applies when this signal is
	// an output net
	if ( this->m_sig_val.get_fanout() == 0 )
		this->m_sig_val.inc_fanout();
}

double  psc_signal_bool::get_sp0() const
{
	return( m_sig_val.get_sp0() );
}

double  psc_signal_bool::get_sp1() const
{
	return( m_sig_val.get_sp1() );
}

void psc_signal_bool::end_of_simulation()
{
	m_sig_val.finish_pending();
}
