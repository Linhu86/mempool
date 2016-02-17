#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "ucom_rtos_mem.h"


/* 
    memory pool structure:

    memorypoolheader
       |
      \ /
    memorypool 1 -> memorychunk 1 -> memorychunk 2 -> memorychunk 3 ...... -> memorychunk n -> NULL
       |
      \ /
    memorypool 2 -> memorychunk 1 -> memorychunk 2 -> memorychunk 3 ...... -> memorychunk n -> NULL
       |
      \ /   
    memorypool 3 -> memorychunk 1 -> memorychunk 2 -> memorychunk 3 ...... -> memorychunk n -> NULL
       |
      \ /
       .
       .
       .
    memorypool n -> memorychunk 1 -> memorychunk 2 -> memorychunk 3 ...... -> memorychunk n -> NULL
       |
      \ /
      NULL
*/

#define ucom_log printf

static const UComUInt8 boundsCheckSize = 16;

static const UComUInt8 s_trashOnCreation = 0xCC;
static const UComUInt8 s_trashOnAllocSignature = 0xAB;
static const UComUInt8 s_trashOnFreeSignature  = 0xFE;


static const UComUInt8 s_startBound[16] = {'[','B','l','o','c','k','.','.','.','.','S','t','a','r','t',']'};
static const UComUInt8 s_endBound[16]   = {'[','B','l','o','c','k','.','.','.','.','.','.','E','n','d',']'};

static MemoryPool_t *p_mempoolHeader = NULL;


UComInt32 UComOsMemCreatePool(MemoryPool_t ** mem, void* mem_area, UComUInt32 size)
{
  UComInt32 res;
  MemParam_t params;

  params.poolBase = (UComUInt8 *)mem_area;
  params.totalPoolSize=(UComUInt32)(1024 + MEMORY_POOL_HEADER_SIZE + MEMORY_CHUNK_HEADER_SIZE);
  params.name="ucomPool";
  params.boundsCheck = 0;
  params.trashOnCreation = 1;
  params.trashOnAlloc = 1;
  params.trashOnFree = 1;

  res = MemoryPoolCreate(mem, &params);

  return res;
}


/* Create a new memory pool. */

UComInt32 MemoryPoolCreate(MemoryPool_t **mem, MemParam_t *params)
{
  UComUInt32 totalPoolSize = params->totalPoolSize;
  UComUInt8 boundsCheck = params->boundsCheck;
  UComUInt8 trashOnCreation = params->trashOnCreation;
  UComUInt32 freePoolSize = totalPoolSize - MEMORY_POOL_HEADER_SIZE - MEMORY_CHUNK_HEADER_SIZE;
  UComUInt8 *poolBase = params->poolBase;
  UComUInt8 *poolBase_offset = poolBase + MEMORY_POOL_HEADER_SIZE;

  MemoryPool_t new_mem_pool;
  Chunk_t first_chunk;

  if(NULL == mem || NULL == params)
  {
    ucom_log("Invalide parameters.\n");
    return UCOM_FAILURE;
  }

  /* write pool header into the memory buffer. */
  memcpy(poolBase, &new_mem_pool, sizeof(new_mem_pool));
  
  MemoryPool_t *p_mem_pool = (MemoryPool_t *)poolBase;
  
  //Log
  ucom_log("StandardPool Constructor initialization in address %p with size %u, MEMORY_POOL_HEADER_SIZE: %lu, MEMORY_CHUNK_HEADER_SIZE: %lu consumed.\n", poolBase, freePoolSize, MEMORY_POOL_HEADER_SIZE, MEMORY_CHUNK_HEADER_SIZE);
  //ucom_log("StandardPool created with m_trashOnCreation:%d m_trashOnAlloc: %d  m_trashOnFree :%d m_boundsCheck %d", m_trashOnCreation, m_trashOnAlloc, m_trashOnFree, m_boundsCheck);

  // Trash it if required
  if(trashOnCreation)
  {
    memset(poolBase, s_trashOnCreation, totalPoolSize);
  }

  first_chunk.name = DEFAULT_CHUNK_NAME;
  first_chunk.next = NULL;
  first_chunk.prev = NULL;
  first_chunk.free = 1;
  first_chunk.pool = p_mem_pool;

  if(boundsCheck)
  {
    freePoolSize -= boundsCheckSize * 2;
    first_chunk.userdataSize= freePoolSize - 2 * boundsCheckSize;
    memcpy(poolBase_offset + boundsCheckSize, &first_chunk, MEMORY_CHUNK_HEADER_SIZE);
    memcpy(poolBase_offset, s_startBound, boundsCheckSize);
    memcpy(poolBase_offset + freePoolSize - boundsCheckSize, s_endBound, boundsCheckSize);
    p_mem_pool->header = (Chunk_t *)poolBase_offset + boundsCheckSize;
  }
  else
  {
    first_chunk.userdataSize = freePoolSize;
    memcpy(poolBase_offset, &first_chunk, sizeof(first_chunk));
    p_mem_pool->header = (Chunk_t *)poolBase_offset;
  }

  p_mem_pool->memParam.poolBase = poolBase;
  p_mem_pool->memParam.name = params->name;
  p_mem_pool->memParam.trashOnAlloc = params->trashOnAlloc;
  p_mem_pool->memParam.trashOnFree = params->trashOnFree;
  p_mem_pool->memParam.freePoolSize = freePoolSize;
  p_mem_pool->memParam.totalPoolSize = totalPoolSize;
  p_mem_pool->memParam.trashOnCreation = trashOnCreation;
  p_mem_pool->memParam.boundsCheck = boundsCheck;

  /* Insert new created poolinto mempool list.*/
  if(NULL == p_mempoolHeader)
  {
    p_mem_pool->next = NULL;
    p_mem_pool->prev = NULL;
    p_mempoolHeader = p_mem_pool;
  }
  else if(NULL == p_mempoolHeader->next)
  {
    p_mempoolHeader->next = p_mem_pool;
    p_mem_pool->prev = NULL;
  }
  else
  {
    p_mem_pool->next = p_mempoolHeader->next;
    p_mem_pool->prev = NULL;
    p_mempoolHeader->next->prev = p_mem_pool;
    p_mempoolHeader->next = p_mem_pool;
  }

  *mem = p_mem_pool;

/*
#ifdef MEM_DEBUG_ON
  dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_HEX);
#endif
*/
}

