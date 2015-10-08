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
#define TEST_MALLOC_TIMES 11915
#define TEST_BLOCK_NUM 40  *1024*4 // Could allocate 11914 blocks
#define TEST_POOL_VOL 1024 *1024
#define TEST_BLOCK_SIZE 16

static Chunk *block[TEST_BLOCK_NUM] = {};
static StandardMemoryPool *pool = new StandardMemoryPool(TEST_POOL_VOL, 1);

static char *malloc_block[TEST_MALLOC_TIMES] = {};

#ifdef TEST_DEBUG_ON
#define test_debug_log(fmt, ...) printf("[%s]"#fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define test_debug_log(fmt, ...) \
{                               \
}
#endif

#ifdef TEST_ERROR_MSG_ON
#define test_error_log(fmt, ...) printf("[%s]"#fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define test_error_log(fmt, ...) \
{                               \
}
#endif


static void print_block_array(Chunk *block_array[], uint32 length)
{
  uint32 i = 0;

  test_debug_log("Start to print all block array.");

  if(!block_array)
  {
    test_error_log("data array is NULL.");
    return;
  }

  if(length <= 0)
  {
    test_error_log("data array length is NULL.");
    return;
  }

  test_debug_log("print data array:");

  printf("\n\nBlock table: ------------------------\n\n");

  for(i = 0; i < length; i++)
  {
    printf("[element %d]: %p\n", i+1, block_array[i]);
  }

  printf("\n\nBlock table: ------------------------\n\n");

  test_debug_log("End to print all block array.");

}

static int get_array_avail_length(Chunk *block_array[], uint32 array_length)
{
  uint32 i = 0;
  int ret_val = 0;

  if(!block_array)
  {
    test_error_log("block array is NULL.");
    return -1;
  }

  for(i = 0; i < array_length; i++)
  {
    if(block_array[i] != NULL)
    {
      ret_val ++;
    }
  }

  test_debug_log("data array length : %d", ret_val);

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
    test_error_log("data array is NULL.");
    return NULL;
  }

  length = get_array_avail_length(block_array, block_size);

  if(!length)
  {
    test_error_log("data array not available.");
    return NULL;
  }

  srand((unsigned uint32)time(0));

  rand = random();

  if(length > 1)
  {
    idx = (rand%(length-1));

    //Log
    test_debug_log("rand: %d idx: %d",rand, idx);

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

  test_debug_log("Get a random block %p from block array.", ret_block);

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

#ifdef TEST_DEBUG_ON
  print_block_array(block, TEST_BLOCK_NUM);
#endif
}

static uint32 mem_pool_stress_test_allocate()
{
  uint32 i = 0, block_num = 0;
  char str[] = "abcdefghijklnmopqrstuvwxyz";

#ifdef TEST_DEBUG_ON
  printf("\n\n\n\n");
#endif

  //Log
  test_debug_log("=================================================================================");
  test_debug_log("==========================Start to test allocate=================================");
  test_debug_log("=================================================================================");

#ifdef TEST_DEBUG_ON
  printf("\n\n");
#endif

  for(i = 0; i < TEST_BLOCK_NUM; i ++)
  {
    test_debug_log("-------------- Start to allocate memory blcok [%d].-----------------------", i+1);

    block[i] = (Chunk *)pool->allocate(TEST_BLOCK_SIZE);

    if(!block[i])
    {
      test_error_log("%d block allocation failed.", i+1);
      block_num = i - 1;
      break;
    }
    else
    {
      test_debug_log("%d block allocation success.", i+1);

#ifdef TEST_DEBUG_ON
      printf("----------------------------------------------------------------------------------------------------------\n\n");
#endif

    }

    memcpy((uint8 *)block[i], str, strlen(str));
  }

  //Log
  test_debug_log("=================================================================================");
  test_debug_log("==========================End to test mem allocate===============================");
  test_debug_log("=================================================================================");

#ifdef TEST_DEBUG_ON
  printf("\n\n\n\n");
#endif
 
  return block_num;
}

static void mem_pool_stress_test_check()
{
  //Log
  test_debug_log("=================================================================================");
  test_debug_log("==========================Start to mem integrity check===========================");
  test_debug_log("=================================================================================");

#ifdef TEST_DEBUG_ON
  printf("\n\n");
  //pool->dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_HEX);
  pool->memory_block_list();
  print_block_array(block, TEST_BLOCK_NUM);
#endif

  if(pool->integrityCheck() == TRUE)
  {
    test_error_log("Integrity check successful");
  }
  else
  {
    test_debug_log("Integrity check fail");
  }
  pool->dumpToFile(DUMP_FILE_NAME, DUMP_ELEMENT_PER_LINE, DUMP_HEX);
  //Log
  test_debug_log("=================================================================================");
  test_debug_log("==========================End to mem integrity check===========================");
  test_debug_log("=================================================================================");

#ifdef TEST_DEBUG_ON
  printf("\n\n\n\n");
#endif
}


