#ifndef __UCOM_RTOS_H__
#define __UCOM_RTOS_H__

//max memory pool size 2G.
#define MAX_MEMPOOL_SIZE 2*1024*1024*1024UL
#define DUMP_ELEMENT_PER_LINE 4
#define UCOM_POOL_NAME_LEN 16
#define DEFAULT_BLOCK_NAME "unknown"
#define MEMORY_POOL_BLOCK_NAME "pool"
#define BLOCK_SIZE_MIN 16


typedef unsigned int UComUInt32;
typedef int UComInt32;
typedef char UComInt8;
typedef char UComUInt8;
typedef char UComChar;


typedef enum{
    UCOM_FAILURE = 0,
    UCOM_SUCCESS
} RETURN_TYPE;

typedef struct MemParam
{
  UComUInt8 *poolBase;
  UComChar *name;
  UComUInt32 freePoolSize;
  UComUInt32 totalPoolSize;
  UComUInt8 boundsCheck;
  UComUInt8 trashOnCreation;
  UComUInt8 trashOnAlloc;
  UComUInt8 trashOnFree;
} MemParam_t;


typedef struct MemoryPool
{
  struct MemParam memParam;
  struct Chunk *header;
  struct MemoryPool *next;
  struct MemoryPool *prev;
} MemoryPool_t;

typedef struct Chunk{
  UComUInt32 userdataSize;
  UComUInt32 freedataSize;
  UComUInt32 free;
  UComChar *name;
  struct Chunk *prev;
  struct Chunk *next;
} Chunk_t;

typedef enum
{
  DUMP_ = 0,
  DUMP_HEX,
  DUMP_CHAR
} dump_type;


#define MEMORY_POOL_HEADER_SIZE sizeof(MemoryPool_t)

#define MEMORY_CHUNK_HEADER_SIZE sizeof(Chunk_t)

#define DEFAULT_CHUNK_NAME "Header";

UComInt32 UComOsMemCreatePool(MemoryPool_t ** mem, void* mem_area, UComUInt32 size);

UComInt32 MemoryPoolCreate(MemoryPool_t **mem, MemParam_t *params);

UComInt32 MemoryPoolDestroy(MemoryPool_t *mem);

void *MemoryPoolAllocate(MemoryPool_t *mem, UComUInt32 size);

int MemoryPoolFree(MemoryPool_t *mem, void* ptr);

int integrityCheck(MemoryPool_t *mem);

void memory_pool_info(MemoryPool_t *mem);

void dumpToFile(const UComChar *fileName, MemoryPool_t *mem, const UComUInt32 itemsPerLine, const UComUInt32 format);

#endif

