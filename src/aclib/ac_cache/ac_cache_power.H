#ifdef POWER_SIM
#include <powersc.h>
#include <stdlib.h>

/* Data struct definition. You should think that it is a row in a table. Each profile will have a certain number of tables. 
     The basic idea is use a profile, with a pre-fixed number of operational frequencies. Each frequency, with a specific 
     table of values */

// This group should be parameters, not defines
#define CACHE_WINDOW_REPORT
#define CACHE_WINDOW_REPORT_FILE "window_power_report"
#define CACHE_POWER_TABLE_FILE "powersc/table_cache_8k_2w_45nm.csv"

#define CACHE_START_WINDOW_SIZE 10000

#define CACHE_MAX_LINESIZE_CSV_FILE 10240 
#define CACHE_MAX_POWER_STATS_NAME_SIZE 30
#define CACHE_MAX_POWER_STATS_DESCR_SIZE 140

//#define CACHE_POWER_DEBUG

enum cache_type_line_t {
  CTL_NUM_PROFILE,
  CTL_PROFILE,
  CTL_IDLE,
  CTL_READ,
  CTL_WRITE,
  CTL_UNKNOWN
};

class cache_power_stats {
    private:
        struct profile
    {
            char power_stats_name[CACHE_MAX_POWER_STATS_NAME_SIZE];
            char power_stats_descr[CACHE_MAX_POWER_STATS_DESCR_SIZE];
            unsigned int freq;
            double freq_scale;
            double energy_scale;
      double idle_power;    //power when cache is idle.
      double read_energy;    //energy to read from the cache
      double write_energy;   //energy to write to the cache
        };

        struct power_stats_data
    {
            profile* p;
        };

        struct dynamic_data
    {
#ifdef CACHE_WINDOW_REPORT
            long long window_num_access;
            double window_energy;
            double window_power;
            long long window_count;
            unsigned int window_size;
      double last_window_time;
#endif

            long long total_num_access; 
            double total_active_energy;
            double total_power;

            unsigned int actual_profile;
            unsigned int num_profiles;
        };

        dynamic_data dyn;
        power_stats_data psc_data;

    #ifdef CACHE_WINDOW_REPORT
        FILE* out_window_power_report;
        #endif

        #ifdef CACHE_POWER_DEBUG 
        FILE* debug_file;
        int contador_debug;
        #endif

    string cache_name;
    string generate_cache_name(const int id) {
      //to_string() needs -std=c++11 to compile
      cache_name = string("Proc") + to_string(id) + " cache";
      return cache_name;
    }
    

    public:
        psc_cell_power_info psc_info;

        // Constructor
    cache_power_stats(const int proc_id):
    psc_info(generate_cache_name(proc_id), "Cache") 
    {
      PSC_NUM_FIRST_SAMPLES(0x7FFFFFFF);
    
        char prefix_table[1024];
        sprintf(prefix_table, "%s/share/archc/%s", getenv("ARCHC_PREFIX"), CACHE_POWER_TABLE_FILE);
        init(prefix_table);

      /*Initialize power state using profile 0*/
            dyn.actual_profile = 0;
            dyn.total_num_access = 0;
            dyn.total_active_energy = 0;
            dyn.total_power = 0;

      #ifdef CACHE_WINDOW_REPORT
            dyn.window_size = CACHE_START_WINDOW_SIZE;
            dyn.window_num_access = 0;
            dyn.window_energy = 0;
            dyn.window_power = 0;
            dyn.window_count = 0;
            dyn.last_window_time = 0;

            char filename[512];
            strcpy(filename, CACHE_WINDOW_REPORT_FILE);
            strcat(filename, "_");
            strcat(filename, cache_name.c_str());
            strcat(filename, ".csv");
            out_window_power_report = fopen(filename, "w");
            if (out_window_power_report == NULL) {
                perror("Couldn't open specified out_window_power_report file");
                exit(1);
            }
            /****/
            #endif
            

            #ifdef CACHE_POWER_DEBUG 
            strcpy (filename, "debug_power");
            strcat(filename, "_");
            strcat(filename, cache_name.c_str());
            strcat(filename, ".txt");
            
            debug_file = fopen(filename, "w");
            if (debug_file == NULL) {
                perror("Couldn't open specified debug file");
                exit(1);
            }
            contador_debug = 0;
                        
            print_psc_data();
            #endif
    }

        // Destructor
        ~cache_power_stats() {
            free(psc_data.p);

      #ifdef CACHE_WINDOW_REPORT
            fclose(out_window_power_report);
            #endif

            #ifdef CACHE_POWER_DEBUG
            fclose(debug_file);
            #endif
        }

        double get_power() {
            return psc_info.get_power();
        }

    // command == 0 -> read
    // command == 1 -> write
        double get_energy_access(int command, int profile)
    {
      double command_energy = (command) ? psc_data.p[profile].write_energy : psc_data.p[profile].read_energy;
            double energy = command_energy * psc_data.p[profile].energy_scale;

            #ifdef CACHE_POWER_DEBUG
            if (contador_debug < 200)
            {
                fprintf(debug_file,"\nGetting access energy.");
                fprintf(debug_file,"\nprofile: %d\t command: %d\t command_energy*energy_scale = %f * %f" , profile, command, command_energy, psc_data.p[profile].energy_scale);
                fprintf(debug_file,"\nReturning: %f", energy);
                contador_debug++;
            }   
            #endif

            return energy;
        }

