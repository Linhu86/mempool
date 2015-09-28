#ifndef __STANDARD_MEMORY_POOL_HPP__
#define __STANDARD_MEMORY_POOL_HPP__

#include "MemoryPool.hpp"
#include "memlog.hpp"
#include <malloc.h>
#include <math.h>
#include <string>
#include <string.h>

class StandardMemoryPool : public MemoryPool
{
  public:
    void *allocate(uint32 size);
    void free(void *ptr);
    int integrityCheck() const;
    void dumpToFile(const std::string& fileName, const uint32 itemsPerLine) const;

    static const uint8 s_minFreeBlockSize = 16;

    friend class MemoryPoolManager;
    StandardMemoryPool(uint32 sizeInBytes, int boundsCheck)
    {

#ifdef DEBUG_ON
      mem_log("StandardPool Constructor initialization.\n");
#endif
    
      if(boundsCheck){
        m_boundsCheck;
      }        

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

  private:
    class Chunk
    {
      public:
        Chunk(uint32 userDataSize) :m_next(NULL), m_prev(NULL), m_userdataSize(userDataSize), m_free(true) {}
        ~Chunk(){}
        void write(void *dest){ memcpy(dest, this, sizeof(Chunk)); }
        void read(void *src) { memcpy (this, src, sizeof(Chunk)); }

        Chunk *m_prev;
        Chunk *m_next;
        uint32 m_userdataSize;
        int m_free;
    };

    uint8 * m_poolMemory;

};



#endif



