#--------------------------------------------------------------------
CC = gcc
AR = ar cru
CFLAGS = -Wall -D_REENTRANT -D_GNU_SOURCE -g -fPIC
SOFLAGS = -shared
LDFLAGS = -lstdc++ -lpthread

LINKER = $(CC)
LINT = lint -c
RM = /bin/rm -f

LIBXML_INC = TinyXML/inc
LIBXML_SRC = TinyXML/src
LIBXML_INCL = -I$(LIBXML_INC)
LIBXML_LIB  = -ltinyxml

LIBMEMPOOL_INC = inc
LIBMEMPOOL_SRC = src
LIBMEMPOOL_INCL = -I$(LIBMEMPOOL_INC)
LIBMEMPOOL_LIB  = -lmemorypool


CFLAGS  += $(LIBEVENT_INCL)
LDFLAGS += $(LIBEVENT_LIB)

LIB_LINK_MODE = -static

#--------------------------------------------------------------------

LIBXMLOBJS = $(LIBMEMPOOL_SRC)/tinystr.o \
             $(LIBXML_INC)/tinyxml.o \
             $(LIBXML_INC)/tinyxmlerror.o \
             $(LIBXML_INC)/tinyxmlparser.o 

LIBMEMPOOLOBJS = $(LIBMEMPOOL_SRC)\MemoryPoolManager.cpp
                 $(LIBMEMPOOL_SRC)\StandardMemoryPool.cpp

TARGET =  libtinyxml.so libtinyxml.a libmempool.so libmemory.a mempool_test

#--------------------------------------------------------------------

all: $(TARGET)

libtinyxml.so: $(LIBXMLOBJS)
	$(LINKER) $(SOFLAGS) $^ -o $@

libtinyxml.a: $(LIBXMLOBJS)
	$(AR) $@ $^

libmempool.so: $(LIBMEMPOOLOBJS)
	$(LINKER) $(SOFLAGS) $^ -o $@

libmempool.a: $(LIBMEMPOOLOBJS)
	$(AR) $@ $^

mempool: src/main.o
	$(LINKER) $^ $(LIB_LINK_MODE) -L. -ltinyxml -lmempool $(LDFLAGS) -o $@

clean:
	@( $(RM) *.o vgcore.* core core.* $(TARGET) )

#--------------------------------------------------------------------

# make rule
%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@	

%.o : %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@	
