/*
Copyright (C) 2013  
Fabien Gaud <fgaud@sfu.ca>, Baptiste Lepers <baptiste.lepers@inria.fr>,
Fabien Mottet <fabien.mottet@inria.fr>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 or later, as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <getopt.h>

#include <time.h>
#include <sys/time.h>

//Specify the debug level
#define DEBUG_LEVEL -1

#include "master_slave.h"
#include "slg_master.h"
#include "debug_tools.h"
#include "stats_utils.h"

// Socket file descriptor
struct sockaddr_in sock_w;
unsigned int len_w;

// Receiving socket
int rs;
struct sockaddr_in server_addr;

// My name
char name[256];

master_stats_t * master_stats;

struct timeval start_time;
struct timeval stop_time;

//////////////////Global var////////////////////////////
slaves_infos_t * slaves_infos;

unsigned int nb_slaves;
char * targets;
unsigned int nb_waves = 0;
unsigned int nb_iterations_in_a_wave = 0;
unsigned int nb_clients = 0;
unsigned int duration = 0;
unsigned int nb_msg_per_connection = 0;
unsigned int delay = 0;

int percent_stats_kept_master = 100;

/** If not N-Copy only one server is targeted
 * with possibly multiple itf
 */
int N_copy = 0;


/** Warmup ? **/
static int warmup_length = 0;

slaves_stats_t * slaves_stats = NULL;
unsigned int nb_clients_min = 0;
unsigned int nb_clients_max = 0;
int step = 0;
char * slavesInfos;
char * config_file = NULL;

typedef struct {
   char* server;
   int port;
}server_t;

server_t* servers;
int nb_distict_servers = 0;

///////////////////////////////////////////////////////
int more_output_for_stderr = 0;

void parse_config_file(char * conf_file) {

   char buffer[2048];

   FILE * stream;
   stream = fopen(conf_file, "r");
   if (stream == NULL) {
      printf("file %s doesn't exist or bad permissions.\n", conf_file);
      exit(EXIT_FAILURE);
   }

   int line_num = 0;

   while (fgets(buffer, 2048, stream) != NULL) {

      line_num++;

      //jump useless lines
      if (strchr(buffer, '#')==buffer) {
         continue;
      }
      else if (strcmp(buffer, "\n")==0) {
         continue;
      }

      const char delimiters[] = " =\n";
      char *cp;
      char * value;
      char * param;

      cp = strdup(buffer);
      param = strtok(cp, delimiters);
      value = strtok(NULL, delimiters);

      if (value == NULL) {
         printf("no value for parameter %s line %d\n", param, line_num);
         exit(EXIT_FAILURE);
      }

      if (strcmp(param, "nb_iterations_in_a_wave") == 0) {
         nb_iterations_in_a_wave = atoi(value);
      }
      else if (strcmp(param, "targets") ==0) {
         targets = value;
      }
      else if (strcmp(param, "nb_clientsMin") ==0) {
         nb_clients_min = atoi(value);
      }
      else if (strcmp(param, "nb_clientsMax") ==0) {
         nb_clients_max = atoi(value);
      }
      else if (strcmp(param, "step") ==0) {
         step = atoi(value);
      }
      else if (strcmp(param, "duration") ==0) {
         duration = atoi(value);
      }
      else if (strcmp(param, "nb_msg_per_connection") ==0) {
         nb_msg_per_connection = atoi(value);
      }
      else if (strcmp(param, "delay") ==0) {
         delay = atoi(value);
      }
      else if (strcmp(param, "percent_stats_kept_master") ==0) {
         percent_stats_kept_master = atoi(value);
      }
      else if (strcmp(param, "slaves") ==0) {
         slavesInfos = value;
      }
      else if (strcmp(param, "N-copy") ==0) {
         N_copy = atoi(value);
      }
      else if (strcmp(param, "warmup") == 0) {
         warmup_length = atoi(value);
      }
      else {
         PANIC("unknown config parameter line %d: %s\n", line_num, param);
      }
   }

   fclose(stream);
}

