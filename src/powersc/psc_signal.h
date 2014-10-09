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

  psc_signal.h -- The psc_signal<T> channel class.

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_SIGNAL_H
#define PSC_SIGNAL_H

#include <systemc.h>

using namespace psc_dt;

// ----------------------------------------------------------------------------
//  CLASS TEMPLATE: psc_signal<T>
//
//  The psc_signal<T> class.
// ----------------------------------------------------------------------------
template <class T>
class psc_signal : public sc_signal<T> 
{
public:
	
	// constructors
	
	psc_signal()
		{
			m_n_connec = 0;
			init_info();
		}

	explicit psc_signal( const char *name_ )
		: sc_signal<T>( name_ )
		{
			m_n_connec = 0;
			init_info(name_);
		}

   inline
   void set_alias( const char *new_alias );

	inline
	void  set_wire_load( const double load );

	inline
	double get_wire_load() const;

	inline
	void set_net_load( const double load );

	inline
	void add_to_net_load( const double load );

	inline
	double get_net_load() const;

	inline
	void set_net_delay( const double delay );

	inline
	double get_net_delay() const;

	inline
	void dont_update( bool dont = true );

	inline
	void
	register_port( sc_port_base &port_, const char* if_typename_);

	inline
	void end_of_simulation();

	inline
	int get_num_conn() const
	{ return( m_n_connec ); }

	inline
	int get_fanout() const
	{ return( m_cur_val.get_fanout() ); }

   inline
   unsigned int get_toggle_count() const
	{ return( m_cur_val.get_toggle_count() ); }

   inline
   string & get_id()
	{ 
		return( m_cur_val.get_id() ); 
	}

   inline
   double get_toggle_rate_u() const
	{ return( m_cur_val.get_toggle_rate_u() ); }

   inline
   double get_toggle_rate() const
	{ return( m_cur_val.get_toggle_rate() ); }

   inline
   vector<double> get_split_toggle_rate() const
	{ return( m_cur_val.get_split_toggle_rate() ); }

   inline
   double get_sp0() const;		// static probaility at 0

   inline
   double get_sp1() const;		// static probaility at 1

	void end_of_elaboration();

   // operators

   psc_signal<T>& operator = ( const T& a )
	{ sc_signal<T>::write( a ); return *this; }

   psc_signal<T>& operator = ( const psc_signal<T>& a )
	{ sc_signal<T>::write( a.read() ); return *this; }

protected:

	// number of connected ports to this signal
	int m_n_connec;

	using sc_signal<T>::m_cur_val;
	
private:
	inline
	void init_info(const char *name_ = NULL);
};

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

template <class T>
inline
void psc_signal<T>::init_info(const char *name_)
{
	::std::string cur_str, new_str;

   // to guarantee that the ID have been set
   this->m_cur_val.force_init_info();
   this->m_new_val.force_init_info();

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
	
	//cur_str += "m_cur_val";
	new_str += "m_new_val";
	
	this->m_cur_val.set_alias(cur_str);
	this->m_cur_val.set_is_net();

	this->m_new_val.set_alias(new_str);
	this->m_new_val.dont_update();
}


template <class T>
inline
void psc_signal<T>::set_alias( const char *new_alias )
{
#ifdef DEBUG_POWER_L3
   cerr << "\t[psc_signal]: Setting alias \"" << new_alias << "\"" << endl;
#endif

   init_info( new_alias );
}


template <class T>
inline
void  psc_signal<T>::set_wire_load( const double load )
{
	this->m_cur_val.set_wire_load( load );
}

template <class T>
inline
double psc_signal<T>::get_wire_load() const
{
	return( this->m_cur_val.get_wire_load() );
}

template <class T>
inline
void psc_signal<T>::set_net_load( const double load )
{
	this->m_cur_val.set_net_load( load );
}

template <class T>
inline
void psc_signal<T>::add_to_net_load( const double load )
{
	this->m_cur_val.add_to_net_load( load );
}

template <class T>
inline
double psc_signal<T>::get_net_load() const
{
	return( this->m_cur_val.get_net_load() );
}

template <class T>
inline
void psc_signal<T>::set_net_delay( const double delay )
{
	this->m_cur_val.set_net_delay( delay );
}

template <class T>
inline
double psc_signal<T>::get_net_delay() const
{
	return( this->m_cur_val.get_net_delay() );
}

template <class T>
inline
void psc_signal<T>::dont_update( bool dont )
{
	this->m_cur_val.dont_update( dont );
	this->m_new_val.dont_update( dont );
}

template <class T>
inline
void psc_signal<T>::register_port( sc_port_base &port_, const char* if_typename_)
{
	// increment the number of connected ports
	m_n_connec++;

	// incrments the fanout for this net
	// if the port type is 'sc_in'
	string ptype = port_.kind();
	if ( ptype == "sc_in" )
		this->m_cur_val.inc_fanout();

	// invoke the overriden method
	sc_signal<T>::register_port( port_, if_typename_ );
}

template <class T>
void psc_signal<T>::end_of_elaboration()
{
	// this case applies when this signal is
	// an output net
	if ( this->m_cur_val.get_fanout() == 0 )
		this->m_cur_val.inc_fanout();
}

template <class T>
inline
double  psc_signal<T>::get_sp0() const
{
	return( m_cur_val.get_sp0() );
}

template <class T>
inline
double  psc_signal<T>::get_sp1() const
{
	return( m_cur_val.get_sp1() );
}

template <class T>
inline
void psc_signal<T>::end_of_simulation()
{
	m_cur_val.finish_pending();
}

#endif