    cache_type_line_t type_line(int line, int num_profiles)
    {
      if (num_profiles == 0)             return CTL_NUM_PROFILE;
      else if (line <= num_profiles + 1) return CTL_PROFILE;
      else if (line == num_profiles + 2) return CTL_IDLE;
      else if (line == num_profiles + 3) return CTL_READ;
      else if (line == num_profiles + 4) return CTL_WRITE;
      else                               return CTL_UNKNOWN;
    }

#ifdef CACHE_WINDOW_REPORT
        void incr_window_energy(double v) {
            dyn.window_energy += v;
        }

        void reset_window_data() {
            dyn.window_num_access = 0;
            dyn.window_energy = 0;
            dyn.window_power = 0;
        }

        void calc_window_power()
    {
      double window_total_time = sc_time_stamp().to_seconds() - dyn.last_window_time;
      dyn.last_window_time = sc_time_stamp().to_seconds();
      double window_active_time = dyn.window_num_access / (psc_data.p[dyn.actual_profile].freq_scale * psc_data.p[dyn.actual_profile].freq);
      dyn.window_power = (dyn.window_energy + (window_total_time - window_active_time) * psc_data.p[dyn.actual_profile].idle_power )/ window_total_time; 
        }

        void window_power_report()
    {
            //estimate_execution_time();
            fprintf(out_window_power_report, "%d,%.10lf,%lld,%.10lf,%.10lf\n", dyn.actual_profile, dyn.last_window_time,
      dyn.window_count, dyn.window_power, sc_time_stamp().to_double());
        }
#endif

        void incr_total_active_energy(double v) {
            dyn.total_active_energy += v;
        }



         double getEnergyPerCache()
        {
            return dyn.total_active_energy;
        }

    //void incr_execution_time(int num_access, int p)
    //{
        //  dyn.execution_time += num_access / (psc_data.p[dyn.actual_profile].freq * psc_data.p[dyn.actual_profile].freq_scale);
    //}

        void update_stat_power(int command)
    {
        dyn.total_num_access++;
            incr_total_active_energy(get_energy_access(command, dyn.actual_profile));
#ifdef CACHE_WINDOW_REPORT
            dyn.window_num_access++;
            incr_window_energy(get_energy_access(command, dyn.actual_profile));
            if (dyn.window_num_access == dyn.window_size) {
                dyn.window_count++;
                calc_window_power();
                window_power_report();
                reset_window_data();
            }
#endif
        }

        void calc_total_power()
        {
      double total_time = sc_time_stamp().to_seconds();
      double total_active_time = dyn.total_num_access / (psc_data.p[dyn.actual_profile].freq_scale * psc_data.p[dyn.actual_profile].freq);
            dyn.total_power = ( dyn.total_active_energy + (total_time - total_active_time) * psc_data.p[dyn.actual_profile].idle_power ) / total_time;

            #ifdef CACHE_POWER_DEBUG
      printf("Total accesses: %lld; Total time: %g; total active time: %g; total active energy:%g\n", dyn.total_num_access, total_time, total_active_time, dyn.total_active_energy);
            fprintf(debug_file,"\n\nCalculating total power = %f:", dyn.total_power);           
            #endif
        }

        void powersc_connect()
    {
            calc_total_power();
            psc_info.set_power(dyn.total_power);
            psc_info.set_level(PSC_RT_LEVEL);
            PSC_INSERT_CELL(psc_info);
        }

        void report()
        {
            PSC_REPORT_POWER;
        }

        char* next_strtok(const char* param, FILE* f, int pos_line)
    {
            char* pch = NULL;
            pch = strtok(NULL,param);
            if (pch == NULL) {
                printf("Error reading csv file, line %d. Unexpected format\n", pos_line);
                fclose(f);
                exit(1);
            }
            return pch;
        }