void print_help_message() {

   printf("-------------------------SLG_MASTER HELP MESSAGE-------------------------\n");
   printf("parameters:\n");
   printf("\t--param (-p): description\n");
   printf("\t--targets (-t): list of target for each slave. (ip:port,ip:port)\n");
   printf("\t--slaves (-S): list of slaves. (slave:port,slave:port)\n");
   printf("\t--nb_iterations (-i): the number of iteration to do in a wave\n");
   printf("\t--nb_clients_min (-m): each slave starts with this number of clients\n");
   printf("\t--nb_clients_max (-M): each slave ends with this number of clients\n");
   printf("\t--step (-s): the steps from nb_clients_min to nb_clients_max\n");
   printf("\t--duration (-D): duration of one iteration (in seconds) \n");
   printf("\t--nb_msg_per_connection (-n): number of request per connection\n");
   printf("\t--delay* (-d): not yet implemented\n");
   printf("\t--percent_stats_kept_master* (-p): the percentage number of samples kept on the master\n");
   printf("\t--file* (-f): config file\n");
   printf("\t-r: dump a summary on stderr to know where is the bench when redirecting in a file\n");

   printf("Parameters with a * are optionnal, all  others are mandatory.\n");
   printf("The parameters from the command line overide the ones in the config file .\n");

   printf("-------------------------------------------------------------------------\n");
   exit(EXIT_SUCCESS);
}

//Check everything is initialized
void check_all_parameters_filled() {
   if (targets == NULL) {
      PANIC("targets not initialized.\n");
   }

   if (nb_iterations_in_a_wave == 0) {
      PANIC("nb_iterations_in_a_wave not initialized.\n");
   }

   if (duration == 0) {
      PANIC("nb_connections not initialized.\n");
   }

   if (nb_msg_per_connection == 0) {
      PANIC("nb_msg_per_connection not initialized.\n");
   }

   if (percent_stats_kept_master<=0 || percent_stats_kept_master>100) {
      PANIC("percent_stats_kept_slaves should be in [1,100].\n");
   }

   if (nb_clients_min == 0) {
      PANIC("nb_clients_min not initialized.\n");
   }

   if (nb_clients_max == 0) {
      PANIC("nb_clients_max not initialized.\n");
   }

   if (slavesInfos == NULL) {
      PANIC("slavesInfos not initialized.\n");
   }

   if(N_copy != 0 && N_copy != 1){
      PANIC("Unknown value for N_Copy");
   }

   if(warmup_length < 0){
      PANIC("Unknown value for warmup");
   }

   return;
}

void parse_command_line(int argc, char** argv) {

   //Command line has priority on config file, so let's do the file first
   int i = 0;
   while (i<argc) {
      if ( (strcmp(argv[i], "-f")==0) || (strcmp(argv[i], "--file")==0)) {
         if (i+1>=argc) {
            printf("missing file name\n");
            exit(EXIT_FAILURE);
         }

         config_file = argv[i+1];
         parse_config_file(config_file);
         break;
      }
      i++;
   }

   int c;

   while (1) {
      int option_index = 0;
      static struct option long_options[] =
      {
               { "nb_iterations", 1, 0, 'i' },
               { "targets", 1, 0, 't' },
               { "nb_clients_min", 1, 0, 'm' },
               { "nb_clients_max", 1, 0, 'M' },
               { "step", 1, 0, 's' },
               { "duration", 1, 0, 'D' },
               { "nb_msg_per_connection", 1, 0, 'n' },
               { "delay", 1, 0,'d' },
               { "slaves", 1, 0, 'S' },
               { "percent_stats_kept_master", 1, 0, 'P' },
               { "help", 0, 0, 'h' },
               { "file", 1, 0, 'f' },
               { 0, 0, 0, 0 }
      };

      c = getopt_long(argc, argv, "i:t:m:M:s:D:n:d:S:f:P:rh",
               long_options, &option_index);

      if (c == -1)
         break;

      switch (c) {

         case 'i':
            //printf("option i with value '%s'\n", optarg);
            nb_iterations_in_a_wave = atoi(optarg);
            break;

         case 't':
            targets = optarg;
            break;

         case 'm':
            nb_clients_min = atoi(optarg);
            break;

         case 'M':
            nb_clients_max = atoi(optarg);
            break;

         case 's':
            step = atoi(optarg);
            break;

         case 'D':
            duration = atoi(optarg);
            break;

         case 'n':
            nb_msg_per_connection = atoi(optarg);
            break;

         case 'd':
            delay = atoi(optarg);
            break;

         case 'f':
            break;

         case 'r':
            more_output_for_stderr = 1;
            break;

         case 'h':
            print_help_message();
            break;

         case 'P':
            percent_stats_kept_master = atoi(optarg);
            break;

         case 'S':
            slavesInfos = optarg;
            break;

         default:
            exit(EXIT_FAILURE);
      }
   }
   if (optind < argc) {
      printf("invalid argument: ");
      while (optind < argc)
         printf("%s ", argv[optind++]);
      printf("\n");
      exit(EXIT_FAILURE);
   }

   check_all_parameters_filled();
}

