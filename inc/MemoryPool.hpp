#ifndef __MEMORY_POOL_HPP__
#define __MEMORY_POOL_HPP__

#include<iostream>

#ifdef _DEBUG
#define TRASH_POOLS 1
#else
#define TRASH_POOLS 0
#endif

typedef unsigned int uint32
typedef unsigned char uint8


class Chunk
{
  public:
      Chunk(uint32 size);
      ~Chunk();
      int write(void *dest);
      int read(void *src);

  private:
      Chunk *m_prev;
      Chunk *m_next;
      uint32 datasize;
      int free;
};

class MemoryPool
{
  public:
    virtual void *allocate(uint32 size) = 0;
    virtual void free(void *ptr) = 0;
    virtual int integrityCheck() const = 0;
    virtual void dumpToFile(const std::string& fileName, const uint32 itemsPerLine) const = 0;
    inline unit32 getFreePoolSize() { return m_freePoolSize; }
    inline unit32 getTotalPoolSize() { return m_totalPoolSize; }
    inline unit32 getBoundsCheck() { return m_boundsCheck; }
    
    static const uint8   s_trashOnCreation = 0xCC;
    static const uint8   s_trashOnAllocSignature = 0xAB;
    static const uint8   s_trashOnFreeSignature  = 0xFE;
    static const uint8   s_boundsCheckSize = 16;
    static const uint8   s_startBound[s_boundsCheckSize];
    static const uint8   s_endBound[s_boundsCheckSize];

  protected:
     MemoryPool() 
        : m_totalPoolSize(0),
          m_freePoolSize(0),
          m_boundsCheck(0),
          m_trashOnCreation(0),
          m_trashOnAlloc(0),
          m_trashOnFree(0)
          {};
     
    virtual ~MemoryPool();

    uint32       m_totalPoolSize;
    uint32       m_freePoolSize;

    // Bitfield
    unsigned    m_trashOnCreation : 1;
    unsigned    m_trashOnAlloc : 1;
    unsigned    m_trashOnFree : 1;
    unsigned    m_boundsCheck : 1;

};

#endif


