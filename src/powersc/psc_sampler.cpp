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

  psc_sampler.cpp -- sample transition counts.

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#include "psc_sampler.h"


bool psc_sampler::m_instantiated; // static member


void psc_sampler::execute()
{
   long long int local_tc;
   double local_avg, local_time;

   // if m_full_sampling is true, all simulation will be monitored since
   // the conditions evaluates to true until the end of the simulation
   if ( m_full_sampling ) {
#ifdef DEBUG_POWER_L2
      cerr << "[psc_sampler]: Full simulation monitoring" << endl;
#endif

      m_to_sample = true;
      next_trigger(); // never trigger this process again
      return;
   }
   
   // if this is the first time running this process, then
   // it will be computed the first transition mean after
   // some time monitoring the design
   if ( m_initial_samples ) {
      m_initial_samples = false;
      m_tmon = m_num_first_samples * m_ts;
      next_trigger( m_tmon, SC_NS );
      m_total_tmon = m_tmon;
      
      cerr << sc_time_stamp() << " - Monitoring the initial samples: samples(" 
         << m_num_first_samples << ") time(" << m_tmon << ")" << endl;
      return;
   }

   // if the first mean is to be calculated, then we can use
   // the actual total TC and divide it by the current simulation time
   if ( m_first_mean ) {
      m_first_mean = false;
      m_first_mon = true;
      m_mon_tc = m_psc_db->total_toggle_count();
      m_t_end_mon = sc_simulation_time();

      // first predicted average has the same value than the actual average
      m_global_pred_avg = (double)m_mon_tc / m_t_end_mon;
      m_local_avg = m_global_pred_avg; // first local avg is equal to first predicted avg
      cerr << sc_time_stamp() << " - first computed average: " << m_global_pred_avg << endl;      
   } else
      m_global_pred_avg = get_global_average();
   
   m_to_sample = ! m_to_sample;

   if ( m_to_sample ) {
      if ( m_first_mon ) {
         decrease_mon_samples();
         m_tmon = m_n_mon * m_ts;
         m_first_mon = false;
      } else {
         local_tc = m_psc_db->total_toggle_count() - m_mon_tc;
         local_time = m_t_end_mon - m_t_begin_mon;
         local_avg = (double)local_tc / local_time;
         
         m_mon_tc += local_tc;

         if ( threshold_check(local_avg, m_global_pred_avg) ) {
            decrease_mon_samples();
            increase_pred_samples();
#if 0
            m_reset_counter = RESET_SAMPLER_COUNTER;
#endif
         } else {
            increase_mon_samples();
            decrease_pred_samples();
#if 0
            // if difference is too big we restart the sampler
            if ( ! reset_threshold_check(local_avg, m_global_pred_avg) ) {
               cerr << "m_reset_counter: " << m_reset_counter << endl;
               if ( --m_reset_counter == 0 ) {
                  reset_sampler();
                  m_reset_counter = RESET_SAMPLER_COUNTER;
               }
            } else
               m_reset_counter = RESET_SAMPLER_COUNTER;
#endif    
         }
         m_local_avg = local_avg;

         cerr << "Local average: " << local_tc << "/" << local_time 
            << " = " << local_avg << endl;
      }

      // set the period of the next monitoring
      m_tmon = m_n_mon * m_ts;
      m_t_begin_mon = sc_simulation_time();
      m_t_end_mon = m_t_begin_mon + m_tmon;
         
      next_trigger( m_tmon, SC_NS );
      m_total_tmon += m_tmon;

      
      cerr << sc_time_stamp() << " - Resume monitoring: samples(" << m_n_mon
         << ") time(" << m_tmon << ")" << endl;
   } else {
      m_tpred = m_n_pred * m_ts;
      m_t_begin_pred = sc_simulation_time();
      m_t_end_pred = m_t_begin_pred + m_tpred;

      m_pred_tc += m_local_avg * m_tpred;

      next_trigger( m_tpred, SC_NS );
      m_total_tpred += m_tpred;

      cerr << "-> The last local average is " << m_local_avg << endl;
      cerr << sc_time_stamp() << " - Stopping monitoring: samples(" << m_n_pred
         << ") time(" << m_tpred << ")" << endl;
   }

   cerr << "[t=" << sc_time_stamp() << "] - current global average: " << m_global_pred_avg 
      << " - m_total_tpred: " << m_total_tpred << " - m_total_tmon: " << m_total_tmon << endl;
}

