#--------------------------------------------------------------------
CC = gcc
AR = ar cru
CFLAGS = -Wall -D_REENTRANT -D_GNU_SOURCE -g -fPIC
SOFLAGS = -shared
LDFLAGS = -lstdc++ -lpthread -ldl

LINKER = $(CC)
LINT = lint -c
RM = /bin/rm -f

LIBMEMPOOL_INC = inc
LIBMEMPOOL_SRC = src
LIBMEMPOOL_INCL = -I$(LIBMEMPOOL_INC)
LIBMEMPOOL_LIB  = -lmempool

UNITEST_LIB = -lcppunit


CFLAGS  += $(LIBXML_INCL) $(LIBMEMPOOL_INCL)
LDFLAGS += $(LIBXML_LIB) $(LIBMEMPOOL_LIB)

#CFLAGS += -DMEM_DEBUG_ON
#CFLAGS += -DMEM_ERROR_MSG_ON

#CFLAGS += -DTEST_DEBUG_ON
CFLAGS += -DTEST_ERROR_MSG_ON

LIB_LINK_MODE = -static

#--------------------------------------------------------------------

LIBMEMPOOLOBJS = $(LIBMEMPOOL_SRC)/MemoryPool.o \
                 $(LIBMEMPOOL_SRC)/StandardMemoryPool.o

TARGET = libmempool.so libmempool.a mempool_test mempool_unitest

#--------------------------------------------------------------------

all: $(TARGET)

libmempool.so: $(LIBMEMPOOLOBJS)
	$(LINKER) $(SOFLAGS) $^ -o $@

libmempool.a: $(LIBMEMPOOLOBJS)
	$(AR) $@ $^

mempool_test: src/main.o
	$(LINKER) $^ $(LIB_LINK_MODE) -L. $(LIBMEMPOOL_LIB) $(LDFLAGS) -g -o $@

mempool_unitest: unit_test/MemoryPoolUnitTest.cpp
	$(LINKER) $^ $(LIB_LINK_MODE) $(CFLAGS) $(LDFLAGS) $(UNITEST_LIB) -L. $(LIBMEMPOOL_LIB) -g -o $@

clean:
	@( $(RM) $(LIBXML_SRC)/*.o $(LIBMEMPOOL_SRC)/*.o vgcore.* core core.* $(TARGET) )

#--------------------------------------------------------------------

# make rule
%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@	

%.o : %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@	


