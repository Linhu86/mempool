#ifndef __STANDARD_MEMORY_POOL_HPP__
#define __STANDARD_MEMORY_POOL_HPP__

#include "MemoryPool.hpp"
#include "memlog.hpp"
#include <malloc.h>
#include <math.h>
#include <string>
#include <string.h>

//max memory pool size 2G.
#define MAX_MEMPOOL_SIZE 2*1024*1024*1024UL
#define DUMP_ELEMENT_PER_LINE 4
#define BLOCK_NAME_LEN 16
#define DEFAULT_BLOCK_NAME "unknown"
#define MEMORY_POOL_BLOCK_NAME "pool"


class Chunk
{
  public:
    Chunk(uint32 userDataSize) :m_next(NULL), m_prev(NULL), m_userdataSize(userDataSize), m_free(true) {
      strncpy(m_name, DEFAULT_BLOCK_NAME, strlen(DEFAULT_BLOCK_NAME));
    }
    ~Chunk(){}
    inline void write(void *src){ memcpy(src, this, sizeof(Chunk)); }
    inline void read(void *dest) { memcpy (this, dest, sizeof(Chunk)); }

    int name_set(const char *name);

    Chunk *m_prev;
    Chunk *m_next;
    uint32 m_userdataSize;
    uint32 m_free;
    char m_name[BLOCK_NAME_LEN];
};

class StandardMemoryPool : public MemoryPool
{
  public:
    StandardMemoryPool(uint64 sizeInBytes, uint32 boundsCheck);
    ~StandardMemoryPool();
    void *allocate(uint64 size);
    int free(void *ptr);
    int integrityCheck() const;
    void dumpToFile(const std::string& fileName, const uint32 itemsPerLine, const uint32 format) const;
    void dumpToStdOut(const uint32 ElemInLine, const uint32 format) const;
    void memory_block_list();
    void memory_pool_info();
    static const uint8 s_minFreeBlockSize = 16;
    friend class MemoryPoolManager;

  private:
    uint8 * m_poolMemory;
    uint32 m_poolSize;

};



#endif



