#ifndef __MEMLOG_HPP__
#define __MEMLOG_HPP__

#include <stdio.h>

#define mem_debug_log(fmt, ...) printf("[%s]"#fmt"\n", __FUNCTION__, ##__VA_ARGS__)

#endif