/**
 * Initialise the stat tables
 */
void init_master_stat_report(int nb_waves, int nb_iterations_in_wave) {
   int w;

   for (w=0; w<nb_waves; w++) {
      init_stats_sample_Lf(&master_stats[w].totalTime, nb_iterations_in_wave, "totalTime");
      init_stats_sample_Lf(&master_stats[w].avgCT, nb_iterations_in_wave, "avgCT");
      init_stats_sample_Lf(&master_stats[w].avgRT, nb_iterations_in_wave, "avgRT");
      init_stats_sample_Lf(&master_stats[w].avgCRT, nb_iterations_in_wave, "avgCRT");
      init_stats_sample_Lf(&master_stats[w].resp_rate, nb_iterations_in_wave, "resp_rate");
      init_stats_sample_Lf(&master_stats[w].bytes_rate, nb_iterations_in_wave, "bytes_rate");
      init_stats_sample_Lf(&master_stats[w].maxCT, nb_iterations_in_wave, "maxCT");
      init_stats_sample_Lf(&master_stats[w].minCT, nb_iterations_in_wave, "minCT");
      init_stats_sample_Lf(&master_stats[w].errors, nb_iterations_in_wave, "errors");
   }

}

/**
 * Fill the slave reported values in the corresponding case of the stats tables
 */
void fill_master_stat_report(
         int nbSlaves,
         __attribute__((unused)) slaves_stats_t * slaves,
         int wave_number,
         unsigned long duration)
{
   int i;
   int w = wave_number;

   long double totalTime = 0;
   long double avgCT = 0;
   long double avgRT = 0;
   long double avgCRT = 0;
   long double bytes_rate = 0;
   long double resp_rate = 0;
   long double maxCT = 0;
   long double minCT = 0;
   long double errors = 0;

   for (i=0; i<nbSlaves; i++) {
      totalTime += slaves[i].totalTime;
      avgCT += slaves[i].avgCT;
      avgRT += slaves[i].avgRT;
      avgCRT += slaves[i].avgCRT;
      bytes_rate += slaves[i].bytes_recv;
      resp_rate += slaves[i].resp_recv;
      errors += slaves[i].errors;
   }

   insert_Lf(&master_stats[w].totalTime, totalTime/nbSlaves);
   insert_Lf(&master_stats[w].avgCT, avgCT/nbSlaves);
   insert_Lf(&master_stats[w].avgRT, avgRT/nbSlaves);
   insert_Lf(&master_stats[w].avgCRT, avgCRT/nbSlaves);

   // *1000000 because we want request per second not per usec
   insert_Lf(&master_stats[w].bytes_rate, (bytes_rate/duration)*1000000);
   insert_Lf(&master_stats[w].resp_rate, (resp_rate/duration)*1000000);

   insert_Lf(&master_stats[w].maxCT, maxCT/nbSlaves);
   insert_Lf(&master_stats[w].minCT, minCT/nbSlaves);

   insert_Lf(&master_stats[w].errors, errors);
}

/**
 * Prints the stats for one wave.
 */
