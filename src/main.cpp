// MemoryPool.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include "StandardMemoryPool.hpp"
#include <string.h>
#include "memlog.hpp"
#include <string>

#define DUMP_FILE_NAME "pool.xml"

static void mem_pool_stress_test()
{
  int i = 0, block_num = 0;
  char str[] = "abcdefghijklnmopqrstuvwxyz";
  Chunk * block[40] = {};

  for(i = 0; i < 40; i++)
  {
    block[i] = NULL;
  }

  StandardMemoryPool *pool = new StandardMemoryPool(2048, 1);

  for(i = 0; i < 40; i ++)
  {
    mem_debug_log("-------------- Start to allocate memory blcok [%d].-----------------------", i+1);

    block[i] = (Chunk *)pool->allocate(64);

    if(!block[i])
    {
      mem_debug_log("%d block allocation failed.", i+1);
      block_num = i - 1;
      break;
    }
    else
    {
      mem_debug_log("%d block allocation success.", i+1);
    }

    memcpy((uint8 *)block[i], str, strlen(str));
  }

#ifdef DEBUG_ON
//  pool->dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_CHAR);
  pool->memory_block_list();
#endif


  if(pool->integrityCheck() == TRUE)
  {
    mem_debug_log("Integrity check successful");
  }
  else
  {
    mem_debug_log("Integrity check fail");
  }
  pool->dumpToFile(DUMP_FILE_NAME, DUMP_ELEMENT_PER_LINE, DUMP_HEX);


  for(i = block_num; i >= 0 ; i --)
  {
    mem_debug_log("-------------- Start to free memory blcok [%d].-----------------------", i+1);

    if(pool->free(block[i]) == FALSE)
    {
      mem_debug_log("%d block free failed.", i+1);
      break;
    }
    else
    {
      mem_debug_log("%d block free success.", i+1);
    }
  }

#ifdef DEBUG_ON
  pool->dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_CHAR);
#endif
}

// Entry point
int main()
{
  mem_pool_stress_test();

  return 0;
}