static void mem_pool_stress_test_free_random(uint32 block_num)
{
  uint32 i = 0, block_free_idx = 0;
  Chunk *block_free = NULL;

#ifdef TEST_DEBUG_ON
  printf("\n\n\n\n");
#endif

  //Log
  test_debug_log("=================================================================================");
  test_debug_log("==========================Start to test free ====================================");
  test_debug_log("=================================================================================");

#ifdef TEST_DEBUG_ON
    printf("\n\n");
#endif

  for(i = block_num; i >= 0 ; i --)
  {
    //Get a random block pointer from block array
    block_free = get_random_block(block, TEST_BLOCK_NUM, &block_free_idx);

    //Log
    test_debug_log("-------------- Start to free memory blcok [%d].-----------------------", block_free_idx);

#ifdef TEST_DEBUG_ON
    print_block_array(block, TEST_BLOCK_NUM);
#endif

    if(pool->free(block_free) == FALSE)
    {
      test_error_log("%d block free failed.", block_free_idx);
      break;
    }
    else
    {
      test_debug_log("%d block free success.", block_free_idx);
#ifdef TEST_DEBUG_ON
      printf("--------------------------------------------------------------------------------------------------\n\n");
#endif
    }
  }

#ifdef TEST_DEBUG_ON
  printf("\n\n\n\n");
#endif
  test_debug_log("=================================================================================");
  test_debug_log("==========================End to test free ======================================");
  test_debug_log("=================================================================================");
}

static void mem_pool_stress_test_free_squence(uint32 block_num)
{
  uint32 i = 0, block_free_idx = 0;

#ifdef TEST_DEBUG_ON
  printf("\n\n\n\n");
#endif

  //Log
  test_debug_log("=================================================================================");
  test_debug_log("==========================Start to test free %d block ==========================", block_num+1);
  test_debug_log("=================================================================================");

#ifdef TEST_DEBUG_ON
    printf("\n\n");
#endif

  for(i = block_num+1; i > 0 ; i --)
  {
    //Log
    test_debug_log("-------------- Start to free memory blcok [%d].-----------------------", i);

#ifdef TEST_DEBUG_ON
    print_block_array(block, TEST_BLOCK_NUM);
#endif

    if(pool->free(block[i-1]) == FALSE)
    {
      test_error_log("%d block free failed.", i);
      break;
    }
    else
    {
      test_debug_log("%d block free success.", i);
      block[i] = NULL;
#ifdef TEST_DEBUG_ON
      printf("--------------------------------------------------------------------------------------------------\n\n");
#endif
    }
  }

#ifdef TEST_DEBUG_ON
  printf("\n\n\n\n");
#endif
  test_debug_log("=================================================================================");
  test_debug_log("==========================End to test free ======================================");
  test_debug_log("=================================================================================");
}



static void mem_pool_stress_test_deinit()
{
#ifdef TEST_DEBUG_ON
  pool->dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_HEX);
#endif

  delete pool;
}

static void mem_pool_stress_test()
{
  uint32 block_num = 0;
  clock_t time_start, time_stop;

  mem_pool_stress_test_init();

  time_start = clock();

  block_num = mem_pool_stress_test_allocate();

  mem_pool_stress_test_check();

 // mem_pool_stress_test_free_random(block_num);

  mem_pool_stress_test_free_squence(block_num);

  time_stop = clock();

  pool->dumpToFile(DUMP_FILE_NAME, DUMP_ELEMENT_PER_LINE, DUMP_HEX);

  mem_pool_stress_test_deinit();

  printf("Memory Pool test execution time: %f s\n", (double)(time_stop-time_start)/CLOCKS_PER_SEC);
}


static void malloc_stress_test_do()
{
  int i = 0;

  for(i = 0; i < TEST_MALLOC_TIMES; i++)
  {
    malloc_block[i] = (char *)malloc(TEST_BLOCK_SIZE*sizeof(char));
  }

  for(i = 0; i < TEST_MALLOC_TIMES; i++)
  {
    free(malloc_block[i]);
    malloc_block[i] = NULL;
  }
}

static void malloc_stress_test()
{
  clock_t time_start, time_stop;

  time_start = clock();

  malloc_stress_test_do();

  time_stop = clock();

  printf("Malloc test execution time: %f s\n", (double)(time_stop-time_start)/CLOCKS_PER_SEC);
}

// Entry point
int main()
{
  mem_pool_stress_test();

  malloc_stress_test();

  return 0;
}