void print_stats(int nb_client_min, int step, int nbSlaves, int wave_num) {
#if DEBUG_TASK
   fprintf(stderr,"----------------------------------------------------------------\n");
   fprintf(stderr,"All presented times are in microseconds(us), excepted those mentionned.\n");
   fprintf(stderr,"All values are average values.\n");
   fprintf(stderr,"----------------------------------------------------------------\n");

   fprintf(stderr,"----------------------------------------------------------------\n");
   fprintf(stderr,"----------------------MASTER RESULTS----------------------------\n");
   fprintf(stderr,"----------------------------------------------------------------\n");
#endif

   int w = wave_num;

   if (more_output_for_stderr) {
      fprintf(stderr, "[ %s ] %5d clients done, throughput: %Lf, stddev: %Lf\n",
               getCurrentTime(),(nb_client_min+(w*step))*nbSlaves,
               average_Lf(&master_stats[w].resp_rate),
               stddev_Lf(&master_stats[w].resp_rate));
   }


   long double resp_rate_avg = percentil_average_Lf(&master_stats[w].resp_rate, percent_stats_kept_master);
   long double resp_rate_stddev = percentil_stddev_Lf(&master_stats[w].resp_rate, percent_stats_kept_master);
   long double bytes_rate_avg = percentil_average_Lf(&master_stats[w].bytes_rate, percent_stats_kept_master);
   long double bytes_rate_stddev = percentil_stddev_Lf(&master_stats[w].bytes_rate, percent_stats_kept_master);
   long double ct_avg = percentil_average_Lf(&master_stats[w].avgCT, percent_stats_kept_master);
   long double ct_stddev = percentil_stddev_Lf(&master_stats[w].avgCT, percent_stats_kept_master);
   long double rt_avg = percentil_average_Lf(&master_stats[w].avgRT, percent_stats_kept_master);
   long double rt_stddev = percentil_stddev_Lf(&master_stats[w].avgRT, percent_stats_kept_master);
   long double crt_avg = percentil_average_Lf(&master_stats[w].avgCRT, percent_stats_kept_master);
   long double crt_stddev = percentil_stddev_Lf(&master_stats[w].avgCRT, percent_stats_kept_master);
   long double tt_avg = percentil_average_Lf(&master_stats[w].totalTime, percent_stats_kept_master);
   long double tt_stddev = percentil_stddev_Lf(&master_stats[w].totalTime, percent_stats_kept_master);
   long double errors_avg = percentil_average_Lf(&master_stats[w].errors, percent_stats_kept_master);
   long double errors_stddev = percentil_stddev_Lf(&master_stats[w].errors, percent_stats_kept_master);

   //nb_clients*nbSlaves, master_stats.req_throughput, master_stats.bytes_throughput, master_stats.avgCT, master_stats.avgRT, master_stats.avgCRT, master_stats.totalTime
   printf("%5d"
            "\t%15.02Lf %3.02Lf%%"
            "\t%15.02Lf %3.02Lf%%"
            "\t%15.02Lf %3.02Lf%%"
            "\t%15.02Lf %3.02Lf%%"
            "\t%15.02Lf %3.02Lf%%"
            "\t%15.02Lf %3.02Lf%%"
            "\t%15.02Lf %3.02Lf%%\n",
            (nb_client_min+(w*step))*nbSlaves,
            resp_rate_avg,
            resp_rate_stddev * 100. / resp_rate_avg,
            bytes_rate_avg / (1024.*1024.) * 8.,
            bytes_rate_stddev / bytes_rate_avg * 100.,
            ct_avg,
            ct_stddev * 100. / ct_avg,
            rt_avg,
            rt_stddev * 100. / rt_avg,
            crt_avg,
            crt_stddev * 100. / crt_avg,
            tt_avg,
            tt_stddev * 100. / tt_avg,
            errors_avg,
            errors_stddev * 100. / errors_avg );

#if DEBUG_TASK
   for (w = 0; w < NB_PRINT_SEPARATORS; w++) {
      fprintf(stderr,"-");
   }
#endif

   fprintf(stderr,"\n");
}

/**
 * Init slaves_infos tab and generate for each slave, its addr, port, target and target_port
 * slavesInfos: the string of slaves
 * targets: the string of targets
 * returns the number of slaves
 *
 */
