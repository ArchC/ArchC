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

  psc_obj_rep.h -- The psc_obj_repository class.

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_OBJ_REP
#define PSC_OBJ_REP

#include <systemc.h>
#include <string>
//#include <ext/hash_map>
#include <unordered_map>

#include "debug_power.h"
//#include "base/psc_objinfo_if.h"
//#include "base/psc_sampler.h"

#include "psc_objinfo_if.h"
#include "psc_sampler.h"


using namespace std;
//using namespace __gnu_cxx;


// The code below is necessary to use as the hash_map's key the type 'string',
// instead of 'const char *'
//namespace __gnu_cxx
//{
//   template<> struct hash< std::string >
//   {
//      size_t operator()( const std::string& s ) const
//      {
//	 return hash< const char* >()( s.c_str() );
//      }
//   };
//} // namespace __gnu_cxx

namespace psc_power_base
{

// classes defined in this module
class psc_obj_repository;

// initial bucket count for the hash maps
const int NUM_ENTRIES_OBJ = 50;
const int NUM_ENTRIES_COND = 25;

struct repository_entry_t {
	repository_entry_t()
	{
		alias = "not set";
		toggle_count = 0;
		toggle_rate = 0.0;
		pobj = NULL;
		valid_pointer = false;
		is_net = false;
		wload = nload = ndelay = 0.0;
		sp0 = sp1 = 0.0;
		fanout = 0;
		techinfo = false;
	}
	
   string alias;
   unsigned long long int toggle_count;
	double toggle_rate;
	double sp0;
	double sp1;
   psc_objinfo_if *pobj;	// pointer to the power object
   bool valid_pointer; 		// true if pobj is valid
	bool is_net; 				// only sc_signal fall into this category
	double wload;				// wire load
	double nload;				// net load
	double ndelay;				// net delay (used to index power tables)
	int fanout;					// number of connections to this net
	bool techinfo;				// true if this obj contains info from techlib
};

typedef struct {
   unsigned long long int hit_count;
} cond_statement_t;
   
struct eqstr 
{
   bool operator () (const string & s1, const string & s2) const
   {
      return(s1 == s2);
   }
};

//using __gnu_cxx::hash;

//typedef hash_map<string, repository_entry_t, hash<string>, eqstr> repository_map;
//typedef hash_map<string, cond_statement_t, hash<string>, eqstr> cond_stat_map;

typedef unordered_map<string, repository_entry_t, hash<string>, eqstr> repository_map;
typedef unordered_map<string, cond_statement_t, hash<string>, eqstr> cond_stat_map;


// ----------------------------------------------------------------------------
//  CLASS : psc_obj_repository
//  
// ----------------------------------------------------------------------------
class psc_obj_repository : public psc_obj_repository_base {
	public:

		// constructor
		psc_obj_repository()
			: m_map(NUM_ENTRIES_OBJ), m_cond(NUM_ENTRIES_COND)
			{
				m_activity_sampler = new psc_sampler( "activity_sampler" );
				m_activity_sampler->set_power_db( this );

#ifdef DEBUG_POWER_L3
				cerr << "\t[psc_obj_repository]: Creating repository -> Initial bucket count is " << m_map.bucket_count() << endl;
				cerr << "\t[psc_obj_repository]: Creating conditional map -> Initial bucket count is " << m_cond.bucket_count() << endl;
#endif
			}

		// destructor
		~psc_obj_repository()
		{
			if ( m_activity_sampler )
				delete( m_activity_sampler );

#ifdef DEBUG_POWER_L3
			cerr << "\t[psc_obj_repository]: Destroying repository -> size=" << m_map.size() << " - bucket_count=" << m_map.bucket_count() << endl;
			cerr << "\t[psc_obj_repository]: Destroying conditonal map -> size=" << m_cond.size() << " - bucket_count=" << m_cond.bucket_count() << endl;
#endif
		}

		inline
		bool is_to_sample() const
			{
				return( m_activity_sampler->is_to_sample() );
			}

		// other methods
		bool add_register(const char *entry_name);
		bool add_register(const char *entry_name, repository_entry_t &e);
		bool unregister(const char *entry_name);
		repository_entry_t *get_entry(const string & name );
		bool update_registry(const char *entry_name, repository_entry_t &e);
		void print_entries();
		void switching_power_report();
		void write_entries_to_csv_file(const char *name);
		bool conditional_hit(string cond_str, int at_line, bool cond_value);
		void print_cond_hits();
		double average_toggle_count();
		unsigned long long int total_toggle_count();
		void full_sampling();
		void enable_status_quo( unsigned int interval, ostream &output );
		void set_samples_pred_range( int min, int max );
		void set_samples_mon_range( int min, int max );
		void set_num_first_samples( int num );
		void set_sampling_threshold( double val );

		double get_switching_power();

	protected:

		// attributes   
		repository_map	m_map;				// objects are registered in this member
		cond_stat_map m_cond; 				// conditional statements captured are kept in this hash map
		psc_sampler *m_activity_sampler;	// sample transition activity
};

}; // psc_power_base

#endif
