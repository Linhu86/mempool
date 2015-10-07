// MemoryPool.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include "Allocation.hpp"
#include "StandardMemoryPool.hpp"
#include <string.h>
#include "memlog.hpp"
#include <string>

#define DUMP_FILE_NAME "pool.xml"

static void mem_pool_stress_test()
{
  int i = 0;
  char str[] = "abcdefghijklnmopqrstuvwxyz";
  Chunk * block[40] = {};

  for(i = 0; i < 40; i++)
  {
    block[i] = NULL;
  }

  StandardMemoryPool *pool = new StandardMemoryPool(2048, 1);

  for(i = 0; i < 40; i ++)
  {
    block[i] = (Chunk *)pool->allocate(64);

    if(!block[i])
    {
      mem_debug_log("%d block allocation failed.", i);
      break;
    }
    else
    {
      mem_debug_log("%d block allocation success.", i);
    }

    memcpy((uint8 *)block, str, strlen(str));
  }

#ifdef DEBUG_ON
  pool->dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_CHAR);
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

  for(i = 0; i < 40; i ++)
  {
    block[i] = (Chunk *)pool->allocate(64);

    if(pool->free(block) == FALSE)
    {
      mem_debug_log("%d block free failed.", i);
      break;
    }
    else
    {
      mem_debug_log("%d block free success.", i);
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

