#ifndef __STANDARD_MEMORY_POOL_HPP__
#define __STANDARD_MEMORY_POOL_HPP__

#include "MemoryPool.hpp"
#include "memlog.hpp"
#include <malloc.h>
#include <math.h>
#include <string>
#include <string.h>

//max memory pool size 1G.
#define MAX_MEMPOOL_SIZE 2*1024*1024*1024
#define DUMP_ELEMENT_PER_LINE 4
#define BLOCK_NAME_LEN 16
#define DEFAULT_BLOCK_NAME "unknown"
#define MEMORY_POOL_BLOCK_NAME "pool"

typedef enum
{
  DUMP_ = 0,
  DUMP_HEX,
  DUMP_CHAR
} dump_type;


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
    void *allocate(uint32 size);
    int free(void *ptr);
    int integrityCheck() const;
    void dumpToFile(const std::string& fileName, const uint32 itemsPerLine) const;
    void dumpToStdOut(uint32 ElemInLine, uint32 format) const;
    void memory_block_list();
    void memory_pool_info();

    static const uint8 s_minFreeBlockSize = 16;

    friend class MemoryPoolManager;
    StandardMemoryPool(uint32 sizeInBytes, int boundsCheck)
    {
      m_poolSize = sizeInBytes;

      m_boundsCheck = boundsCheck;

      m_poolMemory = new uint8[sizeInBytes];

#ifdef DEBUG_ON
      mem_debug_log("StandardPool Constructor initialization in address %p with size %d\n", m_poolMemory, sizeInBytes);
      printf("StandardPool created with m_trashOnCreation:%d m_trashOnAlloc: %d  m_trashOnFree :%d m_boundsCheck %d\n", m_trashOnCreation, m_trashOnAlloc, m_trashOnFree, m_boundsCheck);
      if(boundsCheck){
        mem_debug_log("Memory pool bounds check feature present.\n");
      }
#endif

      m_freePoolSize = sizeInBytes - sizeof(Chunk);
      m_totalPoolSize = sizeInBytes;

      // Trash it if required
      if(m_trashOnCreation)
      {
          memset(m_poolMemory, s_trashOnCreation, sizeInBytes);
      }

      if(m_boundsCheck)
      {
        m_freePoolSize -= s_boundsCheckSize * 2;
        Chunk freeChunk(sizeInBytes - sizeof(Chunk) - 2 * s_boundsCheckSize);
        freeChunk.name_set(MEMORY_POOL_BLOCK_NAME);
        freeChunk.write(m_poolMemory + s_boundsCheckSize);
        memcpy(m_poolMemory, s_startBound, s_boundsCheckSize);
        memcpy(m_poolMemory + sizeInBytes - s_boundsCheckSize, s_endBound, s_boundsCheckSize);
        freeChunk.m_next = NULL;
        freeChunk.m_prev = NULL;
      }
      else
      {
        Chunk freeChunk(sizeInBytes - sizeof(Chunk));
        freeChunk.name_set(MEMORY_POOL_BLOCK_NAME);
        freeChunk.write(m_poolMemory);
        freeChunk.m_next = NULL;
        freeChunk.m_prev = NULL;
      }

#ifdef DEBUG_ON
       dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_CHAR);
#endif

    }

    ~StandardMemoryPool()
    {
#ifdef DEBUG_ON
      mem_debug_log("[%s] StandardPool Deonstructor deconstruction.", __FUNCTION__);
#endif
      delete [] m_poolMemory;
    }

  private:
    uint8 * m_poolMemory;
    uint32 m_poolSize;

};



#endif



