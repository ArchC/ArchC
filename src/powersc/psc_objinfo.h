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

  psc_objinfo.h -- The psc_objinfo<W, T> object information class.

  Original Author: Felipe Klein <klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_OBJINFO_H
#define PSC_OBJINFO_H

#include <stack>
#include <string>
#include <systemc.h>

//#include "base/psc_objinfo_if.h"
//#include "base/psc_obj_rep.h"
//#include "base/psc_techlib.h"

#include "psc_objinfo_if.h"
#include "psc_obj_rep.h"
#include "psc_techlib.h"


using namespace std;

namespace psc_power_base
{


// classes defined in this module
class psc_objinfo_base;
template <int W, class T> class psc_objinfo;


// ----------------------------------------------------------------------------
//  CLASS : psc_objinfo_base
//
// The only purpose of this class is to maintain a static object (repository)
// shared amongst all instances of PowerSC datatypes
// ----------------------------------------------------------------------------
class psc_objinfo_base
{
public:

	psc_objinfo_base()
	{
	}
	
   static psc_obj_repository repository; // like a database of PowerSC objects
	static psc_cells_repository cells;
	static psc_techlib techlib; // info from the technology library
   
   inline
   bool self_register(const char *entry_name)
      { return( repository.add_register(entry_name) ); }

   inline
   bool self_unregister(const char *entry_name)
      { return( repository.unregister(entry_name) ); }

   inline
   bool self_register(const char *entry_name, repository_entry_t &e)
   	{ return( repository.add_register(entry_name, e) ); }
   
   inline
   bool update_registry(const char *entry_name, repository_entry_t &e)
   	{ return( repository.update_registry(entry_name, e) ); }
};


// ----------------------------------------------------------------------------
//  CLASS TEMPLATE : psc_objinfo
//
// Each PowerSC object has some unique information like its ID, and other
// non-unique information like an alias string for the object. This information
// is managed by this class. This class is also responsible for gathering 
// signal statistics throughout the simulation.
// ----------------------------------------------------------------------------
template <int W, class T>
class psc_objinfo : public psc_objinfo_base, public psc_objinfo_if
{
   
typedef int (*perform_diff_on_bits)(const T &, const T &);
typedef unsigned short int (*number_of_bits_on)(const T &);
   
public:

   // constructor

   psc_objinfo(const char *prefix)
      {
         m_strPrefix = prefix;
         m_strAlias = "none";
			m_bDontUpdate = false;
         m_nToggleCount = 0; 
         m_bFirstTime = true;
			m_bPending = true;
         m_bInfoSet = false;
			m_d_wire_load = m_d_net_load = m_d_net_delay = 0.0;
			m_b_is_net = false; // only sc_signals set this to true
			m_n_fanout = 0;

			memset( m_tc_bits, 0, sizeof(unsigned int)*W );
			memset( m_time_at_0, 0, sizeof(uint64)*W );
			memset( m_time_at_1, 0, sizeof(uint64)*W );

#ifdef DEBUG_POWER_L3
         cerr << "\t[psc_objinfo]: Creating " << prefix << W << " object" << endl;
#endif
      }

   // destructor

   virtual ~psc_objinfo()
      {
#ifdef DEBUG_POWER_L3
         if ( m_bFirstTime )
            cerr << "\t[psc_objinfo]: Destroying temporary " << m_strPrefix << W << " object" << endl;
         else
            cerr << "\t[psc_objinfo]: Destroying " << m_strID.c_str() << endl;
#endif

         if ( get_toggle_count() > 0 ) {
            flush_data(); // flushes information to repository if necessary
         } else if ( ! m_bFirstTime ) {
            self_unregister( m_strID.c_str() );
         }
      }

   // public methods

   virtual unsigned short int on_bits( const T & v ) = 0; // must returns the number of bits with value '1'
   virtual T  bit_diff(const T & v1, const T & v2) = 0; // makes a xor between v1 and v2
   virtual void update_toggle_count(const T & cur_val, const T & new_val);
   void inc_toggle_count(int v);
   void inc_bit_toggle_count(int index, int v);

	virtual uint64 uint64value( const T & v ) const = 0;
	void update_static_prob( const uint64 & changed );

   inline
   unsigned int get_toggle_count() const
      { return( m_nToggleCount ); }

   inline
   double get_toggle_rate_u() const;

   inline
   double get_toggle_rate() const;

	inline
	vector<double> get_split_toggle_rate() const;

   inline
   double get_sp0() const;		// static probability at 0

   inline
   double get_sp1() const;		// static probability at 1

	inline
	void set_is_net();

	inline
	int get_fanout() const;

	inline
	void inc_fanout();

   inline
   const char * get_str_id()
      { return m_strID.c_str(); }
   
	inline
	string & get_id()
		{ return( m_strID ); }

