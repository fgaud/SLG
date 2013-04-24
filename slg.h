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

#ifndef SLG_H_
#define SLG_H_

//Specify the debug level
#define DEBUG_LEVEL -1
#include "master_slave.h"

#include "circular_buffer.h"
#include "debug_tools.h"
#include "stats_utils.h"

#if DEBUG_LEVEL == 1
#define MAX_SESSION_DUMP 50
#endif //DEBUG_LEVEL 1


/**** Protocol ****/
#if HTTP_PROTOCOL
#define BUILD_REQUEST build_http_request

#elif MEMCACHED_PROTOCOL
#define BUILD_REQUEST build_memcached_request

#else
#error You must specify either -DHTTP_PROTOCOL or -DMEMCACHED_PROTOCOL
#endif

/**** Socket options ****/

/** Send/Receive buffer size **/
#define OPT_USE_DEFAULT_SOCK_BUF_SIZE -1

/** Use Non blocking socket **/
#define NON_BLOCKING_SOCKET 1

// Number of maximum clients which this injector can simulate
#define MAX_CLIENTS 1024

// Default server dir (generally /) ended by a /
#define DEFAULT_DIR "/"

/**** Some constants ****/
/* Max http headers size */
/* This value is used to reset client's buffer and to reduce memset costs*/
#define MAX_HDR_LENGTH 2048

#if MEMCACHED_PROTOCOL
#define REQUEST_BUFFER_SIZE (2048)
#elif SPECWEB05_FILE_ACCESS_PATTERN
#define REQUEST_BUFFER_SIZE (1024)
#else
#define REQUEST_BUFFER_SIZE (512)
#endif

/*Nb attempt of connection failed before considering server unreachable*/
#define NB_CONNECT_ATTEMPTS_MAX 1

#define IGNORE_HTTP_CONTENT	  1

/**** Clients structs ****/
/* Possible states for a client */
typedef enum states_t {
	ST_WAITING, ST_READING, ST_WRITING, ST_CONNECT
} states_t;


/* Structure representing a client */
typedef struct {
	int number;
	// Socket used by the client
	int fd;
	// Current state
	states_t state;
	// Number of requests done during the current iteration
	unsigned int nbRequestsOnIteration;
	// number of bytes read
	unsigned int bytes_read;
	// number of bytes written
	unsigned int bytes_write;

	// number of errors
	unsigned long errors;

	// Read buffer
	unsigned int content_length;
	unsigned int header_length;
	char read_hdr_buf[MAX_HDR_LENGTH];
	char* read_content_buf;

	// Request pre-allocated buffer
	int new_request;
	char request_buf[REQUEST_BUFFER_SIZE];
	unsigned int request_total_len;

	// Port used by this client
	unsigned short port;

	int connect_in_progress;

	int nb_connect_attempts;

	//All stats attached to a client.

	//TODO
	// Number of bytes received
	unsigned long long total_bytes_recv;
	unsigned long total_resp_recv;

	// Connection start time :
	struct timeval CTstart;
	struct timeval RTstart;
	struct timeval CRTstart;

	pthread_mutex_t state_mutex;

	unsigned int req_unique_id;

	/** Target server for all clients */
	struct hostent *ent;
	struct sockaddr_in soc_address;

	int current_target;
} client_t;

#define SET_IF_MIN(min, val) if(val<min){min = val;}
#define SET_IF_MAX(max, val) if(val>max){max = val;}
#endif /*SLG_H_*/
