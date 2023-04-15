#ifndef IG_NOOP_5_COMMON_H
#define IG_NOOP_5_COMMON_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <queue>
#include <iomanip>
#include <time.h>
#include <algorithm>
#include <windows.h>
#include <direct.h>
#include <sstream>
#include <assert.h>
#include <stack>
#include <map>

#define DEBUG_LEVEL 2

#define __output(...) \
    printf(__VA_ARGS__); 

#if DEBUG_LEVEL == 1
#define __format(__fmt__) "<%s>: " __fmt__ "\n"
#define DEBUG(__fmt__, ...) \
         __output(__format(__fmt__), __FUNCTION__, ##__VA_ARGS__);
#elif DEBUG_LEVEL == 2
#define __format(__fmt__) "(%d)-<%s>: " __fmt__ "\n"
#define DEBUG(__fmt__, ...) \
         __output(__format(__fmt__), __LINE__, __FUNCTION__, ##__VA_ARGS__);
#else 
#define __format(__fmt__) "%s(%d)-<%s>: " __fmt__ "\n"
#define DEBUG(__fmt__, ...) \
         __output(__format(__fmt__), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#endif

#define GDBSTOP(expr) if (expr) asm("int3");
#endif
