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

#ifndef MASTER_SLAVE_H_
#define MASTER_SLAVE_H_

#include "debug_tools.h"

#define UNIQUE_FILE_ACCESS_PATTERN      1
#define SPECWEB99_FILE_ACCESS_PATTERN   0
#define SPECWEB05_FILE_ACCESS_PATTERN   0

// Other protocol
#define CLOSE_AFTER_REQUEST             0


#if SPECWEB99_FILE_ACCESS_PATTERN
// For specweb99 explaination see http://www.spec.org/web99/docs/whitepaper.html
#define SPECWEB99_NB_CLASS                      4
#define SPECWEB99_NB_FILES                      9
#define SPECWEB99_LOAD                          1000


// Frequency of each class
const double class_freq[] =                     {0.35, 0.85, 0.99, 1};

//fancy class frequencies to show increased throughput (DON'T USE FOR BENCH, NOT RELEVANT)
//const double class_freq[] =                     {0.35, 0.55, 0.70, 1};

// File frequency within each class
const double file_freq[] =                     {0.039, 0.098, 0.186, 0.363, 0.716, 0.834, 0.905, 0.955, 1};


#elif SPECWEB05_FILE_ACCESS_PATTERN
// For specweb99 explaination see http://www.spec.org/web2005/docs/SupportDesign.html
#define SPECWEB05_NB_CLASS                      6
#define SPECWEB05_NB_FILES                      16
#define SPECWEB05_MAX_FILE_PER_CLASS            5

#define SPECWEB05_SIMULTANEOUS_SESSIONS         1000
//#define SPECWEB05_DIRSCALING                    0.25  //Legacy value but doesn't fit in memory
#define SPECWEB05_DIRSCALING                    0.00300
#define SPECWEB05_ZIPF_ALPHA                    1.2

// Frequency of each class
const double class_freq[] =                     {0.1366, 0.2627, 0.5467, .7699, .8949, 1.};
// Order of file popularity within each class
const double file_freq[SPECWEB05_NB_CLASS][SPECWEB05_MAX_FILE_PER_CLASS]
                                           =    {
                                                    {0.273, 0.364, 0.529, 0.715,  1.00},
                                                    {0.579, 0.757,  1.00, -1.00, -1.00},
                                                    {0.275, 0.445, 0.615,  1.00, -1.00},
                                                    {0.666,  1.00, -1.00, -1.00, -1.00},
                                                    { 1.00, -1.00, -1.00, -1.00, -1.00},
                                                    { 1.00, -1.00, -1.00, -1.00, -1.00}
                                           };

/** PHP **/
#define PHP_NB_FILES                            6

const char* php_files[PHP_NB_FILES] = {
         "index.php",
         "fileCatalog.php?id=3279&category=4&os=4&lang=9",
         "search.php?q=cdrom+wireless+memory",
         "file.php?id=36043",
         "product.php?id=1812",
         "catalog.php?id=1165"
};

/** Images **/
#define IMAGES_NB_FILES                         31
const char* images_files[IMAGES_NB_FILES] = {
         "template_javascripts",
         "flattab_sl",
         "h_product_selection",
         "blue_arrow_top",
         "print",
         "email",
         "flattab_nr",
         "bar",
         "button-1",
         "nav_q",
         "us",
         "help",
         "aaa",
         "masthead_transparent",
         "button-3",
         "masthead_local_sep",
         "H_Service_Tag_Unkown",
         "content_arrow",
         "masthead_global",
         "global",
         "content_action",
         "H_D",
         "masthead_subnavsep",
         "note",
         "button-2",
         "___",
         "flattab_nl",
         "blue_arrow_right",
         "flattab_sr",
         "ccc",
         "spacer"
};

/** Workload Mix **/
/** images, PHP, downloads **/
const double type_freq[] = {
         0.85,
         0.99,
         1
};
#endif

/** Internal for communication **/
// Master orders buffer size
#define MASTER_ORDER_BUFFER_SIZE                256

// Response max size
#define RESPONSE_SIZE	                        1024

#if UNIQUE_FILE_ACCESS_PATTERN + SPECWEB99_FILE_ACCESS_PATTERN + SPECWEB05_FILE_ACCESS_PATTERN != 1
#error 'You must choose a (unique) file workload'
#endif


#ifdef __x86_64__
#define rdtscll(val) { \
    unsigned int __a,__d;                                        \
    asm volatile("rdtsc" : "=a" (__a), "=d" (__d));              \
    (val) = ((unsigned long)__a) | (((unsigned long)__d)<<32);   \
}

#else
   #define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))
#endif

#endif /*MASTER_SLAVE_H_*/
