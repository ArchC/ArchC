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

  psc_sampler.h -- sample transition counts.

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_SAMPLER_H
#define PSC_SAMPLER_H

#include <systemc.h>

//#include "base/psc_obj_rep_base.h"
#include "psc_obj_rep_base.h"

using psc_power_base::psc_obj_repository_base;


// ----------------------------------------------------------------------------
// Default values used if none is specified explicitly
// ----------------------------------------------------------------------------

// minimum and maximum samples using the predicted average
// DEFAULT_MIN_SAMPLES_PRED <= x <= DEFAULT_MAX_SAMPLES_PRED
const int DEFAULT_MIN_SAMPLES_PRED = 1;
const int DEFAULT_MAX_SAMPLES_PRED = 256;

// minimum and maximum samples monitoring the transition activity
// DEFAULT_MIN_SAMPLES_MON <= x <= DEFAULT_MAX_SAMPLES_MON
const int DEFAULT_MIN_SAMPLES_MON = 1;
const int DEFAULT_MAX_SAMPLES_MON = 128;

const int DEFAULT_FIRST_SAMPLES_NUM = DEFAULT_MAX_SAMPLES_MON; 	// default number of samples used to compute the first mean
const double DEFAULT_SAMPLE_TIME = 10000;		// in default time units
const double DEFAULT_THRESHOLD = 0.005; 		// +/- 0.5% of global average
#if 0
const double DEFAULT_RESET_THRESHOLD = 5.0;	// +/- 500.0% of global average
const int RESET_SAMPLER_COUNTER = 3;
#endif

// ----------------------------------------------------------------------------
//  MODULE : psc_sampler
//
// This class is responsible for collecting samples of transition counts, in 
// order to calculate the average and decide whether to use a pre-calculated
// average or continue to collect samples
// ----------------------------------------------------------------------------
SC_MODULE(psc_sampler)
{
   // processes
   void execute();
   void status_quo();	// get the current switching activity figure within determined times

   SC_CTOR(psc_sampler)
   {
      check_uniqueness();

      init_defaults();

#ifdef DEBUG_POWER_L2
      cerr << "[psc_sampler]: " << this->name() << endl;
#endif

      SC_METHOD(execute);

      SC_METHOD(status_quo);
   }

public:
   
   ~psc_sampler()
   {
#ifdef DEBUG_POWER_L3
      cerr << "[psc_sampler]: Destroying object" << endl;
      cerr << "Final simulation time: " << sc_simulation_time() << endl;
      cerr << "total_tmon = " << m_total_tmon << endl;
      cerr << "Final monitoring window: " << m_t_end_mon << endl;
#endif

      if ( m_t_end_mon > sc_simulation_time() ) {
         cerr << "Adjusting monitored time" << endl;
         double real_t_mon = m_total_tmon - (m_t_end_mon - sc_simulation_time());
         cerr << "Monitored time: " << real_t_mon << endl;
      }
   }

   static bool m_instantiated;			// used to guarantee that only one instance of this object is created

   inline
   void set_power_db( psc_obj_repository_base *r )
   { m_psc_db = r; }

   inline
   bool is_to_sample() const
   { return( m_to_sample ); }

   inline
   void set_threshold( double t )
   { m_threshold = t; }

   inline
   void set_num_first_samples( int n )
   { m_num_first_samples = n; }

   inline
   bool is_full_sampling() const
   { return( m_full_sampling ); }

   inline
   void set_pred_samples_range( int min, int max )
   { 
      m_min_samples_pred = min;
      m_max_samples_pred = max;
   }

   inline 
   void set_mon_samples_range( int min, int max )
   {
      m_min_samples_mon = min;
      m_max_samples_mon = max;
   }

   inline
   double get_global_mean() const
   { return( m_global_pred_avg ); }
   
   void set_full_sampling( bool b );
   double get_monitored_time() const;	// effectively monitored monitoring
   double get_predicted_time() const;	// effectively stopped monitoring
   void enable_status_quo( unsigned int interval, ostream &output );
   
protected:
   psc_obj_repository_base *m_psc_db; 	// the PowerSC objects database   
   bool m_to_sample;							// define when the objects must gather transition activity
   bool m_first_mean;						// first samples to compute the first mean
   bool m_initial_samples;					// initial monitored samples
   double m_threshold;                 // threshold over global average
#if 0
   double m_reset_threshold;           // threshold to reset the sampler
   int m_reset_counter;
#endif
   double m_ts;                        // time of each sample (in default time units)
   int m_num_first_samples;				// number of samples used to compute the first mean
   double m_tmon;								// time monitoring transition activity
   int m_n_mon;								// number of samples monitoring the activity
   double m_total_tmon;						// total monitored simulation time
   bool m_first_mon;							// first time monitoring after initial samples
   double m_tpred;							// time using predicted average
   int m_n_pred;								// number of samples using predicted average
   double m_global_pred_avg;				// predicted average (monitored+stopped)
   double m_local_avg;						// current local average is equal to the last local monitored average
   long long int m_mon_tc;					// total toggle count actually monitored
   double m_pred_tc;							// total predicted toggle count
   double m_t_begin_mon;					// beginning time of the monitoring window
   double m_t_end_mon;						// ending time of the monitoring window
   double m_t_begin_pred;					// beginning time of the predicted
   double m_t_end_pred;						// ending time of the predicted window
   double m_total_tpred;					// total predicted simulation time
   bool m_full_sampling;					// if true all simulation is monitored
   bool m_status_quo;						// define if status_quo process is enabled
   unsigned int m_status_interval;		// time intervals between getting the status
   bool m_wait_first_interval;
   long long int  m_n_last_toggle_count;// last toggle seen by the status_quo process
   ostream *m_status_stream;
   int m_min_samples_pred;					// minimum samples predicting the switching activity
   int m_max_samples_pred;					// maximum samples predicting the switching activity
   int m_min_samples_mon;					// minimum samples monitoring the switching activity;
   int m_max_samples_mon;					// maximum samples monitoring the switching activity;   


   // protected methods
   void init_defaults(); // set initial default values
   bool threshold_check( double local, double pred ); // checks if local average respects the defined threshold
   
   void increase_mon_samples();
   void decrease_mon_samples();

   void increase_pred_samples();
   void decrease_pred_samples();

   double get_global_toggle_count();	// actual+predicted toggle count
   double get_global_average();
      
#if 0
   bool reset_threshold_check( double local, double pred ); // check reset threshold
   void reset_sampler();					// restart number of samples as the beginning
#endif

private:

   void check_uniqueness();
};

#endif
