#ifndef __ALLOCATION_HPP__
#define __ALLOCATION_HPP__

typedef unsigned int uint32
typedef unsigned char uint8


/* Overload operator new */
inline void *operator new(size_t size, MemoryPool &pool)
{
  void *ptr = pool.allocate(size);

  assert(ptr);

  return ptr;
}

/* Overload operator delete */
inline void *operator delete(void *ptr, MemoryPool &pool)
{
  pool.free(ptr);
}

template <class T>
void __delete__(T *ptr, MemoryPool &pool)
{
  if(ptr)
  {
    ptr->~T();
    pool.free(ptr);
  }
}


#undef DELETE 

#define NEW(memoryPool)     new(*memoryPool)
#define DELETE(memoryPool)  __delete__(ptr, *memoryPool)
#define POOL(name)          (MemoryPoolManager::it().getPool(name))


#endif




