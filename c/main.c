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

  MemoryPool_t **mem;

  UComUInt32 *ptr = NULL;

  UCOM_OSMEM_ALLOC_POOL_MEMAREA_STATIC(test1, 1024);

  UComOsMemCreatePool(mem, test1, 1024);

  UComOsMemAlloc(*mem, 200, (void **)&(ptr));

  UComOsMemFree(ptr);

  memory_pool_info(*mem);

  dumpToFile("dump.txt", *mem, 8, DUMP_HEX);

  UComOsMemDeletePool(*mem);

  return 0;
}



