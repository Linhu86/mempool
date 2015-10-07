#ifndef __MEMORY_POOL_TYPE_HPP__
#define __MEMORY_POOL_TYPE_HPP__

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned long uint64;


typedef enum{
  FALSE = -1,
  TRUE = 0
} result_t;

typedef enum
{
  DUMP_ = 0,
  DUMP_HEX,
  DUMP_CHAR
} dump_type;


#endif