   inline
   const char * get_str_alias()
      { return m_strAlias.c_str(); }

   inline
   void set_alias(const char *new_alias)
      {
         force_init_info();
         
#ifdef DEBUG_POWER_L3
         cerr << "\t[psc_objinfo]: Setting alias \"" << new_alias << 
            "\" for object with id " << get_str_id() << endl;
#endif
         m_strAlias = new_alias; 
      }

	inline
	void  set_wire_load( const double load )
	{ m_d_wire_load = load; }

	inline
	double get_wire_load() const
	{ return( m_d_wire_load ); }

	inline
	void set_net_load( const double load )
	{ m_d_net_load = load; }

	inline
	void add_to_net_load( const double load );

	inline
	double get_net_load() const
	{ return( m_d_net_load + m_d_wire_load ); }

	inline
	void set_net_delay( const double delay )
	{ m_d_net_delay = delay; }

	inline
	double get_net_delay() const
	{ return( m_d_net_delay ); }

   inline
   void set_alias(const ::std::string & new_alias)
      {
         force_init_info();
         
#ifdef DEBUG_POWER_L3
         cerr << "\t[psc_objinfo]: Setting alias \"" << new_alias << 
            "\" for object with id " << get_str_id() << endl;
#endif
         m_strAlias = new_alias; 
      }

   inline
   void refresh_last_update()
      { m_last_update_time = sc_time_stamp(); }

   inline
   sc_time get_last_update() const
      { return( m_last_update_time ); }

	inline
	void dont_update(bool dont = true) 
		{ m_bDontUpdate = dont; }

	inline
	bool must_update() 
		{ return( !m_bDontUpdate ); }

   inline
   void force_init_info()
   {
      init_info();
   }

   void finish_pending();

	// debugging purposes
	void print_time_at();
   
protected:

   void init_info();
   void set_obj_id();
   void flush_data( bool destroying = true );
   
private:
   
   // attributes

   // basic object information
   static int m_nID; 			// object identifier
   string m_strID;			// string identifier (psc_uint_W_m_nID)
   string m_strAlias; 		// string alias (useful for debugging)
   string m_strPrefix; 		// prefix used in the identifier string
	bool m_b_is_net;
	int m_n_fanout;
   
   // toggling information
   unsigned int m_nToggleCount;		// keeps the bit toggling count value
   T m_cur_val;				// current object's value
   T m_new_val;				// the new object's value

	// technology library information for the object
	double m_d_wire_load;			// capacitance associated to the wire only
	double m_d_net_load;				// capacitance on the net (includes the wire load)
	double m_d_net_delay;			// transition time calculated based on the RC tree type

	// information used to calculate the static probability
	uint64 m_time_at_0[W];
	uint64 m_time_at_1[W];

	unsigned int m_tc_bits[W];			// toggle count for individual bits

