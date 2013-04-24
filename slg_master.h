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

#ifndef SLG_MASTER_H_
#define SLG_MASTER_H_

#define NB_PRINT_SEPARATORS 180

#include "stats_utils.h"

#ifdef USE_RST
//#define SLEEP_TIME_BETWEEN_TWO_ITERATIONS 500
#define SLEEP_TIME_BETWEEN_TWO_ITERATIONS 5000000 //5s
#else
#define SLEEP_TIME_BETWEEN_TWO_ITERATIONS 1000000
#endif

/** Structs used by the master **/
typedef struct slaves_stats_t{

	int fd;
	
	char id[128];
	unsigned long totalTime;
	double avgCT;
	double avgRT;
	double avgCRT;
	
	unsigned long resp_recv;
	unsigned long long bytes_recv;
	
	unsigned long maxCT;
	unsigned long minCT;
	
	unsigned long errors;
	
} slaves_stats_t;

typedef struct slaves_infos_t{

	char addr[128];
	int port;
	char target[128];
	int target_port;
	int socket;
	
} slaves_infos_t;

typedef struct master_stats_t{

	stats_sample_Lf_t totalTime;
	stats_sample_Lf_t avgCT;
	stats_sample_Lf_t avgRT;
	stats_sample_Lf_t avgCRT;
	
	stats_sample_Lf_t resp_rate;
	stats_sample_Lf_t bytes_rate;
	
	stats_sample_Lf_t maxCT;
	stats_sample_Lf_t minCT;	
	
	stats_sample_Lf_t errors;
	
} master_stats_t;
#endif /*SLG_MASTER_H_*/
