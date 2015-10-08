#ifndef __MEMLOG_HPP__
#define __MEMLOG_HPP__

#include <stdio.h>

#ifdef MEM_DEBUG_ON
#define mem_debug_log(fmt, ...) printf("[%s]"#fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define mem_debug_log(fmt, ...) \
{                               \
}
#endif

#ifdef MEM_ERROR_MSG_ON
#define mem_error_log(fmt, ...) printf("[%s]"#fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define mem_error_log(fmt, ...) \
{                               \
}
#endif


#endif

