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

#ifndef __DEBUG_TOOLS_H__
#define __DEBUG_TOOLS_H__

/**
 * These macros could be used to monitor a function execution time
 * They must be used in the same function
 */
#define START struct timeval start; gettimeofday(&start, NULL)
#define STOP  struct timeval stop; gettimeofday(&stop, NULL); printf("%lud\t %s:%d us\n", compare_time(&start, &stop), __FUNCTION__,__LINE__)

/**
 * This macro permit to have != debug levels
 * -1 : no traces
 * 1 : per-session requests dump
 */

#if DEBUG_TASK
#define DEBUG(msg, args...) \
   fprintf(stderr,"(%s,%d) ",__FUNCTION__,__LINE__); \
   fprintf(stderr,msg, ##args)
#else
#define DEBUG(msg, args...)
#endif

#if DEBUG_LEVEL==1
#define DUMP_REQUEST(client) do {													\
	if (client->number < MAX_SESSION_DUMP){											\
	  unsigned long long int t;																	\
	  rdtscll(t);																	\
	  fprintf(stderr,"[%llu] %d '%s'\n", t, client->number, client->request_buf);	\
	}																				\
	} while(0)
#else
#define DUMP_REQUEST(msg, args...)
#endif //DEBUG_LEVEL==1

#define DEBUG_TMP(msg, args...) \
   fprintf(stderr,"(%s,%d) " msg, __FUNCTION__, __LINE__, ##args)


#define PRINT_ALERT(msg, args...) \
   fprintf(stderr,"-------\t(%s,%d) " msg, __FUNCTION__, __LINE__, ##args); \
   fflush(stderr);


#define PANIC(msg, args...) {\
   fprintf(stderr,"*******\t(%s,%d) " msg, __FUNCTION__, __LINE__, ##args); \
   exit(EXIT_FAILURE); \
}



/**
 *  compare_time returns a long int representing the difference between 2 time
 */
unsigned long compare_time(struct timeval* start_time, struct timeval* stop_time);

/**
 * getCurrentTime returns a string representing the current time in a human readable way
 */
char* getCurrentTime();

void do_proc_info(FILE* output, char *proc_file);


#endif /*__DEBUG_TOOLS_H__*/