UComInt32 UComOsMemDeletePool(MemoryPool_t *mem)
{
  MemoryPoolDestroy(mem);
}

UComInt32 MemoryPoolDestroy(MemoryPool_t *mem)
{
  /* Remove the memory pool from list */
  MemoryPool_t *ptr = p_mempoolHeader;
  UComInt32 ret = UCOM_FAILURE;

  if(NULL == mem)
  {
    ucom_log("Invalid parameters.\n");
    return UCOM_FAILURE;
  }

  if(NULL == ptr)
  {
    return UCOM_FAILURE;
  }

  while(ptr)
  {
    if(ptr == mem)
    {
      if(ptr->prev == NULL)
      {
        p_mempoolHeader = ptr->next;
      }
      else if(ptr->next == NULL)
      {
        ptr->prev->next = NULL;
        ptr->prev = NULL;
      }
      else{
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
        ptr->prev = NULL;
        ptr->next = NULL;
      }

      ucom_log("find pool %p to delete.\n", mem);
      memset(mem, '\0', mem->memParam.totalPoolSize);
      mem = NULL;
      ret = UCOM_SUCCESS;
      break;
    }
    ptr = ptr->next;
  }  


  return UCOM_SUCCESS;
}


typedef struct MemBlock
{
  MemoryPool_t *mem;
  UComUInt8 * mem_addr;
} MemBlock_t;

UComInt32 UComOsMemAlloc(MemoryPool_t *mem, UComUInt32 size, void **ptr)
{
  *ptr = MemoryPoolAllocate(mem, size);

  return UCOM_SUCCESS;
}

