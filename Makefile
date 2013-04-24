CFLAGS = -W -Wall -Werror -O2 -g
#CFLAGS = -W -Wall -Werror -O0 -g3
#DEBUG_FLAGS = -DDEBUG_TASK -DDEBUG_RUID


SLG_FLAGS = -DHTTP_PROTOCOL -DUSE_RST
#SLG_FLAGS = -DHTTP_PROTOCOL -DUSE_RST
#SLG_FLAGS = -DMEMCACHED_PROTOCOL -DUSE_RST
#-DUSE_RST
#-DHTTP_PROTOCOL
#-DMEMCACHED_PROTOCOL
#-DUSE_RST

LDFLAGS = -lm -lpthread 

# Rules
APPS = slg slg_master 

all: ${APPS}

CC = gcc

# Build libraries
%.o : %.c %.h master_slave.h debug_tools.h
	${CC} $(CFLAGS) $(SLG_FLAGS) $(DEBUG_FLAGS) $(INCFLAGS) -c $<

# Binaries
slg: slg.o circular_buffer.o stats_utils.o debug_tools.o
	${CC} -o $@ $^ $(LDFLAGS) 

slg_master: slg_master.o stats_utils.o debug_tools.o
	${CC} -o $@ $^ $(LDFLAGS) 

clean:
	rm -f ${APPS}
	rm -f *.o
	rm -f core*