   bool m_bFirstTime;
	bool m_bPending;					// once finish_pending is called, this must be always false
   sc_time m_last_update_time;
   sc_time m_previous_change_time;
	bool m_bDontUpdate;
   bool m_bInfoSet; // init_info already called?
};

template <int W, class T> int psc_objinfo<W, T>::m_nID; // static member used to create an unique ID for the object

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

template<int W, class T>
void psc_objinfo<W, T>::update_toggle_count(const T & cur_val, const T & new_val)
{
   if ( ! repository.is_to_sample() )
      return;
   
	// if you don't want to take the toggle count
	// of this object into account
	if (m_bDontUpdate)
		return;

   if ( m_bFirstTime ) {
      init_info(); // name the object
      
      // insert an entry in the repository for this object
      repository_entry_t entry;
      entry.toggle_count = 0; // initially, this value holds zero
      entry.alias = get_str_alias();
      entry.pobj = this;
      entry.valid_pointer = true;
		entry.is_net = m_b_is_net;
		entry.wload = m_d_wire_load;
		entry.nload = m_d_net_load;
		entry.ndelay = m_d_net_delay;
		entry.fanout = m_n_fanout;
      self_register( m_strID.c_str(), entry );

		m_bFirstTime = false;
      refresh_last_update();
		update_static_prob( 0 );

      m_cur_val = cur_val;
      m_new_val = new_val;

      m_previous_change_time = get_last_update();

//		cerr << endl << PRINT_OBJ_STR << "m_previous_change_time = " << m_previous_change_time << endl;
//		cerr << endl << "\tNOW IS " << sc_time_stamp() << endl;
//		cerr << "\tm_cur_val = " << m_cur_val << endl;
//		cerr << "\tm_new_val = " << m_new_val << endl;
   } else if ( sc_time_stamp() == get_last_update() ) {
      m_new_val = new_val;
   } else {
      T changed_bits;
      unsigned short changed_count = 0;

      // increment the toggle count with the number of bits that toggled
      changed_bits = bit_diff(m_new_val, m_cur_val);
      changed_count = on_bits(changed_bits);
      inc_toggle_count(changed_count);

      m_cur_val = m_new_val;
      m_new_val = new_val;

      if (changed_count != 0) {
			update_static_prob( uint64value(changed_bits) );
	 		m_previous_change_time = get_last_update();
		}
      
      refresh_last_update();
	      
#ifdef DEBUG_POWER_L2
   if (changed_count == 0)
      cerr << "\t" << PRINT_OBJ_STR << " -> Updating Toggle Count: NO bits changed since " << m_previous_change_time << endl;
   else {
      cerr << hex << "\t" << PRINT_OBJ_STR << " -> Updating Toggle Count: from " << m_cur_val << " to " << m_new_val
	 << " => " << dec << changed_count << " bit(s) changed @ " << m_previous_change_time << endl;
   }
#endif

   }
}

template<int W, class T>
void psc_objinfo<W, T>::update_static_prob( const uint64 & changed )
{
#ifdef DEBUG_POWER_L2
	cerr << "\t" << PRINT_OBJ_STR << " -> Updating the static probability" << endl;
#endif
	// difference between last update and previous change time
	uint64 diff;

	// bits that have changed have the value '1'
	sc_uint<W> changed_bits = changed;

	// temporary variables used to detect which 
	// change has occurred (0->1 or 1->0)
	sc_uint<W> curval = uint64value(m_cur_val);
	sc_uint<W> newval = uint64value(m_new_val);
	
	for ( int i = 0 ; i < W ; i++ ) {
		diff = get_last_update().value() - m_previous_change_time.value();

		if ( changed_bits[i] ) {
			if ( curval[i] == 0 ) {
				m_time_at_1[ i ] += diff;
			} else {
				m_time_at_0[ i ] += diff;
			}
		} else {
			if ( newval[i] == 0 ) {
				m_time_at_0[ i ] += diff;
			} else {
				m_time_at_1[ i ] += diff;
			}
		}
	}

#ifdef DEBUG_POWER_L2
	cerr << "\tT0=";
	for ( int i = 0 ; i < W ; i++ )
		cerr << m_time_at_0[i] << "\t";
	cerr << endl;

	cerr << "\tT1=";
	for ( int i = 0 ; i < W ; i++ )
		cerr << m_time_at_1[i] << "\t";
	cerr << endl;
#endif
}

template<int W, class T>
void psc_objinfo<W, T>::inc_toggle_count(int v)
{ 
	// if you don't want to take the toggle count
	// of this object into account
	if (m_bDontUpdate)
		return;
	
   m_nToggleCount += v;
}

template<int W, class T>
void psc_objinfo<W, T>::inc_bit_toggle_count(int index,  int v)
{ 
	// if you don't want to take the toggle count
	// of this object into account
	if (m_bDontUpdate)
		return;
	
   m_tc_bits[ index ] += v;
}

// This method is used only for objects that account for toggle transitions
// greater than zero. If the object is a temporary object (e.g., created by a
// conversion in an assignment), this code is not executed.
template <int W, class T>
void psc_objinfo<W, T>::init_info()
{
   if ( ! m_bInfoSet ) {
      m_nID++;
      set_obj_id();
      m_bInfoSet = true;
   }
}

template <int W, class T>
void psc_objinfo<W, T>::set_obj_id()
{
	char str[32];

	sprintf(str, "%s_%d_#%d", m_strPrefix.c_str(), W, m_nID);
	m_strID = str;
}

template <int W, class T>
inline
double psc_objinfo<W, T>::get_toggle_rate_u() const
{
	/* The value calculated here considers that
	 * the toggle rate of the individual bits
	 * is uniformly distributed */
	return( (double)m_nToggleCount / sc_simulation_time() );
}

/* using the default time unit
 * this value must be adapted to the library's
 * time unit */
template <int W, class T>
inline
double psc_objinfo<W, T>::get_toggle_rate() const
{
	double TR, sim_time;

	TR = 0;
	sim_time = sc_simulation_time();

//	cerr << "alias = " << m_strAlias << endl;
	for ( int i = 0 ; i < W ; i++ ) {
//		cerr << "\tTC[" << i << "] = " << m_tc_bits[i];
//		cerr << "\ttime_at_1[" << i << "] = " << m_time_at_1[i] << endl;
		TR += ((double)m_tc_bits[ i ]);
	}
//	cerr << "\tsummation: TR = " << TR << endl;
//	cerr << "\tsummation: W = " << W << endl;
//	cerr << "\tsummation: sim_time = " << sim_time << endl;
//	cerr << "--------------------------" << endl;
	return( (TR/sim_time)/(double)W );
}

/* using the default time unit
 * this value must be adapted to the library's
 * time unit */
template <int W, class T>
inline
vector<double> psc_objinfo<W, T>::get_split_toggle_rate() const
{
	vector<double> bits;
	double TR, sim_time;

	TR = 0;
	sim_time = sc_simulation_time();

	for ( int i = 0 ; i < W ; i++ ) {
		TR = ((double)m_tc_bits[ i ]);
		TR /= sim_time;
		bits.push_back( TR );
	}
	return( bits );
}

template <int W, class T>
inline
double psc_objinfo<W, T>::get_sp0() const
{
	double SP0, SP1;

	SP0 = SP1 = 0;

	for ( int i = 0 ; i < W ; i++ ) {
		SP0 += (double)m_time_at_0[ i ];
		SP1 += (double)m_time_at_1[ i ];
	}

	return( SP0/(SP0+SP1) );
}

template <int W, class T>
inline
double psc_objinfo<W, T>::get_sp1() const
{
	double SP1, SP0/*, sim_time*/;

	SP0 = SP1 = 0;
//	sim_time = sc_simulation_time();

	for ( int i = 0 ; i < W ; i++ ) {
		SP0 += (double)m_time_at_0[ i ];
		SP1 += (double)m_time_at_1[ i ];
	}

	return( SP1/(SP0+SP1) );
//	return( (SP1/sim_time)/(double)W );
}

template <int W, class T>
inline
void psc_objinfo<W, T>::set_is_net()
{
	m_b_is_net = true;
}

template <int W, class T>
inline
void psc_objinfo<W, T>::add_to_net_load( const double load )
{
	m_d_net_load += load;
}

template <int W, class T>
inline
int psc_objinfo<W, T>::get_fanout() const
{
	return( m_n_fanout );
}

template <int W, class T>
inline
void psc_objinfo<W, T>::inc_fanout()
{
	m_n_fanout++;
}

template <int W, class T>
void psc_objinfo<W, T>::flush_data( bool destroying )
{
#ifdef DEBUG_POWER_L3
   cerr << "\t[psc_objinfo]: Flushing data for " << m_strID.c_str() << endl;
#endif

   // otherwise update entry in the registry with current data
   repository_entry_t entry;
   entry.toggle_count = get_toggle_count();
	entry.toggle_rate = get_toggle_rate();
   entry.alias = get_str_alias();
   entry.pobj = (destroying) ? NULL : this;
   entry.valid_pointer = (destroying) ? false : true;
	entry.is_net = m_b_is_net;
	entry.wload = m_d_wire_load;
	entry.nload = m_d_net_load;
	entry.ndelay = m_d_net_delay;
	entry.fanout = m_n_fanout;
	entry.sp0 = get_sp0();
	entry.sp1 = get_sp1(); 
   update_registry( get_str_id(), entry );
}

template <int W, class T>
void psc_objinfo<W, T>::finish_pending()
{
	// if you don't want to take the toggle count
	// of this object into account
	if (m_bDontUpdate)
		return;

   if (!m_bFirstTime && m_bPending) {
      T changed_bits;
      unsigned short changed_count = 0;

      // increment the toggle count with the number of bits that toggled
      changed_bits = bit_diff(m_new_val, m_cur_val);
      changed_count = on_bits(changed_bits);
      inc_toggle_count(changed_count);

      if (changed_count != 0) {
			update_static_prob( uint64value(changed_bits) );
	 		m_previous_change_time = get_last_update();
		}

		m_bPending = false; // nevemore it is to be executed (this block, of course)

#ifdef DEBUG_POWER_L2
      if (changed_count == 0)
	 		cerr << "\t" << PRINT_OBJ_STR << " -> Updating Toggle Count: NO bits changed since " << m_previous_change_time << endl;
      else {
	 		cerr << hex << "\t" << PRINT_OBJ_STR << " -> Updating Toggle Count: from " << m_cur_val << " to " << m_new_val
	    		<< " => " << dec << changed_count << " bit(s) changed @ " << m_previous_change_time << endl;
      }
#endif

		// last update to the static probability vectors
      m_cur_val = m_new_val;
      refresh_last_update();
		update_static_prob( 0 );
   }

//	print_time_at();
}

template <int W, class T>
void psc_objinfo<W, T>::print_time_at()
{
	string prefix;

	cerr << "\n--- time at 0/1 - " << get_str_id() << " ---" << endl;

	prefix = "";
	cerr << "\ttime_at_0 = [";
	for ( int i = 0 ; i < W ; i++ ) {
		cerr << prefix << m_time_at_0[ i ];
		prefix = ", ";
	}
	cerr << "]" << endl;

	prefix = "";
	cerr << "\ttime_at_1 = [";
	for ( int i = 0 ; i < W ; i++ ) {
		cerr << prefix << m_time_at_1[ i ];
		prefix = ", ";
	}
	cerr << "]" << endl;
}


} // namespace psc_power_base

#endif