        // Read from file 
        void init(const char* filename)
    {
            FILE* f = NULL;
            char c = 0;
            char line[CACHE_MAX_LINESIZE_CSV_FILE];
            char aux[CACHE_MAX_LINESIZE_CSV_FILE];
            double value;
      char* res;

            unsigned int index = 0;
            unsigned int pos_line = 0;
            unsigned int valid_line = 0;
      unsigned int profile_id = 0;
            fpos_t pos;

            char* pch = NULL;

            f = fopen(filename, "r");
            if (f == NULL) {
                sprintf(aux, "Power file %s not found", filename);
                perror(aux);
                exit(1);
            }

      dyn.num_profiles = 0; // Set a default value 

      int state_id = 0;

            do {
                //fgetpos (f, &pos);
                res = fgets(line, CACHE_MAX_LINESIZE_CSV_FILE, f); // Get a new line from file
                if (feof(f)) break;
                pos_line++; // It says what line I am reading now
                pch = strtok(line, ",\""); // It parses the line

                if (pch[0] == '#'); // If it is a comment, ignore
                else if (pch == NULL) break;
                else
        { // Just found a valid new line
                    valid_line++;
                    // First Valid Line: number of profiles
          switch(type_line(valid_line, dyn.num_profiles)) {
            case CTL_NUM_PROFILE:
              dyn.num_profiles = atoi(pch);
                        psc_data.p = (profile *)malloc(sizeof(profile) * dyn.num_profiles);
              for (int i = 0; i < dyn.num_profiles; i++) {
                psc_data.p[i].idle_power = 0;
                psc_data.p[i].read_energy = 0;
                psc_data.p[i].write_energy = 0;
              }
            break;
            case CTL_PROFILE:
              profile_id = state_id++;

              if (profile_id >= dyn.num_profiles) {
                printf("Error: Invalid profile_id greater than num_profiles: %d > %d\n", 
                  profile_id, dyn.num_profiles);
              }
                        
              //pch = next_strtok(",\"", f, pos_line);
                        psc_data.p[profile_id].freq = atoi(pch);
                        
                        pch = next_strtok(",\"", f, pos_line);
                        psc_data.p[profile_id].freq_scale = atof(pch);
                        
                        pch = next_strtok(",\"", f, pos_line);
                        psc_data.p[profile_id].energy_scale = atof(pch);
                        
                        printf("%f",psc_data.p[profile_id].energy_scale);

                        
                        pch = next_strtok(",\"", f, pos_line);
                        strcpy(psc_data.p[profile_id].power_stats_name, pch);
                        
                        pch = next_strtok(",\"", f, pos_line);
                        strcpy(psc_data.p[profile_id].power_stats_descr, pch);
            break;
            case CTL_IDLE:
                        psc_data.p[0].idle_power = atof(pch);

                        for(int i = 1; i < dyn.num_profiles;i++) {
                            pch = next_strtok(",\"", f, pos_line);
                            psc_data.p[i].idle_power = atof(pch);
                        }
            break;
            case CTL_READ:
                        psc_data.p[0].read_energy = atof(pch);

                        for(int i = 1; i < dyn.num_profiles;i++) {
                            pch = next_strtok(",\"", f, pos_line);
                            psc_data.p[i].read_energy = atof(pch);
                        }
            break;
            case CTL_WRITE:
                        psc_data.p[0].write_energy = atof(pch);

                        for(int i = 1; i < dyn.num_profiles;i++) {
                            pch = next_strtok(",\"", f, pos_line);
                            psc_data.p[i].write_energy = atof(pch);
                        }
            break;
            default: // CTL_UNKNOWN
              sprintf(aux, "Too much values in power table %s.", filename);
              perror(aux);
              exit(1);
            break;
          }
                }
            } while(!feof(f));

            fclose(f);
        }

        void print_psc_data() {
            #ifdef CACHE_POWER_DEBUG
            
      int i = 0, p = 0;
            for(p = 0; p < dyn.num_profiles; p++) {
                printf("\nProfile %d\n", p);
                printf("Name: %s\n", psc_data.p[p].power_stats_name);
                printf("Description: %s\n", psc_data.p[p].power_stats_descr);
                printf("Frequency: %d\n\n", psc_data.p[p].freq);
                printf("Frequency: %f\n\n", psc_data.p[p].freq_scale);

                
                fprintf(debug_file,"\nProfile %d\n", p);
                fprintf(debug_file,"Name: %s\n", psc_data.p[p].power_stats_name);
                fprintf(debug_file,"Description: %s\n", psc_data.p[p].power_stats_descr);
                fprintf(debug_file,"Frequency: %d\n\n", psc_data.p[p].freq);
                fprintf(debug_file,"Frequency: %f\n\n", psc_data.p[p].freq_scale);
                
                
            }

            printf("Cache State       ");
            fprintf(debug_file,"Cache State");
            for(p = 0; p < dyn.num_profiles; p++) {
                printf(" | Power Profile %d", p);
                fprintf(debug_file," | Power Profile %d", p);
            }
            printf("\n");
            fprintf(debug_file,"\n");
      printf("Idle Power        ");
      for(p = 0; p < dyn.num_profiles; p++) {
        printf(" | %15.3lf", psc_data.p[p].idle_power);
      }
      printf("\n");
      fprintf(debug_file,"\n");
      printf("Cache read energy ");
      for(p = 0; p < dyn.num_profiles; p++) {
        printf(" | %15.3lf", psc_data.p[p].read_energy);
        fprintf(debug_file," | %15.3lf", psc_data.p[p].read_energy);
      }
      printf("\n");
      fprintf(debug_file,"\n");
      printf("Cache write energy");
      for(p = 0; p < dyn.num_profiles; p++) {
        printf(" | %15.3lf", psc_data.p[p].write_energy);
        fprintf(debug_file," | %15.3lf", psc_data.p[p].write_energy);
      }
      printf("\n");
      fprintf(debug_file,"\n");

      #endif
        }
};

#endif