void psc_sampler::status_quo()
{
   if ( ! m_status_quo ) {
#ifdef DEBUG_POWER_L2
      cerr << "[psc_sampler]: " << "switching activity status quo retrieval disabled" << endl;
#endif
      next_trigger(); // nevermore during the simulation
      return;
   } else if ( m_wait_first_interval ) {
      // wait for the first interval
#ifdef DEBUG_POWER_L2
      cerr << "[psc_sampler]: " << "switching activity status quo retrieval enabled" << endl;
#endif
      next_trigger( m_status_interval, SC_NS );
      m_wait_first_interval = false;
      m_n_last_toggle_count = 0;
      return;
   }

   long long int total_TC, interval_TC;
   double curr_sim_time, curr_tc_avg, interval_avg;
   
   total_TC = m_psc_db->total_toggle_count();
   
   if ( m_full_sampling ) {
      curr_sim_time = sc_simulation_time();
      
      interval_TC = total_TC - m_n_last_toggle_count;
      interval_avg = (double)interval_TC / (double)m_status_interval;
      
      curr_tc_avg = (double)total_TC / curr_sim_time;

      *m_status_stream << "t=" << sc_time_stamp() << " ; Global TC average at this time: " 
         << curr_tc_avg << " ; Last interval average: " << interval_avg << endl;
   } else {
      // if the first mean is not yet calculated, so all simulation
      // time until now is monitored. Otherwise, we use the global predicted
      // average
      if ( m_first_mean ) {
         curr_tc_avg = (double)total_TC / sc_simulation_time();
         *m_status_stream << "t=" << sc_time_stamp() << " ; Global TC average at this time " 
            << "[firstsamples]: " <<  curr_tc_avg << endl;
      } else {
            *m_status_stream << "t=" << sc_time_stamp() << " ; Global TC average at this time "
               << ((m_to_sample) ? "[monitoring]: " : "[stopped]: ") << get_global_average() << endl;
      }
   }

   m_n_last_toggle_count = total_TC; // update the value

   next_trigger( m_status_interval, SC_NS );
}

bool psc_sampler::threshold_check( double local, double pred )
{
   double max, min, ratio, diff;

   max = (local > pred) ? local : pred;
   min = (local > pred) ? pred : local;

   ratio = (max / min);
   diff = ratio - 1.0;

   cerr << "threshold_check - Local: " << local << " - Pred: " << pred
      << " - diff: " << diff << " (" << ((diff <= m_threshold) ? "OK": "Not OK") << ")" << endl;

   return( diff <= m_threshold );
}



void psc_sampler::init_defaults()
{
   m_psc_db = NULL;

   m_min_samples_pred = DEFAULT_MIN_SAMPLES_PRED;
   m_max_samples_pred = DEFAULT_MAX_SAMPLES_PRED;

   m_min_samples_mon = DEFAULT_MIN_SAMPLES_MON;
   m_max_samples_mon = DEFAULT_MAX_SAMPLES_MON;
   
   m_to_sample = true;	// start sampling by default
   m_first_mean = true;
   m_initial_samples = true;
   m_threshold = DEFAULT_THRESHOLD;
#if 0
   m_reset_threshold = DEFAULT_RESET_THRESHOLD;
   m_reset_counter = RESET_SAMPLER_COUNTER;
#endif
   m_ts = DEFAULT_SAMPLE_TIME;
   m_global_pred_avg = 0.0;
   m_local_avg = 0.0;
   
   m_num_first_samples = DEFAULT_FIRST_SAMPLES_NUM;
   
   m_tmon = 0.0;
   m_n_mon = m_num_first_samples;
   m_total_tmon = 0.0;
   m_first_mon = false;
   
   m_tpred = 0.0;
   m_n_pred = m_min_samples_pred;

   m_pred_tc = 0.0;
   m_mon_tc = 0;
   m_t_begin_mon = 0.0;
   m_t_end_mon = 0.0;
   m_t_begin_pred = 0.0;
   m_t_end_pred = 0.0;
   m_total_tpred = 0.0;

   m_full_sampling = false; // don't sample all the simulation by default

   m_status_quo = false; // don't get the status quo by default
   m_wait_first_interval = true;
   m_status_stream = NULL;
   m_n_last_toggle_count = 0; // last toggle count when entrying the status_quo process
}

void psc_sampler::check_uniqueness()
{
   if ( ! m_instantiated )
      m_instantiated = true;
   else {
      cerr << "Error: only one instance of psc_sampler is allowed throughout the simulation.\n"
         << "Stopping simulation now!" << endl;
      sc_stop();
   }
}

void psc_sampler::increase_mon_samples()
{
   int tmp = m_n_mon * 2;
   m_n_mon = (tmp <= m_max_samples_mon) ? tmp : m_n_mon;
}

