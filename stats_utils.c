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

#include "stats_utils.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NO_YET_IMPLEMENTED printf("%s, l.%d: not yet implemented.\n", __FUNCTION__, __LINE__); exit(EXIT_FAILURE)

/******************************************************************************************/
void init_stats_sample_Lf(stats_sample_Lf_t * sample, int size_max, char * name){
	assert(sample !=NULL);
	sample->s = calloc(size_max, sizeof(long double));
	assert(sample->s !=NULL);	
	sample->nb_elem = 0;
	sample->size_max = size_max;
	sample->name = name;

}

/******************************************************************************************/
void destroy_stats_sample_Lf(stats_sample_Lf_t * sample){
	assert(sample->s !=NULL);
	free(sample->s);
	sample->s = NULL;
	sample->nb_elem = 0;
	sample->size_max = 0;
	sample->name = NULL;
}

/******************************************************************************************/
void insert_Lf(stats_sample_Lf_t *sample, long double val){
	assert(sample->nb_elem < sample->size_max);
	sample->s[sample->nb_elem] = val;
	sample->nb_elem++;
}

/******************************************************************************************/
void  dump_Lf(stats_sample_Lf_t *sample){
	unsigned long i;
		for (i=0; i<sample->nb_elem; i++){
			printf("DUMP [%s]->s[%lu]: %Lf\n", sample->name, i, sample->s[i]);
		}
}

/******************************************************************************************/
long double _helper_average_Lf(long double *tab, unsigned long start_i, unsigned long end_i){

	long double avg = 0;
	unsigned long i;
	
	for (i=start_i; i<=end_i; i++){
		avg += tab[i];
	}
	avg = avg / (long double)(end_i-start_i+1);
	
	return avg;
}

/******************************************************************************************/
long double _helper_stddev_Lf(long double *tab, unsigned long start_i, unsigned long end_i){

	long double avg = 0;
	long double stddev=0;
	unsigned long i;
	
	avg = _helper_average_Lf(tab, start_i, end_i);
	
	for (i=start_i; i<=end_i; i++)	{
		stddev += powl(tab[i]-avg, 2);
	}
	
	stddev = sqrtl(stddev / (long double)(end_i-start_i+1));
	
	return stddev;
}

/******************************************************************************************/
long double average_Lf(stats_sample_Lf_t *sample) {
	assert(sample !=NULL);
	
	if (sample->nb_elem == 0){
		return 0;
	}
	
	return _helper_average_Lf(sample->s, 0, sample->nb_elem-1);
	
}

/******************************************************************************************/
long double stddev_Lf(stats_sample_Lf_t *sample){
	assert(sample !=NULL);
	
	if (sample->nb_elem == 0){
			return 0;
	}
	
	return _helper_stddev_Lf(sample->s, 0, sample->nb_elem-1);
}

/******************************************************************************************/
long double interval_average_Lf(stats_sample_Lf_t *sample, int percent){
	assert(sample !=NULL);
	assert(percent>0 && percent<=100);
	
	if (sample->nb_elem == 0){
		return 0;
	}
	
	float side_percent_to_cut = ((100.0-(float)percent)/2.0)/100.0; 
	unsigned long int start_i = (unsigned long)floor(sample->nb_elem * side_percent_to_cut);
	unsigned long int end_i = sample->nb_elem - start_i;	
	assert(end_i > start_i);
	
	return _helper_average_Lf(sample->s, start_i, end_i-1);
}

/******************************************************************************************/
long double interval_stddev_Lf(stats_sample_Lf_t *sample, int percent){
	
	assert(sample !=NULL);
	assert(percent>0 && percent<=100);
	
	if (sample->nb_elem == 0){
		return 0;
	}
	
	float side_percent_to_cut = ((100.0-(float)percent)/2.0)/100.0; 
	
	unsigned long int start_i = (unsigned long)floor(sample->nb_elem * side_percent_to_cut);
	unsigned long int end_i = sample->nb_elem - start_i;	
	assert(end_i > start_i);
	
	return _helper_stddev_Lf(sample->s, start_i, end_i-1);

}

