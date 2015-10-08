// MemoryPool.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#include "StandardMemoryPool.hpp"
#include "memlog.hpp"

#define DUMP_FILE_NAME "pool.xml"
#define TEST_BLOCK_NUM 40
#define TEST_POOL_VOL 2048
#define TEST_BLOCK_SIZE 64

static Chunk *block[TEST_BLOCK_NUM] = {};
static StandardMemoryPool *pool = new StandardMemoryPool(TEST_POOL_VOL, 1);


static void print_block_array(Chunk *block_array[], uint32 length)
{
  uint32 i = 0;

  mem_debug_log("\nStart to print all block array.");

  if(!block_array)
  {
    mem_debug_log("data array is NULL.");
    return;
  }

  if(length <= 0)
  {
    mem_debug_log("data array length is NULL.");
    return;
  }

  mem_debug_log("print data array:");

  for(i = 0; i < length; i++)
  {
    printf("[element %d]: %p\n", i+1, block_array[i]);
  }

  printf("End to print all block array.\n\n\n");
}

static int get_array_avail_length(Chunk *block_array[], uint32 array_length)
{
  uint32 i = 0;
  int ret_val = 0;

  if(!block_array)
  {
    mem_debug_log("block array is NULL.");
    return -1;
  }

  for(i = 0; i < array_length; i++)
  {
    if(block_array[i] != NULL)
    {
      ret_val ++;
    }
  }

  mem_debug_log("data array length : %d", ret_val);

  return ret_val;
}

static Chunk* get_random_block(Chunk *block_array[], uint32 block_size, uint32 *ret_block_idx)
{
  uint32 i = 0;
  uint32 idx = 0;
  uint32 length = 0;
  uint32 rand = 0;
  Chunk *ret_block = NULL;

  if(!block_array)
  {
    mem_debug_log("data array is NULL.");
    return NULL;
  }

  length = get_array_avail_length(block_array, block_size);

  if(!length)
  {
    mem_debug_log("data array not available.");
    return NULL;
  }

  srand((unsigned uint32)time(0));

  rand = random();

  if(length > 1)
  {
    idx = (rand%(length-1));

    mem_debug_log("rand: %d idx: %d",rand, idx);

    ret_block = block_array[idx];
    *ret_block_idx = idx;

    for(i = idx; i < length - 1; i++)
    {
      block_array[i] = block_array[i+1];
    }
  }
  else if(length == 1)
  {
    ret_block = block_array[0];
    *ret_block_idx = 1;
  }

  mem_debug_log("Get a random block %p from block array.", ret_block);
  block_array[i] = NULL;
  return ret_block;
}

static void mem_pool_stress_test_init()
{
  uint32 i = 0;
  for(i = 0; i < TEST_BLOCK_NUM; i++)
  {
    block[i] = NULL;
  }

#ifdef DEBUG_ON
  print_block_array(block, TEST_BLOCK_NUM);
#endif
}

static uint32 mem_pool_stress_test_allocate()
{
  uint32 i = 0, block_num = 0;
  char str[] = "abcdefghijklnmopqrstuvwxyz";

  printf("\n\n\n\n");
  mem_debug_log("=================================================================================");
  mem_debug_log("==========================Start to test allocate=================================");
  mem_debug_log("=================================================================================");

  for(i = 0; i < TEST_BLOCK_NUM; i ++)
  {
    mem_debug_log("-------------- Start to allocate memory blcok [%d].-----------------------", i+1);

    block[i] = (Chunk *)pool->allocate(TEST_BLOCK_SIZE);

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

  mem_debug_log("=================================================================================");
  mem_debug_log("==========================End to test mem allocate===============================");
  mem_debug_log("=================================================================================");
  printf("\n\n\n\n");
  return block_num;
}

static void mem_pool_stress_test_check()
{
#ifdef DEBUG_ON
  //  pool->dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_CHAR);
  pool->memory_block_list();
  print_block_array(block, TEST_BLOCK_NUM);
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
}


static void mem_pool_stress_test_free(uint32 block_num)
{
  uint32 i = 0, block_free_idx = 0;
  Chunk *block_free = NULL;

  printf("\n\n\n\n");
  mem_debug_log("=================================================================================");
  mem_debug_log("==========================Start to test free ====================================");
  mem_debug_log("=================================================================================");

  for(i = block_num; i >= 0 ; i --)
  {
    //Get a random block pointer from block array
    block_free = get_random_block(block, TEST_BLOCK_NUM, &block_free_idx);

    mem_debug_log("-------------- Start to free memory blcok [%d].-----------------------", block_free_idx);

#ifdef DEBUG_ON
    mem_debug_log("Get a ramdom blcok: %p\n", block_free);
    print_block_array(block, TEST_BLOCK_NUM);
#endif

    if(pool->free(block_free) == FALSE)
    {
      mem_debug_log("%d block free failed.", block_free_idx);
      break;
    }
    else
    {
      mem_debug_log("%d block free success.", block_free_idx);
    }
  }

  printf("\n\n\n\n");
  mem_debug_log("=================================================================================");
  mem_debug_log("==========================End to test free ======================================");
  mem_debug_log("=================================================================================");

}

static void mem_pool_stress_test_deinit()
{
#ifdef DEBUG_ON
  pool->dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_CHAR);
#endif

  delete pool;
}

static void mem_pool_stress_test()
{
  uint32 block_num = 0;

  mem_pool_stress_test_init();

  block_num = mem_pool_stress_test_allocate();

  mem_pool_stress_test_check();

  mem_pool_stress_test_free(block_num);

  mem_pool_stress_test_deinit();
}

// Entry point
int main()
{
  mem_pool_stress_test();

  return 0;
}












