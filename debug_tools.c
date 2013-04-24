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

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "debug_tools.h"

unsigned long compare_time(struct timeval* start_time, struct timeval *stop_time){
	unsigned long sec_res = stop_time->tv_sec - start_time->tv_sec;
	unsigned long usec_res = stop_time->tv_usec - start_time->tv_usec;

	return 1000000*sec_res + usec_res;

}

char* getCurrentTime(){
	time_t timeseconds;
	timeseconds = time(NULL);
	char *timestring = ctime(&timeseconds);
	timestring[strlen(timestring)-1] = '\0';

	return timestring;
}

void do_proc_info(FILE* output, char *proc_file)
{
	char proc_info[256];

	FILE *fp;
	if ((fp = fopen(proc_file,"r")) == NULL){
		DEBUG_TMP("Error while opening %s\n",proc_file);
		exit(EXIT_FAILURE);
	}

	// Per proc_file rules
	if(strncmp(proc_file,"/proc/sys/net/ipv4/ip_local_port_range",strlen(proc_file)) == 0 ){
		char proc_info2[256];
		if(fscanf (fp,"%s\t%s",proc_info,proc_info2)){};

#ifndef USE_RST
		fprintf(output,"* %-50s\t\e[01;31m%s %s\e[m\n", proc_file, proc_info, proc_info2);
#else
		fprintf(output,"* %-50s\t%s %s\n", proc_file, proc_info, proc_info2);
#endif
	}
	else if(strncmp(proc_file,"/proc/sys/net/ipv4/tcp_fin_timeout",strlen(proc_file)) == 0 ){
		if(fscanf (fp,"%s",proc_info)){};

#ifndef USE_RST
		fprintf(output,"* %-50s\t\e[01;31m%s\e[m\n", proc_file, proc_info);
#else
		fprintf(output,"* %-50s\t%s\n", proc_file, proc_info);
#endif
	}
	else if(strncmp(proc_file,"/proc/sys/net/ipv4/tcp_tw_recycle",strlen(proc_file)) == 0 ){
		if(fscanf (fp,"%s",proc_info)){};

#ifndef USE_RST
		fprintf(output,"* %-50s\t\e[01;31m%s\e[m\n", proc_file, proc_info);
#else
		fprintf(output,"* %-50s\t%s\n", proc_file, proc_info);
#endif
	}
	else{
		if(fscanf (fp,"%s",proc_info)){};
		fprintf(output,"* %-50s\t%s\n", proc_file, proc_info);
	}

	fclose(fp);
}
