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
#include <sstream>
#include <assert.h>
#include <stack>
#include <map>
#include <iomanip>
#include <functional>
#include <tuple>
#include <chrono>

#include "json/json-forwards.h"
#include "json/json.h"

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
#define THROW(exception) \
    do { \
        std::ostringstream oss; \
        oss << "Error at line " << __LINE__ << " in function " << __func__; \
        throw std::runtime_error(oss.str()); \
    } while (0)

template <typename R, typename F, typename... Args>
std::pair<double, R> measureTime(F func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    R result = std::invoke(func, std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double, std::milli>(end - start).count();
    return { duration, result };
}
//void version
template <typename F, typename... Args>
double measureTime(F func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    std::invoke(func, std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double, std::milli>(end - start).count();
    return duration;
}

//全局日志
#define LOG logFile
std::fstream logFile;

#endif

