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

  psc_techlib.cpp -- Technology library related classes.

  Original Author: Felipe Klein <klein@ic.unicamp.br>, IC/UNICAMP, 2006.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 ***************************************************************************/
#include <stdarg.h>
#include "psc_techlib.h"
#include "psc_objinfo.h"

using namespace std;

namespace psc_power_base
{

//--------------------------------------------------------------------------
//  CLASS : psc_techlib
//--------------------------------------------------------------------------
psc_techlib psc_objinfo_base::techlib; 

void psc_techlib::set_voltage( const double val )
{
	m_d_oc_volt = val;
}

double psc_techlib::get_voltage() const
{
	return( m_d_oc_volt );
}

void psc_techlib::set_volt_unit( const double unit )
{
	m_d_unit_volt = unit;
}

double psc_techlib::get_volt_unit() const
{
	return( m_d_unit_volt );
}

void psc_techlib::set_cap_unit( const double unit )
{
	m_d_unit_capac = unit;
}

double psc_techlib::get_cap_unit() const
{
	return( m_d_unit_capac );
}

void psc_techlib::set_time_unit( const double unit )
{
	m_d_unit_time = unit;
}

double psc_techlib::get_time_unit() const
{
	return( m_d_unit_time );
}

void psc_techlib::set_leak_unit( const double unit )
{
	m_d_unit_leakpwr = unit;
}

double psc_techlib::get_leak_unit() const
{
	return( m_d_unit_leakpwr );
}

void psc_techlib::set_dynpwr_unit( const double unit )
{
	m_d_unit_dynpwr = unit;
}

double psc_techlib::get_dynpwr_unit() const
{
	return( m_d_unit_dynpwr );
}

void psc_techlib::print( const char *fname )
{
	FILE *fp = NULL;

	if ( fname ) {
		fp = fopen( fname, "w" );
		if ( !fp ) {
			cerr << "ERROR: the file '" << fname
				<< "' could not be opened for writing" << endl;
			return;
		}
	} else
		fp = stdout;
	
	fprintf( fp, "--------------- TECHNOLOGY LIBRARY INFO ---------------\n" );
	fprintf( fp, "Operating conditions:\n" );
	fprintf( fp, "  Voltage: %.4eV\n", (m_d_unit_volt * m_d_oc_volt) ); 
	fprintf( fp, "\n" );	
	fprintf( fp, "Units:\n" );
	fprintf( fp, "  Time         : %.4es\n", m_d_unit_time );
	fprintf( fp, "  Voltage      : %.4eV\n", m_d_unit_volt );
	fprintf( fp, "  Capacitive   : %.4ef\n", m_d_unit_capac );
	fprintf( fp, "  Leakage power: %.4eW\n", m_d_unit_leakpwr );
	fprintf( fp, "  Dynamic power: %.4eW\n", m_d_unit_dynpwr );
	fprintf( fp, "-------------------------------------------------------\n" );
}

//--------------------------------------------------------------------------
//  CLASS : psc_pin_power_info
//--------------------------------------------------------------------------
void psc_pin_power_info::set_rel_pins( const int argc, ... )
{
	va_list arguments;

	va_start( arguments, argc );

	for ( int i = 0 ; i < argc ; i++ ) {
		string curr_pid = va_arg( arguments, char *);
		m_rel_pins.push_back( curr_pid );
	}

	va_end( arguments );
}

vector<string> & psc_pin_power_info::get_rel_pins()
{
	return( m_rel_pins );
}

string psc_pin_power_info::get_id()
{
	return( m_str_id );
}

double psc_pin_power_info::get_energy()
{
	return( m_d_energy );
}

//--------------------------------------------------------------------------
//  CLASS : psc_cells_power_info
//--------------------------------------------------------------------------
void psc_cell_power_info::add( const psc_pin_power_info & info )
{
	m_pin_info.push_back( info );
}

void psc_cell_power_info::set_leak_power( const double power )
{
	m_d_leak_power = power;
}

void psc_cell_power_info::set_level( psc_level_t level )
{
	m_level = level;
	m_b_macro = (m_level != PSC_UNDEF_LEVEL) 
		&& (m_level != PSC_GATE_LEVEL);
}

void psc_cell_power_info::set_power( const double power )
{
	m_d_aggr_power = power;
}

double psc_cell_power_info::get_leak_power() const
{
	return( m_d_leak_power );
}

string psc_cell_power_info::get_name()
{
	return( m_str_name );
}

string psc_cell_power_info::get_type()
{
	return( m_str_type );
}

psc_level_t psc_cell_power_info::get_level()
{
	return( m_level );
}

bool psc_cell_power_info::is_macrocell() const
{
	return( m_b_macro );
}

double psc_cell_power_info::get_power()
{
	return( m_d_aggr_power );
}

list<psc_pin_power_info> & psc_cell_power_info::get_pins_info()
{
	return( m_pin_info );
}

//--------------------------------------------------------------------------
//  CLASS : psc_cells_repository
//--------------------------------------------------------------------------
psc_cells_repository psc_objinfo_base::cells;

void psc_cells_repository::add( const psc_cell_power_info & info )
{
	m_power_info.push_back( info );
}

vector<psc_cell_power_info> & psc_cells_repository::get_power_info()
{
	return( m_power_info );
}

void psc_cells_repository::print_cells()
{
	list<psc_pin_power_info>::iterator it_p;
			
	cerr << "--- Cells repository ---" << endl;

	for ( unsigned int i = 0 ; i < m_power_info.size() ; i++ ) {
		psc_cell_power_info & cell = m_power_info[ i ];

		cerr << "cell name: " << cell.get_name() << endl;

		cerr << "\ttype: " << cell.get_type();
		cerr << "\tlevel: " << cell.get_level() << endl;
		cerr << "\tleakage power: " << cell.get_leak_power();
		cerr << "\taggregate power: " << cell.get_power();
		cerr << endl;

		it_p = cell.get_pins_info().begin();
		for ( ; it_p != cell.get_pins_info().end() ; it_p++ ) {
			psc_pin_power_info & pinfo = *it_p;	

			cerr << "\tid: " << pinfo.get_id();
			cerr << "\tenergy: " << pinfo.get_energy();
			cerr << "\trelated pins: ";

			vector<string> & relpins = pinfo.get_rel_pins();
			string prefix = "";
			for ( unsigned int j = 0 ; j < relpins.size() ; j++ ) {
				cerr << prefix << relpins[ j ];
				prefix = ", ";
			}
			cerr << endl;
		}
	}

	cerr << "------------------------" << endl;
}

void psc_cells_repository::power_report( const char *fname )
{
	FILE *fp = NULL;

	if ( fname ) {
		fp = fopen( fname, "w" );
		if ( !fp ) {
			cerr << "ERROR: the file '" << fname
				<< "' could not be opened for writing" << endl;
			return;
		}
	} else
		fp = stdout;
	
	string level, cellname;
	double leak_unit, leakpwr, dynpwr_unit, intpwr, swpwr,
			 aggrpwr, sum_leak, sum_dyn, sum_int, sum_aggr;

	sum_leak = sum_int = sum_dyn = sum_aggr = 0;
	leak_unit = psc_objinfo_base::techlib.get_leak_unit();
	dynpwr_unit = psc_objinfo_base::techlib.get_dynpwr_unit();

	fprintf( fp, "----------------------------------------------- POWER REPORT ---------------------------------------\n" );
	fprintf( fp, " %-3s - %-20s - %-13s - %-15s - %-17s - %-18s\n", " L ", "     Cell  Name     ",
			"  Cell Type  ", " Leakage Power ", " Internal Power ", " Aggregate Power " );
	fprintf( fp, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n" );

	for ( int i = 0 ; i < (int)m_power_info.size() ; i++ ) {
		psc_cell_power_info & cell = m_power_info[ i ];

		if ( cell.get_level() == PSC_GATE_LEVEL ) {
			leakpwr = cell.get_leak_power() * leak_unit;
			intpwr = compute_intpower( cell ) * dynpwr_unit;
			aggrpwr = 0;
		} else if ( cell.get_level() == PSC_RT_LEVEL ) {
			leakpwr = 0;
			intpwr = 0;
			aggrpwr = cell.get_power();
		} else {
			// undefined level...so
			leakpwr = 0;
			intpwr = 0;
			aggrpwr = 0;
		}

		// increment the total leakage/dynamic power
		// and also the aggregate power
		sum_leak += leakpwr;
		sum_int += intpwr;
		sum_aggr += aggrpwr;

		level = "U";
		level = (cell.get_level() == PSC_GATE_LEVEL) ? "G" : "RT";

		cellname = cell.get_name();
		if ( cell.get_name().length() > 20 ) {
			cellname = cellname.substr(0, 7) + "..."
				+ cellname.substr(cellname.length()-10);
		}

		fprintf( fp, " %-3s - %-20s - %-13s - %-15e - %-17e - %-17e\n", level.c_str(), cellname.c_str(),
			  cell.get_type().c_str(), leakpwr, intpwr, aggrpwr);
	}

	swpwr = psc_objinfo_base::repository.get_switching_power();
	sum_dyn = sum_int + swpwr;

	fprintf( fp, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n" );
	fprintf( fp, "Summary:\n" );
	fprintf( fp, "  Switching power : %15eW\n", swpwr );
	fprintf( fp, "  Internal power  : %15eW\n", sum_int );
	fprintf( fp, "  Leakage power   : %15eW\n", sum_leak );
	fprintf( fp, "  Aggregate power : %15eW\n", sum_aggr );
	fprintf( fp, "                     ----------------\n");
	fprintf( fp, "  TOTALS          : %15eW\n", sum_leak+sum_dyn+sum_aggr );
	fprintf( fp, "----------------------------------------------------------------------------------------------------\n" );
}

double psc_cells_repository::compute_intpower( psc_cell_power_info & cell )
{
	list<psc_pin_power_info> pinsinfo; // cell's pins info
	list<psc_pin_power_info>::iterator it1, it2;
	vector<string> done;
	double power, energy, totalSP;
	string id, idrel;
	repository_entry_t *infoid, *inforel;

	// initial values
	pinsinfo = cell.get_pins_info();
	power = 0;
	inforel = infoid = NULL;
	done.clear();

	it1 = pinsinfo.begin();
	for ( bool found ; it1 != pinsinfo.end() ; it1++ ) {
		// check if the pin has been already checked
		found = false;
		for ( int i = 0 ; i < (int)done.size() ; i++ )
			if ( done[i] == (*it1).get_id() ) {
				found = true;
				break;
			}

		if ( found )
			continue;
		
		energy = totalSP = 0;

		it2 = pinsinfo.begin();
		for ( ; it2 != pinsinfo.end() ; it2++ ) {
			if ( (*it2).get_id() == (*it1).get_id() ) {
				idrel = (*it2).get_rel_pins().front();
				inforel = psc_objinfo_base::repository.get_entry( idrel );

				if ( inforel && (inforel->toggle_rate > 0) ) {
					totalSP += inforel->sp1;
					energy += (*it2).get_energy() * inforel->sp1;
				}
			}
		}

		id = (*it1).get_id();
		infoid = psc_objinfo_base::repository.get_entry( id );

		if ( infoid && (totalSP > 0) )
			power += (energy * infoid->toggle_rate) / totalSP;

		/*cerr << "id=" << (*it1).get_id();
		cerr << "\tenergy=" << energy;
		cerr << "\ttotalSP=" << totalSP;
		cerr << "\tpower=" << power << endl;*/

		// add the ID to the done list
		done.push_back( (*it1).get_id() );
	}

	return( power );
}

double psc_cells_repository::get_total_leakpower()
{
	double leak_unit, sum_leak;

	sum_leak = 0;
	leak_unit = psc_objinfo_base::techlib.get_leak_unit();

	for ( int i = 0 ; i < (int)m_power_info.size() ; i++ ) {
		psc_cell_power_info & cell = m_power_info[ i ];

		sum_leak += cell.get_leak_power() * leak_unit;
	}

	return( sum_leak );
}

double psc_cells_repository::get_total_intpower()
{
	double dynpwr_unit, intpwr, sum_int;

	sum_int = 0;
	dynpwr_unit = psc_objinfo_base::techlib.get_dynpwr_unit();

	for ( int i = 0 ; i < (int)m_power_info.size() ; i++ ) {
		psc_cell_power_info & cell = m_power_info[ i ];

		intpwr = compute_intpower( cell ) * dynpwr_unit;

		// increment the total internal power
		sum_int += intpwr;
	}

	return( sum_int );
}

} // namespace psc_power_base
