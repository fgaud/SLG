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

#ifndef __STATS_UTILS_H__
#define __STATS_UTILS_H__

#include <math.h> //for sqrt and pow

/**********************************************************************
 * 
 * This file contains tools to easilly handle statitic computation
 * There are two kinds of functions:
 * 		-The ones that maipulates a struct which contains samples.
 * 		-Basic functions which compte on a table of values
 * 
 **********************************************************************/

/***********************stats_sample_Lf_t functions****************************/
/**
 * This structure is used by the programmer to store values
 * It must be initialized with init_stats_sample_Lf.
 */
typedef struct{
	long double * s;
	unsigned long nb_elem;
	unsigned long size_max;
	char * name;
}stats_sample_Lf_t;

void init_stats_sample_Lf(stats_sample_Lf_t * sample, int size_max, char * name);
void destroy_stats_sample_Lf(stats_sample_Lf_t * sample);

void insert_Lf(stats_sample_Lf_t *sample, long double val);

void  dump_Lf(stats_sample_Lf_t *sample);

long double average_Lf(stats_sample_Lf_t *sample);
long double average_multiple_Lf(stats_sample_Lf_t **sample, unsigned int nb_tabs);
long double stddev_Lf(stats_sample_Lf_t *sample);

long double interval_average_Lf(stats_sample_Lf_t *sample, int percent);
long double interval_stddev_Lf(stats_sample_Lf_t *sample, int percent);

long double percentil_average_Lf(stats_sample_Lf_t *sample, int percentil);
long double percentil_stddev_Lf(stats_sample_Lf_t *sample, int percentil);

long double min_Lf(stats_sample_Lf_t *sample);
long double max_Lf(stats_sample_Lf_t *sample);

/***********************Table functions****************************/
double t_average_f(double *table,int n);
double t_average_ul(unsigned long *table,int n);
unsigned long t_minSearch_ul(unsigned long *table,int n);
unsigned long t_maxSearch_ul(unsigned long *table,int n);

#endif //__STATS_UTILS_H__
