#include "StandardMemoryPool.hpp"
#include "MemoryPool.hpp"
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>

int Chunk :: name_set(const char *name)
{
  if(strlen(name) > BLOCK_NAME_LEN)
  {
    mem_debug_log("memory block name length larger than 16");
    return FALSE;
  }
  memset(m_name, '\0', BLOCK_NAME_LEN);
  strncpy(m_name, name, strlen(name));
#ifdef DEBUG_ON
  mem_debug_log("New set memory block name: %s", m_name);
#endif
  return TRUE;
}

StandardMemoryPool :: StandardMemoryPool(uint64 sizeInBytes, uint32 boundsCheck)
{
  m_poolSize = sizeInBytes;

  m_boundsCheck = boundsCheck;

  m_poolMemory = new uint8[sizeInBytes];

#ifdef DEBUG_ON
  mem_debug_log("StandardPool Constructor initialization in address %p with size %lu\n", m_poolMemory, sizeInBytes);
  printf("StandardPool created with m_trashOnCreation:%d m_trashOnAlloc: %d  m_trashOnFree :%d m_boundsCheck %d\n", m_trashOnCreation, m_trashOnAlloc, m_trashOnFree, m_boundsCheck);
  if(boundsCheck){
    mem_debug_log("Memory pool bounds check feature present.\n");
  }
#endif

  m_freePoolSize = sizeInBytes - sizeof(Chunk);
  m_totalPoolSize = sizeInBytes;

  // Trash it if required
  if(m_trashOnCreation)
  {
    memset(m_poolMemory, s_trashOnCreation, sizeInBytes);
  }

  if(m_boundsCheck)
  {
    m_freePoolSize -= s_boundsCheckSize * 2;
    Chunk freeChunk(sizeInBytes - sizeof(Chunk) - 2 * s_boundsCheckSize);
    freeChunk.name_set(MEMORY_POOL_BLOCK_NAME);
    freeChunk.write(m_poolMemory + s_boundsCheckSize);
    memcpy(m_poolMemory, s_startBound, s_boundsCheckSize);
    memcpy(m_poolMemory + sizeInBytes - s_boundsCheckSize, s_endBound, s_boundsCheckSize);
    freeChunk.m_next = NULL;
    freeChunk.m_prev = NULL;
  }
  else
  {
    Chunk freeChunk(sizeInBytes - sizeof(Chunk));
    freeChunk.name_set(MEMORY_POOL_BLOCK_NAME);
    freeChunk.write(m_poolMemory);
    freeChunk.m_next = NULL;
    freeChunk.m_prev = NULL;
  }

#ifdef DEBUG_ON
    dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_CHAR);
#endif
}

StandardMemoryPool :: ~StandardMemoryPool()
{
#ifdef DEBUG_ON
  mem_debug_log("[%s] StandardPool Deonstructor deconstruction.", __FUNCTION__);
#endif
  delete [] m_poolMemory;
}

void *StandardMemoryPool :: allocate(uint64 size)
{
#if 0
#ifdef DEBUG_ON
  mem_debug_log("Start to allocate block with size.");
  memory_block_list();
#endif
#endif

  if(size > MAX_MEMPOOL_SIZE)
  {
    mem_debug_log("Wrong memory pool size.\n");
    return NULL;
  }

  uint64 requiredSize = size + sizeof(Chunk);

  if(m_boundsCheck)
  {
    requiredSize += s_boundsCheckSize *2;
  }

  Chunk *block = (Chunk *)(m_boundsCheck == 1 ? m_poolMemory + s_boundsCheckSize : m_poolMemory);

  while(block)
  {
     if(block->m_free && block->m_userdataSize >= requiredSize)
     {
        break;
     }
     block = block->m_next;
  }

  uint8 * blockData = (uint8 *)block;

  // If block is found, return NULL
  if(!block)
  {
    mem_debug_log("Free block not found.");
    return NULL;
  }
#ifdef DEBUG_ON
  else
  {
    mem_debug_log("First free block address: %p.", blockData);
  }
#endif

  // If the block is valid, create a new free block with what remains of the block memory
  uint64 freeUserDataSize = block->m_userdataSize - requiredSize;
#ifdef DEBUG_ON
  mem_debug_log("User required allocate size: %lu, block remain size:%lu, sizeof(Chunk): %lu", requiredSize, freeUserDataSize, sizeof(Chunk));
#endif

  if( freeUserDataSize > s_minFreeBlockSize)
  {
    Chunk freeBlock(freeUserDataSize);
    freeBlock.m_next = block->m_next;
    freeBlock.m_prev = block;

#ifdef DEBUG_ON
    mem_debug_log("New header saved in address: %p", blockData + requiredSize);
#endif

    if(freeBlock.m_next)
    {
      freeBlock.m_next->m_prev = (Chunk*)(blockData + requiredSize);
    }

    freeBlock.write(blockData + requiredSize);

    if(m_boundsCheck)
    {
       memcpy(blockData + requiredSize - s_boundsCheckSize, s_startBound, s_boundsCheckSize);
    }

    block->m_next = (Chunk*)(blockData + requiredSize);
    block->m_userdataSize = size;
  }

  // If a block is found, update the pool size
  m_freePoolSize -= block->m_userdataSize;

  // Set the memory block
  block->m_free = false;

  // Move the memory around if guards are needed
  if(m_boundsCheck)
  {
    memcpy(blockData - s_boundsCheckSize, s_startBound, s_boundsCheckSize);
    memcpy(blockData + sizeof(Chunk) + block->m_userdataSize, s_endBound, s_boundsCheckSize);
  }

  //Trash on alloc if required
  if(m_trashOnAlloc)
  {
    memset(blockData + sizeof(Chunk), s_trashOnAllocSignature, block->m_userdataSize);
  }

#if 1
#ifdef DEBUG_ON
  memory_block_list();
//  dumpToStdOut(DUMP_ELEMENT_PER_LINE, DUMP_CHAR);
  mem_debug_log("Retrun allocated block address: %p", blockData + sizeof(Chunk));
#endif
#endif
  return (blockData + sizeof(Chunk));
}


