#include "StandardMemoryPool.hpp"
#include "MemoryPool.hpp"
#include <assert.h>
#include <iostream>
#include <stdio.h>

void *StandardMemoryPool :: allocate(uint32 size)
{
#ifdef DEBUG_ON
  printf("[%s] Start to allocate block with size.\n", __FUNCTION__);
#endif
  uint32 requiredSize = size + sizeof(Chunk);

  if(m_boundsCheck)
  {
    requiredSize += s_boundsCheckSize *2;
  }
  
  Chunk *block = (Chunk *)(m_boundsCheck == 1 ? m_poolMemory + s_boundsCheckSize : m_poolMemory);

  while(block)
  {
     if(block->m_free && block->m_userdataSize >= requiredSize ) 
        break;
     block = block->m_next;
  }

  uint32 * blockData = (uint32 *)block;

  // If block is found, return NULL
  if(!block)
  {
    return NULL;
  }
  // If the block is valid, create a new free block with what remains of the block memory
  uint32 freeUserDataSize = block->m_userdataSize - requiredSize;
  if( freeUserDataSize > s_minFreeBlockSize)
  {
    Chunk freeBlock(freeUserDataSize);
    freeBlock.m_next = block->m_next;
    freeBlock.m_prev = block;
    freeBlock.write( blockData + requiredSize );
    if(freeBlock.m_next)
      freeBlock.m_next->m_prev = (Chunk*)(blockData + requiredSize);
    if(m_boundsCheck)
       memcpy( blockData + requiredSize - s_boundsCheckSize, s_startBound, s_boundsCheckSize );
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
    memcpy( blockData - s_boundsCheckSize, s_startBound, s_boundsCheckSize );
    memcpy( blockData + sizeof(Chunk) + block->m_userdataSize, s_endBound, s_boundsCheckSize );
  }

  //Trash on alloc if required
  if(m_trashOnAlloc)
  {
    memset(blockData + sizeof(Chunk), s_trashOnAllocSignature, block->m_userdataSize);
  }

#ifdef DEBUG_ON
  dumpToStdOut(4);
#endif
  return (blockData + sizeof(Chunk));
}

void StandardMemoryPool :: free(void* ptr)
{
    // is a valid node?
    if(!ptr) 
      return;

    Chunk* block = (Chunk*)((uint8 *)ptr - sizeof(Chunk));

    assert(block->m_free == false);

    if(block->m_free)
      return;

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
         if( block->m_next->m_free )
         {
            // We will point to next's next
            next = block->m_next->m_next;

            if(block->m_next->m_next)
              block->m_next->m_next->m_prev = headBlock;

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

    // Include the next node in the block size so we trash it as well
    fullBlockSize += m_boundsCheck == 1 ? block->m_next->m_userdataSize + sizeof(Chunk) + s_boundsCheckSize * 2 : block->m_next->m_userdataSize + sizeof(Chunk);
  }

  // Create the free block
  uint32* freeBlockStart = (uint32*)headBlock;

  if(m_trashOnFree)
    memset(m_boundsCheck == 1 ? freeBlockStart - s_boundsCheckSize : freeBlockStart, s_trashOnFreeSignature, fullBlockSize);

  uint32 freeUserDataSize = fullBlockSize - sizeof(Chunk);

  freeUserDataSize = (m_boundsCheck == 1) ? freeUserDataSize - s_boundsCheckSize * 2 : freeUserDataSize;    

  Chunk freeBlock(freeUserDataSize);
  freeBlock.m_prev = prev;
  freeBlock.m_next = next;
  freeBlock.write(freeBlockStart);

  // Move the memory around if guards are needed
  if(m_boundsCheck)
  {
    memcpy( freeBlockStart - s_boundsCheckSize, s_startBound, s_boundsCheckSize );
    memcpy( freeBlockStart + sizeof(Chunk) + freeUserDataSize, s_endBound, s_boundsCheckSize );
  }
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
        return false; 

      if(memcmp(((uint8*)temp) + sizeof(Chunk) + temp->m_userdataSize, s_endBound, s_boundsCheckSize) != 0)
        return false; 

      temp = temp->m_next;
    }
  }
  return true;
}

