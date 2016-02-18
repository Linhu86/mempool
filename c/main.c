#include <stdio.h>
#include <stdlib.h>

#include "ucom_rtos_mem.h"

#define UCOM_RTOS_DATA_POOL_YOCTO_HEADER_SIZE MEMORY_POOL_HEADER_SIZE + MEMORY_CHUNK_HEADER_SIZE

#define UCOM_OSMEM_ALLOC_POOL_MEMAREA_STATIC(osmem_pool_name, osmem_pool_size)        \
        static unsigned int osmem_pool_name[(osmem_pool_size + UCOM_RTOS_DATA_POOL_YOCTO_HEADER_SIZE + 3)/sizeof(unsigned int)]



int main()
{
  UComInt32 res;
  MemParam_t params;

  MemoryPool_t mem;
  MemoryPool_t *p_mem = &mem;
  MemoryPool_t ** pmem = &p_mem;

  MemoryPool_t mem2;
  MemoryPool_t *p_mem2 = &mem2;
  MemoryPool_t ** pmem2 = &p_mem2;


  UComUInt32 *ptr = NULL;
  UComUInt32 *ptr2 = NULL;


  UCOM_OSMEM_ALLOC_POOL_MEMAREA_STATIC(test1, 1024);
  UCOM_OSMEM_ALLOC_POOL_MEMAREA_STATIC(test2, 10240);

  UComOsMemCreatePool(pmem, test1, 1024);

  UComOsMemAlloc(*pmem, 200, (void **)&(ptr));

  memory_pool_info(*pmem);

  UComOsMemFree(ptr);

  UComOsMemAlloc(*pmem, 100, (void **)&(ptr));

  memory_pool_info(*pmem);

  UComOsMemFree(ptr);

  memory_pool_info(*pmem);

  dumpToFile("dump.txt", *pmem, 8, DUMP_HEX);


  /* test pool 2 */
  UComOsMemCreatePool(pmem2, test2, 10240);

  UComOsMemAlloc(*pmem2, 200, (void **)&(ptr2));

  memory_pool_info(*pmem2);

  UComOsMemFree(ptr2);

  UComOsMemAlloc(*pmem2, 100, (void **)&(ptr2));

  memory_pool_info(*pmem2);

  UComOsMemFree(ptr2);

  memory_pool_info(*pmem2);

  dumpToFile("dump2.txt", *pmem2, 8, DUMP_HEX);

  UComOsMemDeletePool(*pmem);

  UComOsMemDeletePool(*pmem2);

  return 0;
}