int StandardMemoryPool :: free(void* ptr)
{
#ifdef DEBUG_ON
    memory_block_list();
#endif
    // is a valid node?
    if(!ptr)
    {
      mem_debug_log("Block pointer to free is not valid.");
      return FALSE;
    }

    Chunk* block = (Chunk*)((uint8 *)ptr - sizeof(Chunk));

    assert(block->m_free == false);

    if(block->m_free)
    {
      mem_debug_log("Block is already freed.");
      return FALSE;
    }

    uint32 fullBlockSize = block->m_userdataSize + sizeof(Chunk) + (m_boundsCheck == 1 ? s_boundsCheckSize * 2 : 0);

    m_freePoolSize += block->m_userdataSize;

    Chunk* headBlock = block;
    Chunk* prev = block->m_prev;
    Chunk* next = block->m_next;

    // If the node before is free I merge it with this one
    if(block->m_prev && block->m_prev->m_free)
    {
      headBlock = block->m_prev;
      prev = block->m_prev->m_prev;
      next = block->m_next;

      // Include the prev node in the block size so we trash it as well
      fullBlockSize += m_boundsCheck == 1 ? block->m_prev->m_userdataSize + sizeof(Chunk) + s_boundsCheckSize * 2 : block->m_prev->m_userdataSize + sizeof(Chunk);

      // If there is a next one, we need to update its pointer
      if(block->m_next)
      {
         // We will re point the next
         block->m_next->m_prev = headBlock;

         // Include the next node in the block size if it is free so we trash it as well
         if(block->m_next->m_free)
         {
            // We will point to next's next
            next = block->m_next->m_next;

            if(block->m_next->m_next)
            {
              block->m_next->m_next->m_prev = headBlock;
            }

            fullBlockSize += m_boundsCheck == 1 ? block->m_next->m_userdataSize + sizeof(Chunk) + s_boundsCheckSize * 2 : block->m_next->m_userdataSize + sizeof(Chunk);
          }
      }
  }
  else
    // If next node is free lets merge it to the current one
  if(block->m_next && block->m_next->m_free)
  {
    headBlock = block;
    prev = block->m_prev;
    next = block->m_next->m_next;

    if(block->m_next->m_next)
    {
      block->m_next->m_next->m_prev = headBlock;
    }

    // Include the next node in the block size so we trash it as well
    fullBlockSize += m_boundsCheck == 1 ? block->m_next->m_userdataSize + sizeof(Chunk) + s_boundsCheckSize * 2 : block->m_next->m_userdataSize + sizeof(Chunk);
  }

  // Create the free block
  uint8* freeBlockStart = (uint8*)headBlock;

  if(m_trashOnFree)
  {
    memset(m_boundsCheck == 1 ? freeBlockStart - s_boundsCheckSize : freeBlockStart, s_trashOnFreeSignature, fullBlockSize);
  }

  uint32 freeUserDataSize = fullBlockSize - sizeof(Chunk);

  freeUserDataSize = (m_boundsCheck == 1) ? freeUserDataSize - s_boundsCheckSize * 2 : freeUserDataSize;

  Chunk freeBlock(freeUserDataSize);
  freeBlock.m_prev = prev;
  freeBlock.m_next = next;
  freeBlock.write(freeBlockStart);

  // Move the memory around if guards are needed
  if(m_boundsCheck)
  {
    memcpy(freeBlockStart - s_boundsCheckSize, s_startBound, s_boundsCheckSize);
    memcpy(freeBlockStart + sizeof(Chunk) + freeUserDataSize, s_endBound, s_boundsCheckSize);
  }

#ifdef DEBUG_ON
  memory_block_list();
#endif

  return TRUE;
}