/******************************************************************************************/
int compare_elem(const void *a,const void *b){
	if((*(long double *) a) < (*(long double *) b)){
		return -1;
	}
	else if((*(long double *) a) > (*(long double *) b)){
		return 1;
	}
	else{
		return 0;
	}
}

/******************************************************************************************/
long double percentil_average_Lf(stats_sample_Lf_t *sample, int percentil){
	assert(sample->s !=NULL);
	assert(percentil>0 && percentil<=100);
		
	if (sample->nb_elem == 0){
		return 0;
	}
	
	float side_percent_to_cut = ((100.0-(float)percentil)/2.0)/100.0; 
	unsigned long int start_i = (unsigned long)floor(sample->nb_elem * side_percent_to_cut);
	unsigned long int end_i = sample->nb_elem - start_i;	
	assert(end_i > start_i);
	
	long double * temp =  (long double *)calloc(sample->nb_elem, sizeof(long double));
	assert(temp != NULL);
	memcpy(temp, sample->s, sample->nb_elem*sizeof(long double));
	qsort(temp , sample->nb_elem, sizeof(long double), compare_elem);
	
	long double avg = _helper_average_Lf(temp, start_i, end_i-1);
	
	free(temp);
	
	return avg;	
}

/******************************************************************************************/
long double percentil_stddev_Lf(stats_sample_Lf_t *sample, int percentil) {
	assert(sample->s !=NULL);
	assert(percentil>0 && percentil<=100);

	if (sample->nb_elem == 0) {
		return 0;
	}

	float side_percent_to_cut = ((100.0-(float)percentil)/2.0)/100.0;
	unsigned long int start_i = (unsigned long)floor(sample->nb_elem * side_percent_to_cut);
	unsigned long int end_i = sample->nb_elem - start_i;
	assert(end_i > start_i);

	long double * temp = (long double *)calloc(sample->nb_elem, sizeof(long double));
	assert(temp != NULL);
	memcpy(temp, sample->s, sample->nb_elem*sizeof(long double));
	qsort(temp, sample->nb_elem, sizeof(long double), compare_elem);

	long double avg = _helper_stddev_Lf(temp, start_i, end_i-1);

	free(temp);

	return avg;
}

/******************************************************************************************/
long double min_Lf(stats_sample_Lf_t *sample){
	assert(sample !=NULL);
	assert(sample->nb_elem > 0);
	
	unsigned long i;
	
	long double min = sample->s[0];
	
	for (i=0; i<sample->nb_elem; i++){
		if(min > sample->s[i]){
			min = sample->s[i];
		}
	}
	
	return min;
}

/******************************************************************************************/
long double max_Lf(stats_sample_Lf_t *sample){
	assert(sample !=NULL);
	assert(sample->nb_elem > 0);
	
	unsigned long i;
	long double max = sample->s[0];
	
	for (i=0; i<sample->nb_elem; i++){
		if(max < sample->s[i]){
			max = sample->s[i];
		}
	}
	
	return max;
}

/******************************************************************************************/
/**********************Table functions*************************************/
double t_average_f(double *table,int n) {
	assert(table!=NULL || n>0);
	double moy = 0;
	int i;
	//average computation
	for (i=0; i<n; i++){
		moy += table[i];
	}
	moy /= n;
	return moy;
}

double t_average_ul(unsigned long *table,int n) {
	assert(table!=NULL || n>0);
	double moy = 0;
	int i;
	//average computation
	for (i=0; i<n; i++){
		moy += table[i];
	}
	moy /= n;
	return moy;
}


unsigned long t_minSearch_ul(unsigned long *table,int n){
	assert(table!=NULL || n>0);
	int i;
	unsigned long min = table[0];
	for( i=1 ; i<n ; i++){
		if(min > table[i]){
			min = table[i];
		}
	}
	return min;
}


unsigned long t_maxSearch_ul(unsigned long *table,int n){
	assert(table!=NULL || n>0);
	int i;
	unsigned long max = table[0];
	for( i=1 ; i<n ; i++){
		if(max < table[i]){
			max = table[i];
		}
	}
	return max;
}