void *MemoryPoolAllocate(MemoryPool_t *mem, UComUInt32 size)
{
  ucom_log("Start to allocate block with size.\n");

/*
#ifdef MEM_DEBUG_ON
  memory_block_list();
#endif
*/

  if(NULL == mem)
  {
    ucom_log("Invalid parameters.\n");
    return UCOM_FAILURE;
  }

  if(size > MAX_MEMPOOL_SIZE || size == 0)
  {
    ucom_log("Wrong memory pool size.\n");
    return NULL;
  }

  UComUInt32 requiredSize = size + MEMORY_CHUNK_HEADER_SIZE;

  if(mem->memParam.boundsCheck)
  {
    requiredSize += boundsCheckSize *2;
  }

  Chunk_t *block = mem->header;

  while(block)
  {
     if(block->free == 1 && block->userdataSize >= requiredSize)
     {
        break;
     }
     block = block->next;
  }

  // If block is found, return NULL
  if(!block)
  {
    ucom_log("Free block not found.\n");
    return NULL;
  }
  else
  {
    ucom_log("First free block address: %p.\n", block);
  }

  UComUInt8 * blockData = (UComUInt8 *)block;

  // If the block is valid, create a new free block with what remains of the block memory
  UComUInt32 freeUserDataSize = block->userdataSize - requiredSize;

  //Log
  ucom_log("User required allocate size: %u, block remain size:%u, sizeof(Chunk): %lu\n", requiredSize, freeUserDataSize, MEMORY_CHUNK_HEADER_SIZE);

  if(freeUserDataSize > BLOCK_SIZE_MIN)
  {
    Chunk_t freeBlock;
    freeBlock.userdataSize= freeUserDataSize;
    freeBlock.pool = mem;
    freeBlock.free = 1;
    freeBlock.next = block->next;
    freeBlock.prev = block;

    //Log
    ucom_log("New header saved in address: %p\n", blockData + requiredSize);

    if(freeBlock.next)
    {
      freeBlock.next->prev = (Chunk_t *)(blockData + requiredSize);
    }

    memcpy(blockData + requiredSize, &freeBlock, sizeof(freeBlock));

    if(mem->memParam.boundsCheck)
    {
       memcpy(blockData + requiredSize - boundsCheckSize, s_startBound, boundsCheckSize);
    }

    block->next = (Chunk_t *)(blockData + requiredSize);
    block->userdataSize = size;
  }

  // If a block is found, update the pool size
  mem->memParam.freePoolSize -= block->userdataSize;

  // Set the memory block
  block->free = 0;

  // Move the memory around if guards are needed
  if(mem->memParam.boundsCheck)
  {
    memcpy(blockData - boundsCheckSize, s_startBound, boundsCheckSize);
    memcpy(blockData + MEMORY_CHUNK_HEADER_SIZE + block->userdataSize, s_endBound, boundsCheckSize);
  }

  //Trash on alloc if required
  if(mem->memParam.trashOnAlloc)
  {
    memset(blockData + MEMORY_CHUNK_HEADER_SIZE, s_trashOnAllocSignature, block->userdataSize);
  }

/*
#ifdef MEM_DEBUG_ON
//memory_block_list();
//dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_HEX);
#endif
*/
  //Log
  ucom_log("Retrun allocated block address: %p\n", blockData + MEMORY_CHUNK_HEADER_SIZE);
  ucom_log("Type: Standard Memory\n");
  ucom_log("Total Size: %u\n", mem->memParam.totalPoolSize);
  ucom_log("Free Size: %u\n", mem->memParam.freePoolSize);

  return (blockData + MEMORY_CHUNK_HEADER_SIZE);
}


UComInt32 UComOsMemFree(void *ptr)
{
  MemoryPoolFree(ptr);
}