int init_slaves_info(char *slavesInfos, char * targets) {

   const char delimiters[] = ",";
   char *token, *cp;
   int nb_slaves = 1;
   int nb_targets = 1;
   char * temp;
   int i;

   temp = slavesInfos;
   //calculate nb_slaves:
   while (1) {
      temp = strchr(temp, delimiters[0]);
      if (temp == NULL)
         break;
      temp += sizeof(char);
      nb_slaves++;
   }

   temp = targets;
   //calculate nb_targets:
   while (1) {
      temp = strchr(temp, delimiters[0]);
      if (temp == NULL)
         break;
      temp += sizeof(char);
      nb_targets++;
   }

   /** Servers structure **/
   servers = calloc(nb_targets,sizeof(server_t));

   if (nb_targets != nb_slaves) {
      printf("Not the same number of targets and slaves\n");
      exit(0);

   }

   slaves_infos = calloc(nb_slaves, sizeof(slaves_infos_t));
   assert(slaves_infos!=NULL);

   //Extract slaves info.
   cp = strdup(slavesInfos);
   token = strtok(cp, delimiters);

   if (token == NULL) {
      printf("Bad slaves adresses format\n");
      exit(0);
   }

   for (i = 0; i<nb_slaves; i++) {
      char * port = strchr(token, ':');
      if (port == NULL) {
         printf("Bad slaves addresse format\n");
         exit(0);
      }
      *port = 0;
      port += sizeof(char);

      //Get hostname
      strcpy(slaves_infos[i].addr, token);

      //Get port
      slaves_infos[i].port = atoi(port);

      //init socket
      slaves_infos[i].socket = -1;

      token = strtok(NULL, delimiters);
      if (token == NULL)
         break;
   }

   //Extract targets info.
   cp = strdup(targets);
   token = strtok(cp, delimiters);

   if (token == NULL) {
      printf("Bad slaves adresses format\n");
      exit(0);
   }
   for (i = 0; i<nb_targets; i++) {
      char * port = strchr(token, ':');
      if (port == NULL) {
         printf("Bad targets address format\n");
         exit(0);
      }
      *port = 0;
      port += sizeof(char);

      //Put target host
      strcpy(slaves_infos[i].target, token);

      //Get target port
      slaves_infos[i].target_port = atoi(port);

      /** Search if server already found **/
      int j = 0;
      for(j = 0; j < nb_distict_servers; j++){
         if(strcmp(slaves_infos[i].target, servers[j].server) == 0
                  && slaves_infos[i].target_port == servers[j].port){
            /** Already inserted **/
            break;
         }
      }

      /** If not found **/
      if(j == nb_distict_servers){
         servers[j].server = slaves_infos[i].target;
         servers[j].port = slaves_infos[i].target_port;
         DEBUG_TMP("Found a new server : %s,%d\n", servers[j].server, servers[j].port );

         nb_distict_servers++;
      }

      token = strtok(NULL, delimiters);
      if (token == NULL)
         break;
   }

   /** If not N-Copy only one server is targeted
    * with possibly multiple itf
    */
   if(!N_copy){
      nb_distict_servers = 1;
   }

   return nb_slaves;
}

/**
 * Connects the master to the slaves
 */
void connect_to_slaves(slaves_infos_t * slaves, int nb_slaves) {
   int i = 0;

   while (i<nb_slaves) {
      int fd;
      struct sockaddr_in fsin;
      struct sockaddr_in soc_address;
      struct sockaddr_in *sin_ptr = &soc_address;

      //init server target
      struct in_addr *ip_addr;
      struct hostent *ent= NULL;

      //Init remote server struct
      if (slaves[i].addr) {
         ent = gethostbyname(slaves[i].addr);
         if (ent == NULL) {
            fprintf(stderr, "lookup on slave's name \"%s\" failed\n", slaves[i].addr);
            exit(-1);
         }
         ip_addr = (struct in_addr *)(*(ent->h_addr_list));
         sin_ptr->sin_family = AF_INET;
         bcopy(ip_addr, &(sin_ptr->sin_addr), sizeof(struct in_addr));
      }

      if (!ent) {
         if (slaves[i].addr)
            fprintf(stderr,"error - didn't get host info for %s\n", slaves[i].addr);
         else
            fprintf(stderr,"error - never called gethostbyname\n");
         exit(-1);
      }

      sin_ptr->sin_port = htons(slaves[i].port);

      //Init client side
      //Getting Socket
      fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (fd == -1) {
         perror("socket");
         //finish();
         exit(0);
      }

      int reuse_addr = 1;
      //Enables local address reuse
      if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr))
               < 0) {
         perror("setsockopt");
      }

      fsin.sin_family = AF_INET;
      fsin.sin_addr.s_addr = htonl(INADDR_ANY);
      fsin.sin_port = htons(0);

      if (bind(fd, (struct sockaddr*)&fsin, sizeof(fsin)) < 0) {
         fprintf(stderr, "(%s) bind error on port %d\n",__FUNCTION__, 0);
         exit(-1);
      }

      int status;
      status
               = connect(fd, (struct sockaddr *)&soc_address, sizeof(soc_address));

      if (status < 0) {
         printf("Error on %s\n", slaves[i].addr);
         perror("connect");
         exit(-1);
      }

      slaves[i].socket = fd;

      i++;
   }

}