void psc_sampler::decrease_mon_samples()
{
   int tmp = m_n_mon / 2;
   m_n_mon = (tmp >= m_min_samples_mon) ? tmp : m_n_mon;
}

void psc_sampler::increase_pred_samples()
{
   int tmp = m_n_pred * 2;
   m_n_pred = (tmp <= m_max_samples_pred) ? tmp : m_n_pred;
}

void psc_sampler::decrease_pred_samples()
{
   int tmp = m_n_pred / 2;
   m_n_pred = (tmp >= m_min_samples_pred) ? tmp : m_n_pred;
}

void psc_sampler::set_full_sampling( bool b )
{
   // if the simulation started, ignore this call
   if ( sc_simulation_time() > 0 ) {
      cerr << "Warning: trying to set full sampling after the simulation started. Nothing done." << endl;
      return;
   }
      
   m_full_sampling = b;
}

double psc_sampler::get_monitored_time() const
{
   double mon_time, sim_time = sc_simulation_time();
   
   if ( m_t_end_mon > sim_time ) { 
      // monitored window end is greater than current simulation time
      mon_time = m_total_tmon - (m_t_end_mon - sim_time);
   } else if ( m_total_tmon > sim_time )
      // current simulation time is before the end of the window
      mon_time = sim_time;
   else
      mon_time = m_total_tmon;
   
   return( mon_time );
}

double psc_sampler::get_predicted_time() const
{
   double pred_time, sim_time = sc_simulation_time();
   
   if ( m_t_end_pred > sim_time ) { 
      // stopped window end is greater than current simulation time
      pred_time = m_total_tpred - (m_t_end_pred - sim_time);
   } else if ( m_total_tpred > sim_time )
      // current simulation time is before the end of the window
      pred_time = sim_time;
   else
      pred_time = m_total_tpred;
   
   return( pred_time );
}


void psc_sampler::enable_status_quo( unsigned int interval, ostream &output )
{
   // if the simulation started, ignore this call
   if ( sc_simulation_time() > 0 ) {
      cerr << "Warning: trying to enable status quo after the simulation started. Nothing done." << endl;
      return;
   }

   m_status_quo = true;
   m_status_interval = interval;
   m_status_stream = &output;
}

double psc_sampler::get_global_toggle_count()
{
   double global_tc = m_pred_tc + m_mon_tc;
   return( global_tc );
}

/**
  This method calculates the global transition activity average.
  The global mean takes into account both means (monitored and 
  predicted), and also these values are weighted according to
  their individual simulation times.
  */
double psc_sampler::get_global_average()
{
   double mon_weight, pred_weight, total_sim_time, mon_avg, 
      pred_avg, global_average, cur_total_tmon;

   // total simulation time
   cur_total_tmon = get_monitored_time();
   total_sim_time = cur_total_tmon + m_total_tpred;
   
   // compute the weights
   mon_weight = cur_total_tmon / total_sim_time;
   pred_weight = m_total_tpred / total_sim_time;
   
   // compute the global average
   mon_avg = (double)m_psc_db->total_toggle_count() / cur_total_tmon;
   pred_avg = m_pred_tc / m_total_tpred;
   
   global_average = (mon_avg * mon_weight) + (pred_avg * pred_weight);

   if ( sc_simulation_time() > 10.0e+6 ) {
      cerr << "---------------------------------------" << endl;
      cerr << "-- m_total_tmon = " << m_total_tmon << endl;
      cerr << "-- m_total_tpred = " << m_total_tpred << endl;
      cerr << "-- total_sim_time = " << total_sim_time << endl;
      cerr << "-- mon_weight = " << mon_weight << endl;
      cerr << "-- pred_weight = " << pred_weight << endl;
      cerr << "-- mon_avg = " << mon_avg << endl;
      cerr << "-- pred_avg = " << pred_avg << endl;
      cerr << "-- global_avg = " << global_average << endl;
      cerr << "---------------------------------------" << endl;
   }
   
   return( global_average );
}

#if 0
bool psc_sampler::reset_threshold_check( double local, double pred )
{
   double max, min, ratio, diff;

   max = (local > pred) ? local : pred;
   min = (local > pred) ? pred : local;

   ratio = (max / min);
   diff = ratio - 1.0;

   cerr << "reset_threshold_check - diff: " << diff << " (" << ((diff <= m_reset_threshold) ? "OK, pass": "Not OK, resetting") << ")" << endl;

   return( diff <= m_reset_threshold );
}

void psc_sampler::reset_sampler()
{
   m_n_mon = m_max_samples_mon;
   m_n_pred = m_min_samples_pred;
}
#endif