/**
*	\brief		Dump the memory state to file
*/
void StandardMemoryPool :: dumpToFile(const std::string& fileName, const uint32 itemsPerLine) const
{
  FILE* f = NULL;
  f = fopen(fileName.c_str(), "w+");
  if(f)
  {
    fprintf(f, "Memory pool ----------------------------------\n");
    fprintf(f, "Type: Standard Memory\n");
    fprintf(f, "Total Size: %d\n", m_totalPoolSize);
    fprintf(f, "Free Size: %d\n", m_freePoolSize);

    // Now search for a block big enough
    Chunk* block = (Chunk*)( m_boundsCheck == 1 ? m_poolMemory + s_boundsCheckSize : m_poolMemory);

    while(block)
    {
      if(block->m_free)
      {
        fprintf(f, "Free:\t0x%p [Bytes:%d]\n", block, block->m_userdataSize);
      }
      else
      {
        fprintf(f, "Used:\t0x%p [Bytes:%d]\n", block, block->m_userdataSize);
        block = block->m_next;
      }
    }

    fprintf(f, "\n\nMemory Dump:\n");
    uint8* ptr = m_poolMemory;
    uint8* charPtr = m_poolMemory;

    fprintf(f, "Start: 0x%p\n", ptr);
    uint8 i = 0;

    // Write the hex memory data
    uint32 bytesPerLine = itemsPerLine * 4;

    fprintf(f, "\n0x%p: ", ptr);
    fprintf(f, "%02x", *(ptr) );
    ++ptr;

    for(i = 1; ((uint32)(ptr - m_poolMemory) < m_totalPoolSize); ++i, ++ptr)
    {
      if(i == bytesPerLine)
      {
        // Write all the chars for this line now
        fprintf(f, "  ", charPtr);
        for(uint32 charI = 0; charI<bytesPerLine; ++charI, ++charPtr)
        {
            fprintf(f, "%c", *charPtr);
        }
        charPtr = ptr;

        // Write the new line memory data
        fprintf(f, "\n0x%p: ", ptr);
        fprintf(f, "%02x", *(ptr) );
        i = 0;
      }
      else
      {
        fprintf(f, ":%02x", *(ptr) );
      }
    }

    // Fill any gaps in the tab
    if( (uint32)(ptr - m_poolMemory) >= m_totalPoolSize)
    {
      uint32 lastLineBytes = i;

      for(i; i< bytesPerLine; i++)
      {
        fprintf(f," --");
      }

      // Write all the chars for this line now
      fprintf(f, "  ", charPtr);

      for(uint32 charI = 0; charI<lastLineBytes; ++charI, ++charPtr)
      {   
        fprintf(f, "%c", *charPtr);
      }
      charPtr = ptr;
    }
  }

  fclose(f);
}

void StandardMemoryPool :: dumpToStdOut(uint32 ElemInLine) const
{
  int i = 0, j = 0;
  int residue = 0;
  uint8 *ptr = m_poolMemory;
  printf("\n\n Start to dump memory pool. \n");  

  residue = m_poolSize%ElemInLine;

  for(i = 0; i < m_poolSize/ElemInLine; i ++)
  {
    for(j = 0; j < ElemInLine; j++)
    {
      printf("[Address: 0x%x] : 0X%x ",ptr, *ptr);
      ptr ++;
    }
    printf("\n");
  }

  if(residue)
  {
    for(i = 0; i < residue; i++)
    {
      printf("0x%x ", *ptr);
      ptr ++;
    }
    printf("\n");
  }

  printf("\n\n Finish to dump memory pool.\n");
}