int MemoryPoolFree(void *ptr)
{
/*
#ifdef MEM_DEBUG_ON
//    memory_block_list();
#endif
*/

  // is a valid node?
  if(!ptr)
  {
    ucom_log("Block pointer to free is not valid.\n");
    return UCOM_FAILURE;
  }

  Chunk_t *block = (Chunk_t *)((UComUInt8 *)ptr - MEMORY_CHUNK_HEADER_SIZE);

  MemoryPool_t *mem = block->pool;

  if(block->free  || block == NULL)
  {
    ucom_log("Block is already freed.\n");
    return UCOM_FAILURE;
  }

  UComUInt32 fullBlockSize = block->userdataSize + MEMORY_CHUNK_HEADER_SIZE + (mem->memParam.boundsCheck == 1 ? boundsCheckSize * 2 : 0);

  mem->memParam.freePoolSize += block->userdataSize;

  Chunk_t* headBlock = block;
  Chunk_t* prev = block->prev;
  Chunk_t* next = block->next;

  // If the node before is free merge it with this one
  if(block->prev && block->prev->free)
  {
    headBlock = block->prev;
    prev = block->prev->prev;
    next = block->next;

    // Include the prev node in the block size so we trash it as well
    fullBlockSize += mem->memParam.boundsCheck == 1 ? block->prev->userdataSize + MEMORY_CHUNK_HEADER_SIZE + boundsCheckSize * 2 : block->prev->userdataSize + MEMORY_CHUNK_HEADER_SIZE;

    // If there is a next one, we need to update its pointer
    if(block->next)
    {
      block->next->prev = headBlock;

      if(block->next->free)
      {
         next = block->next->next;

         if(block->next->next)
         {
           block->next->next->prev = headBlock;
         }

         fullBlockSize += mem->memParam.boundsCheck == 1 ? block->next->userdataSize + MEMORY_CHUNK_HEADER_SIZE + boundsCheckSize * 2 : block->next->userdataSize + MEMORY_CHUNK_HEADER_SIZE;
      }
    }
  }
  else if(block->next && block->next->free)
  {
    headBlock = block;
    prev = block->prev;
    next = block->next->next;

    if(block->next->next)
    {
      block->next->next->prev = headBlock;
    }

    fullBlockSize += mem->memParam.boundsCheck == 1 ? block->next->userdataSize + MEMORY_CHUNK_HEADER_SIZE + boundsCheckSize * 2 : block->next->userdataSize + MEMORY_CHUNK_HEADER_SIZE;
  }

  UComUInt8 * freeBlockStart = (UComUInt8 *)headBlock;

  if(mem->memParam.trashOnFree)
  {
    memset(mem->memParam.boundsCheck == 1 ? freeBlockStart - boundsCheckSize : freeBlockStart, s_trashOnFreeSignature, fullBlockSize);
  }

  UComUInt32 freeUserDataSize = fullBlockSize - MEMORY_CHUNK_HEADER_SIZE;

  freeUserDataSize = (mem->memParam.boundsCheck == 1) ? freeUserDataSize - boundsCheckSize * 2 : freeUserDataSize;

  Chunk_t freeBlock;
  freeBlock.userdataSize= freeUserDataSize;
  freeBlock.pool = mem;
  freeBlock.prev = prev;
  freeBlock.next = next;
  freeBlock.free = 1;
  memcpy(freeBlockStart, &freeBlock, sizeof(freeBlock));

  // Move the memory around if guards are needed
  if(mem->memParam.boundsCheck)
  {
    memcpy(freeBlockStart - boundsCheckSize, s_startBound, boundsCheckSize);
    memcpy(freeBlockStart + MEMORY_CHUNK_HEADER_SIZE + freeUserDataSize, s_endBound, boundsCheckSize);
  }

#ifdef MEM_DEBUG_ON
//  memory_block_list();
#endif

  return UCOM_SUCCESS;
}

/**
*	\brief		Make an integrity test whether the bounds check flag is true
*/
int integrityCheck(MemoryPool_t *mem)
{
  if(NULL == mem)
  {
    ucom_log("Invalid parameters.\n");
    return UCOM_FAILURE;
  }

  if(mem->memParam.boundsCheck == 1)
  {
    Chunk_t* temp = (Chunk_t *)(mem->memParam.poolBase + boundsCheckSize);

    while(temp != NULL)
    {
      if(memcmp(((UComUInt8 *)temp) - boundsCheckSize, s_startBound, boundsCheckSize) != 0)
      {
        return UCOM_FAILURE;
      }

      if(memcmp(((UComUInt8 *)temp) + MEMORY_CHUNK_HEADER_SIZE + temp->userdataSize, s_endBound, boundsCheckSize) != 0)
      {
        return UCOM_FAILURE;
      }

      temp = temp->next;
    }
  }
  return UCOM_SUCCESS;
}


/**
*	\brief		Dump the memory state to file
*/

void dumpToFile(const UComChar *fileName, MemoryPool_t *mem, const UComUInt32 itemsPerLine, const UComUInt32 format)
{
  FILE* f = NULL;
  UComUInt32 i = 0, j = 0;
  UComUInt8 *ptr = mem->memParam.poolBase;
  UComUInt32 residue = mem->memParam.totalPoolSize%itemsPerLine;

  f = fopen(fileName, "w+");

  if(!f)
  {
    ucom_log("File open error: %s", strerror(errno));
    return;
  }

  fprintf(f, "Memory pool ------------------------------------------------------------------------------------------------\n\n");
  fprintf(f, "Type: Standard Memory\n");
  fprintf(f, "Total Size: %u\n", mem->memParam.totalPoolSize);
  fprintf(f, "Free Size: %u\n", mem->memParam.freePoolSize);

  for(i = 0; i < mem->memParam.totalPoolSize/itemsPerLine; i ++)
  {
    for(j = 0; j < itemsPerLine; j++)
    {
      if(format == DUMP_HEX)
      {
        fprintf(f, "[Address: %p] : 0x%x ",ptr, *ptr);
      }
      else if(format == DUMP_CHAR)
      {
        fprintf(f, "[Address: %p] : %c ",ptr, *ptr);
      }
      else
      {
        ucom_log("Error dump format.\n");
        return;
      }

      ptr ++;
    }
    fprintf(f, "\n");
  }
  
  if(residue)
  {
    for(i = 0; i < residue; i++)
    {
      fprintf(f, "%c", *ptr);
      ptr ++;
    }
    fprintf(f, "\n");
  }

  fprintf(f, "\n\nMemory pool ------------------------------------------------------------------------------------------------\n");

  ucom_log("Successful to dump memory pool.");

  fclose(f);
}