/**
 * Sends order to slaves
 * Warning: msg MUST be a string with a null termination character.
 */
void send_order_to_slaves(slaves_infos_t * slaves, int nb_slaves, int warmup) {
   int i = 0;
   int cnt = 0;

   char msg[1024];
   char name[256];

   while (i<nb_slaves) {
      bzero(msg, sizeof(msg));
      bzero(name, sizeof(name));

      gethostname(name, 255);

      //creating the query string to send to the slaves.
      //params: slave_number, reportingAddress, host, port, nb_clients, nb_iterations, nb_msg_per_connection, delay, slaves_interval_avg_percent
      int _duration = duration;
      if(warmup){
         _duration = warmup_length;
      }

      sprintf(msg, "%d,%s,%s,%d,%d,%d,%d,%d\n",
               i, name, slaves[i].target, slaves[i].target_port,
               nb_clients, _duration, nb_msg_per_connection,
               delay);

      cnt = 0;
      do {
         cnt += write(slaves[i].socket, msg, strlen(msg));

         if (cnt == -1) {
            perror("Send failed");
            exit(0);
         }
      } while ((unsigned int)cnt<strlen(msg));

      if (cnt < 0) {
         perror("sendto");
         exit(1);
      }

      i++;
   }

}

/**
 * Read responses from the slaves
 */
void read_slaves_response(slaves_infos_t * slaves_infos, slaves_stats_t * slaves, int nb_slaves, int warmup) {

   int i;

   char* buf = malloc(RESPONSE_SIZE * sizeof(char));

   if (buf == NULL) {
      perror("malloc");
      exit(0);
   }

   for (i = 0; i < nb_slaves; i++) {
      //Read request
      bzero(buf, RESPONSE_SIZE * sizeof(char));

      //Reading message until \n
      int rd = 0;
      int totalLength = 0;
      do {
         rd = read(slaves_infos[i].socket, buf + totalLength, RESPONSE_SIZE - totalLength);

         if (rd == 0) {
            printf("Connection closed on socket %d (client %s) and slave response not full.\n", slaves_infos[i].socket, slaves_infos[i].addr);
            printf("Already received %d bytes : %s\n", totalLength, buf);
            exit(-1);
         }
         else if (rd == -1) {
            printf("Error while reading on socket %d\n", slaves_infos[i].socket);
            exit(-1);
         }

         totalLength += rd;

         if (buf[totalLength - 1] == '\n') {
            break;
         }
      } while (1);

      DEBUG("Received data: %s\n", buf);

      if (!warmup) {
         //Filling slaves_stats with the incoming data :
         struct sockaddr_in csin;
         unsigned int size = sizeof(csin);
         getpeername(slaves_infos[i].socket, (struct sockaddr*) &csin, &size);

         //setting slave id
         sprintf(slaves[i].id, "%s:%d", inet_ntoa(csin.sin_addr), htons(csin.sin_port));

         //Getting results
         // total time, global_bytes_throughput, global_req_throughput, global_avgCT, global_avgRT, global_avgCRT, global_minCT, global_maxCT, global_errors
         const char delimiters[] = ",";
         char *token, *cp;

         cp = strdup(buf);
         token = strtok(cp, delimiters);
         slaves[i].totalTime = atol(token);

         token = strtok(NULL, delimiters);
         slaves[i].bytes_recv = atoll(token);

         token = strtok(NULL, delimiters);
         slaves[i].resp_recv = atol(token);

         token = strtok(NULL, delimiters);
         slaves[i].avgCT = atof(token);
         token = strtok(NULL, delimiters);
         slaves[i].avgRT = atof(token);
         token = strtok(NULL, delimiters);
         slaves[i].avgCRT = atof(token);
         token = strtok(NULL, delimiters);
         slaves[i].minCT = atol(token);
         token = strtok(NULL, delimiters);
         slaves[i].maxCT = atol(token);

         token = strtok(NULL, delimiters);
         slaves[i].errors = atol(token);
      }

      close(slaves_infos[i].socket);

      DEBUG("\tMaster received responses from %s: %s\n", slaves[i].id, getCurrentTime());
   }

   free( buf);

}

