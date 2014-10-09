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

#include "psc_obj_rep.h"
#include "psc_objinfo.h"
#include <stdio.h>
#include <fstream>

using namespace std;

namespace psc_power_base
{

psc_obj_repository psc_objinfo_base::repository;

/**
  Register the PowerSC object in the map.
  Return false if any problem occurs, otherwise return true.
  */
bool psc_obj_repository::add_register(const char *entry_name)
{
   repository_entry_t entry;

   return( add_register(entry_name, entry) );
}

bool psc_obj_repository::add_register(const char *entry_name, repository_entry_t &e)
{
   repository_map::const_iterator itr = m_map.find(entry_name); // look for the entry in the map

   if (itr != m_map.end()) {
      cerr << "Error: entry '" << entry_name << "' already exists. Skipping entry registration" << endl;
      return(false);
   } 
   
   string name = entry_name;
   
   m_map[name] = e;

#ifdef DEBUG_POWER_L3
   cerr << "\t[psc_obj_repository]: Registering " << entry_name << endl;
#endif

   return(true);
}

/**
  Unregister the PowerSC object from the map.
  Return false if any problem occur, otherwise return true.
  */
bool psc_obj_repository::unregister(const char *entry_name)
{
   string name = entry_name;
   repository_map::iterator itr = m_map.find(name);

   if (itr == m_map.end()) {
      cerr << "Error: entry '" << entry_name << "' does not exist. Skipping entry un-registration" << endl;
      return(false);
   }

   m_map.erase(itr); // remove the entry pointed by itr

#ifdef DEBUG_POWER_L3
   cerr << "\t[psc_obj_repository]: Unregistering " << entry_name << endl;
#endif
   
   return(true);
}

bool psc_obj_repository::update_registry(const char *entry_name, repository_entry_t &e)
{
   string name = entry_name;
   repository_map::iterator itr = m_map.find(name);

   if (itr == m_map.end()) {
      cerr << "Error: entry '" << entry_name << "' does not exist. Skipping entry update" << endl;
      return(false);
   }

   m_map[name] = e;
   
#ifdef DEBUG_POWER_L3
   cerr << "\t[psc_obj_repository]: Updating entry " << entry_name << endl;
#endif

   return(true);
}

repository_entry_t *psc_obj_repository::get_entry( const string & name )
{
	repository_map::iterator it = m_map.find( name );

	// the entry does not exist. This will happen when:
	// 1: the entry simply does not exist :-)
	// 2: the toggle count for the entry is zero (so, the
	// object has been destroyed)
	if ( it == m_map.end() )
		return( NULL );

	repository_entry_t *entry = &m_map[ name ];

	return( entry );
}

void psc_obj_repository::print_entries()
{
   double average_tc;
   long long int totalTC;
   repository_map::const_iterator itr = m_map.begin();
   
   cerr << "--- psc_obj_repository entries ---" << endl;

   totalTC = 0;
   
   for (int i = 0; itr != m_map.end(); i++, itr++) {
      const string & current_key = (*itr).first;
      const repository_entry_t & current_value = (*itr).second;

      totalTC += current_value.toggle_count; // increments the total toggle count

		cerr << " " << i+1 << ". id=" << current_key << "\tTC=" << 
			current_value.toggle_count << "\talias=" << current_value.alias;
		
		cerr << "\tnet=" << current_value.is_net;
		cerr << "\tfanout=" << current_value.fanout;
		cerr << "\tnload=" << current_value.nload;
		cerr << "\tndelay=" << current_value.ndelay;
		cerr << "\twload=" << current_value.wload;
		cerr << "\tTR=" << current_value.toggle_rate;
		cerr << "\tSP0=" << current_value.sp0;
		cerr << "\tSP1=" << current_value.sp1;
		cerr << "\ttechinfo=" << current_value.techinfo;
		cerr << endl;
   }
   
   cerr << "----------------------------------" << endl;
   cerr << "TOTAL toggle count: " << totalTC << endl;

   if ( m_activity_sampler->is_full_sampling() ) {
      average_tc = (double)(totalTC) / sc_simulation_time();
      cerr << "Average toggle count (fully simulated): " << average_tc << " transitions / " << sc_get_default_time_unit() << endl;
   } else {
      double mon_time = m_activity_sampler->get_monitored_time();
      double percent = (mon_time / sc_simulation_time()) * 100.0;
      average_tc = m_activity_sampler->get_global_mean();
      cerr << "Average toggle count (sampled): " << average_tc << " transitions / " << sc_get_default_time_unit() << endl;
      cerr << "Sampled time: " << percent << "%" << endl;
   }
   
   cerr << "----------------------------------" << endl;
}

void psc_obj_repository::write_entries_to_csv_file(const char *name)
{
   repository_map::const_iterator itr = m_map.begin();
   ofstream output(name, ios_base::out);

   if (!output) {
      cerr << "Error: cannot create file '" << name << "'" << endl;
      return;
   }

   output << "\"#\",\"id\",\"TC\",\"alias\"" << endl;
   for (int i = 0; itr != m_map.end(); i++, itr++) {
      const string & current_key = (*itr).first;
      const repository_entry_t & current_value = (*itr).second;
      output << "\"" << i+1 << "\",\"" << current_key << "\",\"" << current_value.toggle_count 
         << "\",\"" << current_value.alias << "\"" << endl;
   }

   output.close();
}

bool psc_obj_repository::conditional_hit(string cond_str, int at_line, bool cond_value)
{
   char line_[8];
   string id = cond_str;
   sprintf(line_, "%d", at_line);
   id += line_;
   
#ifdef DEBUG_POWER_LEVEL2
   cerr << "\t[psc_obj_repository]: Conditional hit -> " << id << endl;
#endif
   
   cond_stat_map::iterator itr = m_cond.find(id); // look for the id in the conditional map

   if (itr == m_cond.end()) {
      // first hit
      cond_statement_t cond_entry;
      cond_entry.hit_count = 1;
      m_cond[id] = cond_entry;
   } else {
      m_cond[id].hit_count++; // increment the hit count
   }
   
   return( cond_value );
}

void psc_obj_repository::print_cond_hits()
{
   cond_stat_map::const_iterator itr = m_cond.begin();

   cerr << "---  Conditional Entries  ---" << endl;

   for (int i = 0; itr != m_cond.end(); i++, itr++) {
      const string & current_key = (*itr).first;
      const cond_statement_t & current_value = (*itr).second;      
      cerr << " " << i+1 << ". id=\"" << current_key << "\" \thit_count=" <<
         current_value.hit_count << endl;
   }
   
   cerr << "----------------------------------" << endl;
}

double psc_obj_repository::average_toggle_count()
{
#ifdef DEBUG_POWER_LEVEL3
   cerr << "\t[psc_obj_repository]: Computing the average toggle count" << endl;
#endif

   int i;
   unsigned long long int accum; // TC accumulator
   double avg;
   repository_map::const_iterator itr = m_map.begin();

   accum = 0;
   avg = 0.0;
   
   for ( i = 0; itr != m_map.end(); i++, itr++ ) {
      const repository_entry_t & current_value = (*itr).second;
      if ( current_value.valid_pointer )
         accum += current_value.pobj->get_toggle_count();
      else
         accum += current_value.toggle_count;
   }

   avg = (accum / i);

   return( avg );
}

unsigned long long int psc_obj_repository::total_toggle_count()
{
#ifdef DEBUG_POWER_LEVEL3
   cerr << "\t[psc_obj_repository]: Computing the total toggle count" << endl;
#endif

   unsigned long long int total = 0; // TC accumulator
   repository_map::const_iterator itr = m_map.begin();

   
   for ( int i = 0; itr != m_map.end(); i++, itr++ ) {
      const repository_entry_t & current_value = (*itr).second;
      if ( current_value.valid_pointer )
         total += current_value.pobj->get_toggle_count();
      else
         total += current_value.toggle_count;
   }

   return( total );
}

void psc_obj_repository::switching_power_report()
{
	repository_map::const_iterator it;
	double Vdd2, netpower;

	// voltage^2
	Vdd2 = psc_objinfo_base::techlib.get_voltage();
	Vdd2 *= Vdd2;

	it = m_map.begin();
	
	cerr << "--- Switching Power ---" << endl;

	for ( ; it != m_map.end() ; it++ ) {
      const string & key = (*it).first;
		const repository_entry_t & entry = (*it).second;
		netpower = ((entry.nload + entry.wload) * entry.toggle_rate);
		netpower *= Vdd2 / 2;
		netpower *= psc_objinfo_base::techlib.get_dynpwr_unit();
		cerr << key << "(" << entry.alias << "): " << netpower << "W" << endl;
	}

	cerr << "--" << endl;
	cerr << "TOTAL: " << get_switching_power() << "W" << endl;
   cerr << "-----------------------" << endl;
}

double psc_obj_repository::get_switching_power()
{
	repository_map::const_iterator it;
	double Vdd, power;	

	Vdd = psc_objinfo_base::techlib.get_voltage();
	it = m_map.begin();

	for ( power = 0.0 ; it != m_map.end() ; it++ ) {
		const repository_entry_t & entry = (*it).second;
		power += ((entry.nload + entry.wload) * entry.toggle_rate);
	}

	power *= (Vdd * Vdd) / 2;
	power *= psc_objinfo_base::techlib.get_dynpwr_unit();

	return( power );
}

void psc_obj_repository::full_sampling()
{
   // if this method is run during elaboration,
   // all simulation will be monitored
   m_activity_sampler->set_full_sampling( true );
}

void psc_obj_repository::enable_status_quo( unsigned int interval, ostream &output )
{
   // this call is allowed only before the simulation starts
   m_activity_sampler->enable_status_quo( interval, output );
}

void psc_obj_repository::set_samples_pred_range( int min, int max )
{
   m_activity_sampler->set_pred_samples_range( min, max );
}

void psc_obj_repository::set_samples_mon_range( int min, int max )
{
   m_activity_sampler->set_mon_samples_range( min, max );
}

void psc_obj_repository::set_num_first_samples( int num )
{
   m_activity_sampler->set_num_first_samples( num );
}

void psc_obj_repository::set_sampling_threshold( double val )
{
   m_activity_sampler->set_threshold( val );
}


} // namespace psc_power_base