/*
//format could be hex or char.
void StandardMemoryPool :: dumpToStdOut(uint32 ElemInLine, const uint32 format) const
{
  uint32 i = 0, j = 0;
  uint32 residue = 0;
  uint8 *ptr = m_poolMemory;

  printf("\n\n");

  mem_debug_log("\n\nStart to dump memory pool.");
  mem_debug_log("Type: Standard Memory");
  mem_debug_log("Total Size: %lu", m_totalPoolSize);
  mem_debug_log("Free Size: %lu", m_freePoolSize);

  printf("\n\nMemory pool ------------------------------------------------------------------------------------------------\n\n");

  residue = m_poolSize%ElemInLine;

  for(i = 0; i < m_poolSize/ElemInLine; i ++)
  {
    for(j = 0; j < ElemInLine; j++)
    {
      if(format == DUMP_HEX)
      {
        printf("[Address: %p] : 0x%x",ptr, *ptr);
      }
      else if(format == DUMP_CHAR)
      {
        printf("[Address: %p] : %c ",ptr, *ptr);
      }
      else
      {
        mem_error_log("Error dump format.\n");
        return;
      }
      ptr ++;
    }
    printf("\n");
  }

  if(residue)
  {
    for(i = 0; i < residue; i++)
    {
      printf("%c", *ptr);
      ptr ++;
    }
    printf("\n");
  }

  printf("\n\nMemory pool ------------------------------------------------------------------------------------------------\n");
  mem_debug_log("Finish to dump memory pool.");

  printf("\n\n");
}


void StandardMemoryPool :: memory_block_list() const
{
  uint32 i = 1;
  Chunk *block = (Chunk *)(m_boundsCheck == 1 ? m_poolMemory + boundsCheckSize : m_poolMemory);

  if(block == NULL)
  {
    mem_error_log("block list is NULL.");
    return;
  }

  mem_debug_log("Start to print the memory block list. Memory total size: %lu, Free memory size: %lu. \n\n\n", m_totalPoolSize, m_freePoolSize);

  while(block)
  {
    printf("[block: %d] address: %p name: %s free: %d size:%d-->\n", i, block, block->m_name, block->m_free, block->m_userdataSize);
    block = block->m_next;
    i++;
  }

  printf("[last block] NULL\n\n\n");
}
*/


void dump_memory_block_list(const UComChar* fileName, MemoryPool_t *mem)
{
  FILE* f = NULL;
  UComInt32 i = 1;
  Chunk_t *block = (Chunk_t *)(mem->memParam.boundsCheck == 1 ? mem->memParam.poolBase + MEMORY_POOL_HEADER_SIZE + boundsCheckSize : mem->memParam.poolBase + MEMORY_POOL_HEADER_SIZE);

  if(block == NULL)
  {
    ucom_log("block list is NULL.");
    return;
  }

  ucom_log("Start to print the memory block list. Memory total size: %u, Free memory size: %u. \n\n\n", mem->memParam.totalPoolSize, mem->memParam.freePoolSize);

  f = fopen(fileName, "w+");

  if(!f)
  {
    ucom_log("File open error: %s", strerror(errno));
    return;
  }

  fprintf(f, "Memory block list ------------------------------------------------------------------------------------------------\n\n");

  while(block)
  {
    fprintf(f, "[block: %d] address: %p name: %s free: %d size:%d-->\n", i, block, block->name, block->free, block->userdataSize);
    block = block->next;
    i++;
  }

  fprintf(f, "[last block] NULL\n\n\n");

  fclose(f);
}



void memory_pool_info(MemoryPool_t *mem)
{
  ucom_log("Start to log memory pool information.\n");

  ucom_log("\n\nMemory Pool information:\n");

  ucom_log("Address: %p\n", mem);

  ucom_log("Total space: %u\n", mem->memParam.totalPoolSize);

  ucom_log("Free space: %u\n", mem->memParam.freePoolSize);

  ucom_log("boundsCheck: %d\n", mem->memParam.boundsCheck);

  ucom_log("Trash on allocate: %d\n", mem->memParam.trashOnAlloc);

  ucom_log("Trash on creation: %d\n", mem->memParam.trashOnCreation);

  ucom_log("Trash on free: %d\n", mem->memParam.trashOnFree);
}


