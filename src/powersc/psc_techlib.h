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

/****************************************************************************

  psc_techlib.h -- Technology library related classes.

  Original Author: Felipe Klein <klein@ic.unicamp.br>, IC/UNICAMP, 2006.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 ***************************************************************************/
#ifndef PSC_TECHLIB_H
#define PSC_TECHLIB_H

#include <string>
#include <vector>
#include <list>

using namespace std;

namespace psc_power_base
{

// Modeling abstraction level
typedef enum {
	PSC_UNDEF_LEVEL,
	PSC_GATE_LEVEL,
	PSC_RT_LEVEL
} psc_level_t;

// classes defined in this module
class psc_techlib;
class psc_pin_power_info;
class psc_cell_power_info;
class psc_cells_repository;

// --------------------------------------------------------------------------
//  CLASS : psc_techlib
//
// This class' objects hold the information from the technology library
// --------------------------------------------------------------------------
class psc_techlib {
	public:

		psc_techlib()
		{
			m_d_oc_volt = 0.0;
			m_d_unit_volt = 0.0;
			m_d_unit_capac = 0.0;
			m_d_unit_time = 0.0;
			m_d_unit_leakpwr = 0.0;
			m_d_unit_dynpwr = 0.0;
		}

		// operating conditions related methods
		void set_voltage( const double val );
		double get_voltage() const;

		// library units related methods
		void set_volt_unit( const double unit );
		double get_volt_unit() const;

		void set_cap_unit( const double unit );
		double get_cap_unit() const;

		void set_time_unit( const double unit );
		double get_time_unit() const;
		
		void set_leak_unit( const double unit );
		double get_leak_unit() const;

		void set_dynpwr_unit( const double unit );
		double get_dynpwr_unit() const;

		void print( const char *fname = NULL );
		
	protected:

		// operating conditions
		double m_d_oc_volt;
		
		// units
		double m_d_unit_volt;
		double m_d_unit_capac;
		double m_d_unit_time;
		double m_d_unit_leakpwr;
		double m_d_unit_dynpwr;
};

// --------------------------------------------------------------------------
//  CLASS : psc_pin_power_info
//
// Used to store the power information related to a cell's pin
// --------------------------------------------------------------------------
class psc_pin_power_info {
	public:

		psc_pin_power_info( const string & id, double E_z )
			: m_str_id( id ), m_d_energy( E_z )
		{}

		psc_pin_power_info( const string & id )
			: m_str_id( id ), m_d_energy( 0.0 )
		{}

		string get_id();
		double get_energy();
		void set_rel_pins( const int argc, ... );
		vector<string> & get_rel_pins();

	protected:

		string m_str_id;				// pin id string
		double m_d_energy;			// the energy calculated for a transition
		vector<string> m_rel_pins;	// related pins list (string id)
};

// --------------------------------------------------------------------------
//  CLASS : psc_cell_power_info
// --------------------------------------------------------------------------
class psc_cell_power_info {
	public:

		psc_cell_power_info( const string & name, const string & type )
			: m_str_name( name ), m_str_type( type ), m_d_leak_power( 0.0 ),
			m_d_aggr_power( 0.0 ), m_level( PSC_UNDEF_LEVEL ),
			m_b_macro( false )
		{}

		void add( const psc_pin_power_info & info );
		void set_leak_power( const double power );
		double get_leak_power() const;
		string get_name();
		string get_type();
		void set_level( psc_level_t level );
		psc_level_t get_level();
		bool is_macrocell() const;
		void set_power( const double power );
		double get_power();
		list<psc_pin_power_info> & get_pins_info();

	protected:

		string m_str_name;
		string m_str_type;
		double m_d_leak_power;
		double m_d_aggr_power; 		// aggregate power (used in macrocells)
		psc_level_t m_level;
		bool m_b_macro; 				// is it a macrocell?
		list<psc_pin_power_info> m_pin_info;
};

// --------------------------------------------------------------------------
//  CLASS : psc_cells_repository
//
// Used to store all instantiated cells from the library
// --------------------------------------------------------------------------
class psc_cells_repository {
	public:

		psc_cells_repository()
		{}

		void add( const psc_cell_power_info & info );
		vector<psc_cell_power_info> & get_power_info();
		void print_cells();
		void power_report( const char *fname = NULL );

		double get_total_leakpower();
		double get_total_intpower();

	protected:

		double compute_intpower( psc_cell_power_info & cell );

		vector<psc_cell_power_info> m_power_info;
};

} // namespace psc_power_base

#endif
