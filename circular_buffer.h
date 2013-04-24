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

#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#define CIRC_BUF_ERROR -1

/*****************************************************************
 *****************************************************************
 * A "thread-safe lock free" circular buffer (See Note).
 * Note: only with ONE producer and ONE consumer
 * 
 * inspired from: 
 * 	-http://www.cppfrance.com/codes/BUFFER-CIRCULAIRE_41402.aspx
 * 
 * **************************************************************
 * **************************************************************/




/* **************************************************
 * The structure used to manage a circular buffer.
 * **************************************************/
typedef struct
{
int *buffer;
int size;
int writePos;
int readPos;
} circularBuffer;


/**************************************
 * Create a new circular buffer.
 * 
 * WARNING 1: In case of an error get and put functions returns CIRC_ERROR
 * which is -1. So you can't have -1 value in aa circular buffer.
 * 
 * WARNING 2: the size of the circular buffer is the amount of data you can effeectivelly read.
 * But, notice that the circular buffer allocates size+1 cases to works fine.
 *****************************************/
circularBuffer * open_circ(circularBuffer *cb, int size);

/**************************************************
 * free a circular buffer.
 * *************************************************/
void close_circ(circularBuffer *cb);

/**************************************************
 * Get the head element from the circular buffer.
 * *************************************************/
int get_circ(circularBuffer *cb);


/**************************************************
 * Put an element in the circular buffer.
 * ***********************************************/
int put_circ(circularBuffer *cb, int e);


/******************************************
 * Printf a circular buffer.
 * ****************************************/
void printfCircularBuffer(circularBuffer* cb);




#endif /*_CIRCULAR_BUFFER_H_*/

