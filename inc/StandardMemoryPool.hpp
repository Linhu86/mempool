#ifndef __STANDARD_MEMORY_POOL_HPP__
#define __STANDARD_MEMORY_POOL_HPP__

#include "MemoryPool.h"
#include "malloc.h"
#include "math.h"
#include <string>

class StandardMemoryPool : public MemoryPool
{
  public:
    void *allocate(uint32 size);
    void free(void *ptr);
    int  integrityCheck() const;
    void dumpToFile(const std::string& fileName, const DWORD itemsPerLine) const;

    staic const uint8 s_minFreeBlockSize = 16;

  private:
    StandardMemoryPool(uint32 sizeInBytes, bool boundsCheck)
    {
      if(boundsCheck)
        m_boundsCheck;

      m_poolMemory = ::new uint8[sizeInBytes];

      m_freePoolSize = sizeInBytes - sizeof(Chunk);
      m_totalPoolSize = sizeInBytes;

      // Trash it if required
      if(m_trashOnCreation)
          memset(m_poolMemory, s_trashOnCreation, sizeInBytes);

      if(m_boundsCheck)
      {
        m_freePoolSize -= s_boundsCheckSize * 2;
        Chunk freeChunk(sizeInBytes - sizeof(Chunk) - 2 * s_boundsCheckSize);
        freeChunk.write(m_poolMemory + s_boundsCheckSize);
        memcpy(m_poolMemory, s_startBound, s_boundsCheckSize);
        memcpy(m_poolMemory + sizeInBytes - s_boundsCheckSize, s_endBound, s_boundsCheckSize);
      }
      else
      {
        Chunk freeChunk(sizeInBytes - sizeof(Chunk));
        freeChunk.write(m_poolMemory);
      }

    }

    ~StandardMemoryPool()
    {
    }
    
    class Chunk
    {
      public:
        Chunk(uint32 userDataSize) :m_next(NULL), m_prev(NULL), datasize(userDataSize), free(true) {}
        ~Chunk(){}
        int write(void *dest){ memcpy(dest, this, sizeof(Chunk)); }
        int read(void *src) { memcpy (this, src, sizeof(Chunk)); }

      private:
        Chunk *m_prev;
        Chunk *m_next;
        uint32 datasize;
        int free;
    };

    uint8 * m_poolMemory;

};



#endif



