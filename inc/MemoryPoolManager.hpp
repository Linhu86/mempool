#ifndef __MEMORY_POOL_MANAGER__
#define __MEMORY_POOL_MANAGER__

#include "MemoryPool.hpp"
#include <map>
#include <string>
#include <iostream>
using namespace std;

class MemoryPoolManager
{
  public:
    static const char* const c_poolsFileXML;
    static MemoryPoolManager& it() { 
      static MemoryPoolManager instance(c_poolsFileXML); 
      return instance; 
    }

    MemoryPool * getPool(const char * const name)
    {
      std::map<std::string, MemoryPool*>::iterator it = m_pool.find(name);
      if(it != m_pool.end())
      {
        return it->second;
      }
      else
      {
        std::cout << "Pool " << std::string(name) << " not found." << endl;
        return NULL;
      }
    }

    void dumpAllPools() const
    {
      dumpAllPools("");
    }

    /* Dump all the pools to some txt files*/
    void  dumpAllPools(const std::string& prefix) const
    { 
      std::map<std::string, MemoryPool*>::const_iterator it = m_pool.begin(); 
      for(it; it != m_pool.end(); ++it)
      {
        it->second->dumpToFile(prefix+std::string(it->first)+".txt", 4);

      }
    }

    /* Dump pool to some txt files*/
    void dumpPool(const std::string& prefix, const MemoryPool* pool) const
    { 
      std::map<std::string, MemoryPool*>::const_iterator it = m_pool.begin(); 
      for(it; it != m_pool.end(); ++it)
      {
        if(pool == it->second)
        {
          it->second->dumpToFile(prefix+std::string(it->first)+".txt", 4);
        }
      }
    }
  
  private:
    MemoryPoolManager(const char * const poolsFileXML);
    int  isAValidPoolType(const char* poolType) const;
    MemoryPool*  allocateByPoolType(const char* poolType) const;

    std::map<std::string, MemoryPool*> m_pool;
};

#endif