//sum up the parameters
void print_parameter_summary(FILE* output) {
   unsigned int i;

   fprintf(output,"\n********** System Info **********\n");
   do_proc_info(output, "/proc/sys/net/ipv4/tcp_tw_recycle");
   do_proc_info(output, "/proc/sys/net/ipv4/tcp_fin_timeout");
   do_proc_info(output, "/proc/sys/net/ipv4/ip_local_port_range");
   fprintf(output,"*********************************\n\n");

   char * config_file= NULL;

   fprintf(output, "Parameters:\n");

   if (config_file != NULL) {
      fprintf(output, "\t- config_file: %s\n", config_file);
   }

   fprintf(output, "\t- targets: ");
   for (i = 0; i<nb_slaves; i++) {
      fprintf(output, "%s:%d", slaves_infos[i].target, slaves_infos[i].target_port);
      if (i!=nb_slaves-1) {
         fprintf(output, ",");
      }
   }
   fprintf(output, "\n");
   fprintf(output, "\t- slaves list: ");
   for (i = 0; i<nb_slaves; i++) {
      fprintf(output, "%s:%d", slaves_infos[i].addr, slaves_infos[i].port);
      if (i!=nb_slaves-1) {
         fprintf(output, ",");
      }
   }
   fprintf(output, "\n");
   fprintf(output,
            "\t- nb slaves: %d\n"
            "\t- nb iterations in a wave: %d\n"
            "\t- nb waves: %d\n"
            "\t- nb clients min:%d\n"
            "\t- nb clients max:%d\n"
            "\t- nb clients step:%d\n"
            "\t- iteration duration:%d s\n"
            "\t- nb msg per connection:%d\n"
            "\t- master percent: %d%%\n"
            "\t- sleep time between two iterations: %d (us)\n"
            "\t- warmup: %d s\n",
            nb_slaves, nb_iterations_in_a_wave, nb_waves, nb_clients_min,
            nb_clients_max, step, duration,nb_msg_per_connection,
            percent_stats_kept_master, SLEEP_TIME_BETWEEN_TWO_ITERATIONS, warmup_length);

#ifdef USE_RST
   fprintf(output,"\t- Using RST flag for closing connection\n");
#endif

#if CLOSE_AFTER_REQUEST
   fprintf(output, "\t--- WARNING: Close after request ... ---\n");
#endif

#ifdef HTTP_PROTOCOL
   fprintf(output,"\t- Using HTTP protocol\n");
#elif MEMCACHED_PROTOCOL
   fprintf(stderr,"\t- Using MEMCACHED protocol\n");
#endif


#if UNIQUE_FILE_ACCESS_PATTERN
   fprintf(output, "\t- Accessing to a unique file\n");
#elif SPECWEB99_FILE_ACCESS_PATTERN
   fprintf(output, "\t- Using SpecWeb99 distribution\n");
#elif SPECWEB05_FILE_ACCESS_PATTERN
   fprintf(output, "\t- Using SpecWeb05 distribution\n");
#else
   PRINT_ALERT("Unknown access file protocol\n");
   exit(EXIT_FAILURE);
#endif

   fprintf(output, "\t- N-Copy server : %s \n", N_copy ? "true" : "false");

   //if(output != stderr){
      fprintf(output,"\n%5s\t%10s %10s\t%14s %10s\t%10s %10s\t%10s %10s\t%10s %10s\t%14s %10s\t%10s %10s\n",
               "nbC",
               "req/s","sdev",
               "Mbits/s","sdev",
               "CT (µs)","sdev",
               "RT (µs)","sdev",
               "CRT (µs)","sdev",
               "tavgTotalTime (µs)","stddev",
               "errors","stddev");

      for (i = 0; i < NB_PRINT_SEPARATORS; i++) {
         fprintf(output,"-");
      }
      fprintf(output,"\n");
   //}
}


void warmup_time(){
   if(warmup_length <= 0){
      return;
   }
   connect_to_slaves(slaves_infos, nb_slaves);
   send_order_to_slaves(slaves_infos, nb_slaves, 1);

   fprintf(stderr,">> Warmup %s\n",getCurrentTime());

   //wait connections from slaves to read reports.
   read_slaves_response(slaves_infos, NULL, nb_slaves, 1);

   //STOP time
   fprintf(stderr,"<< Warmup %s\n",getCurrentTime());

   usleep(SLEEP_TIME_BETWEEN_TWO_ITERATIONS);

}

