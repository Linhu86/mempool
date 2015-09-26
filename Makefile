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
LIBMEMPOOL_LIB  = -lmempool


CFLAGS  += $(LIBXML_INCL) $(LIBMEMPOOL_INCL)
LDFLAGS += $(LIBXML_LIB) $(LIBMEMPOOL_LIB)

LIB_LINK_MODE = -static

#--------------------------------------------------------------------

LIBXMLOBJS = $(LIBXML_SRC)/tinystr.o \
             $(LIBXML_SRC)/tinyxml.o \
             $(LIBXML_SRC)/tinyxmlerror.o \
             $(LIBXML_SRC)/tinyxmlparser.o 

LIBMEMPOOLOBJS = $(LIBMEMPOOL_SRC)/MemoryPool.o \
                                 $(LIBMEMPOOL_SRC)/MemoryPoolManager.o \
                                 $(LIBMEMPOOL_SRC)/StandardMemoryPool.o

TARGET =  libtinyxml.so libtinyxml.a libmempool.so libmempool.a mempool_test

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

mempool_test: src/main.o
	$(LINKER) $^ $(LIB_LINK_MODE) -L. -ltinyxml -lmempool $(LDFLAGS) -o $@

clean:
	@( $(RM) $(LIBXML_SRC)/*.o $(LIBMEMPOOL_SRC)/*.o vgcore.* core core.* $(TARGET) )

#--------------------------------------------------------------------

# make rule
%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@	

%.o : %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@	