/**
*	\brief		Make an integrity test whether the bounds check flag is true
*/
int StandardMemoryPool :: integrityCheck() const
{
  if(m_boundsCheck == 1)
  {
    Chunk* temp = (Chunk*)(m_poolMemory + s_boundsCheckSize);

    while(temp != NULL)
    {
      if(memcmp(((uint8*)temp) - s_boundsCheckSize, s_startBound, s_boundsCheckSize) != 0)
      {
        return FALSE;
      }

      if(memcmp(((uint8*)temp) + sizeof(Chunk) + temp->m_userdataSize, s_endBound, s_boundsCheckSize) != 0)
      {
        return FALSE;
      }

      temp = temp->m_next;
    }
  }
  return TRUE;
}

/**
*	\brief		Dump the memory state to file
*/
void StandardMemoryPool :: dumpToFile(const std::string& fileName, const uint32 itemsPerLine, const uint32 format) const
{
  FILE* f = NULL;
  uint32 i = 0, j = 0;
  uint8 *ptr = m_poolMemory;
  uint32 residue = m_poolSize%itemsPerLine;

  f = fopen(fileName.c_str(), "w+");

  if(!f)
  {
    mem_debug_log("File open error: %s", strerror(errno));
    return;
  }

  fprintf(f, "Memory pool ----------------------------------\n\n");
  fprintf(f, "Type: Standard Memory\n");
  fprintf(f, "Total Size: %lu\n", m_totalPoolSize);
  fprintf(f, "Free Size: %lu\n", m_freePoolSize);

  for(i = 0; i < m_poolSize/itemsPerLine; i ++)
  {
    for(j = 0; j < itemsPerLine; j++)
    {
      if(format == DUMP_HEX)
      {
        fprintf(f, "[Address: %p] : %c",ptr, *ptr);
      }
      else if(format == DUMP_CHAR)
      {
        fprintf(f, "[Address: %p] : %c ",ptr, *ptr);
      }
      else
      {
        mem_debug_log("Error dump format.\n");
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

  fprintf(f, "\n\nMemory pool ----------------------------------\n");
  printf("\n Successful to dump memory pool.\n");

  fclose(f);
}

//format could be hex or char.
void StandardMemoryPool :: dumpToStdOut(uint32 ElemInLine, const uint32 format) const
{
  uint32 i = 0, j = 0;
  uint32 residue = 0;
  uint8 *ptr = m_poolMemory;
  printf("\n\n Start to dump memory pool. \n");

  residue = m_poolSize%ElemInLine;

  for(i = 0; i < m_poolSize/ElemInLine; i ++)
  {
    for(j = 0; j < ElemInLine; j++)
    {
      if(format == DUMP_HEX)
      {
        printf("[Address: %p] : %p",ptr, *ptr);
      }
      else if(format == DUMP_CHAR)
      {
        printf("[Address: %p] : %c ",ptr, *ptr);
      }
      else
      {
        mem_debug_log("Error dump format.\n");
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

  printf("\n\n Finish to dump memory pool.\n");
}


void StandardMemoryPool :: memory_block_list()
{
  uint32 i = 1;
  Chunk *block = (Chunk *)(m_boundsCheck == 1 ? m_poolMemory + s_boundsCheckSize : m_poolMemory);

  if(block == NULL)
  {
    mem_debug_log("block list is NULL.");
    return;
  }

  printf("Start to print the memory block list.\n\n\n");

  while(block)
  {
    printf("[block: %d] address: %p name: %s Free: %d size:%d-->\n", i, block, block->m_name, block->m_free, block->m_userdataSize);
    block = block->m_next;
    i++;
  }

  printf("[last block] NULL\n\n\n");
}


void StandardMemoryPool :: memory_pool_info()
{
  mem_debug_log("Start to log memory pool information.");

  printf("\n\nMemory Pool information:\n");

  printf("Address: %p\n", m_poolMemory);

  printf("Total space: %lu\n", m_totalPoolSize);

  printf("Free space: %lu\n", m_freePoolSize);

  printf("Trash on allocate: %d\n", m_trashOnAlloc);

  printf("Trash on creation: %d\n", m_trashOnCreation);

  printf("Trash on free: %d\n", m_trashOnFree);

}