/**
 *
 * For each step, slg_master sends a wave onto the webserver:
 *   1- A wave is composed of nb_iterations_in_a_wave basic loads.
 *   2- A basic load is a single load generation order sent to all slg_slaves.
 *
 * So, there is one wave per number of cl   DEBUG_TMP("Send special request to server ")ients to bench. (Each step creates a new wave).
 *
 */
int main(int argc, char** argv) {

   unsigned int i;

   if (argc == 1) {
      print_help_message();
   }

   parse_command_line(argc, argv);

#if CLOSE_AFTER_REQUEST
   if(nb_msg_per_connection > 1){
      PRINT_ALERT("Multiple messages per connection are not allowed with CLOSE_AFTER_REQUEST\n");
      exit(EXIT_FAILURE);
   }
#endif

   nb_slaves = init_slaves_info(slavesInfos, targets);

   //nb_waves computation
   nb_waves = ( (nb_clients_max - nb_clients_min) / step ) + 1;

   //alloc a struct to store slaves results
   slaves_stats = malloc(nb_slaves * sizeof(slaves_stats_t));
   if (slaves_stats==NULL) {
      perror("malloc slaves_stats");
      exit(0);
   }
   bzero(slaves_stats, nb_slaves * sizeof(slaves_stats_t));

   //Alloc master_stats
   master_stats = calloc(nb_waves, sizeof(master_stats_t));
   if (master_stats==NULL) {
      perror("malloc master_stats");
      exit(0);
   }

   init_master_stat_report(nb_waves, nb_iterations_in_a_wave);

   fprintf(stderr,"[ %s ] START\n",getCurrentTime());

   print_parameter_summary(stderr);

   int current_wave_number = 0;

   nb_clients = nb_clients_min;

   int wave_length = warmup_length + nb_iterations_in_a_wave * duration;

   while (nb_clients <= nb_clients_max) {
      fprintf(stderr, "\n##########################\n");
      int remains_sec = (wave_length * (nb_waves - current_wave_number ));

      fprintf(stderr, "%d clients [test %d / %d, %d min %d sec]\n",
               nb_clients*nb_slaves,
               (current_wave_number + 1),
               nb_waves,
               remains_sec / 60,
               remains_sec - (remains_sec / 60)*60
               );
      fprintf(stderr, "##########################\n");

      warmup_time();

      for (i = 0; i<nb_iterations_in_a_wave; i++) {
         fprintf(stderr,">> [Iteration %d / %d]  %s\n", (i+1) , nb_iterations_in_a_wave, getCurrentTime());

         DEBUG("Master connect and send to slave : %s\n",getCurrentTime());
         //connect and send order to slaves
         connect_to_slaves(slaves_infos, nb_slaves);
         send_order_to_slaves(slaves_infos, nb_slaves, 0);

         //START time
         assert(gettimeofday(&start_time,NULL) == 0);

         //wait connections from slaves to read reports.
         read_slaves_response(slaves_infos, slaves_stats, nb_slaves, 0);

         DEBUG("Master received all responses : %s\n",getCurrentTime());

         //STOP time
         assert(gettimeofday(&stop_time,NULL) == 0);

         //Accumulate threads reports.
         fill_master_stat_report(nb_slaves, slaves_stats, current_wave_number, compare_time(&start_time, &stop_time));

         //Reset slave reports.
         bzero(slaves_stats, nb_slaves * sizeof(slaves_stats_t));

         //wait
         fprintf(stderr,"<< [Iteration %d / %d] %s\n",(i+1),nb_iterations_in_a_wave,getCurrentTime());

         usleep(SLEEP_TIME_BETWEEN_TWO_ITERATIONS);
      }

      //compute_reports(nb_iterations_in_a_wave, current_wave_number, current_wave_number);
      print_stats(nb_clients_min, step, nb_slaves, current_wave_number);

      current_wave_number++;
      //update nbClients
      nb_clients += step;
   }

   fprintf(stderr,"[ %s ] STOP\n",getCurrentTime());

   if (more_output_for_stderr) {
      print_parameter_summary(stdout);
   }

   free(slaves_infos);
   free(slaves_stats);

   return 0;
}
