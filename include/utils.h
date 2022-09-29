#ifndef _UTILS_H_
#define _UTILS_H_
#include <cassert>
#include <iostream>

#ifdef DEBUG
#define DEBUG_ASSERT_NOT_REACH                                  \
    std::cerr << "\033[1mFATAL ERROR, aborting now..\033[0m\n"; \
    assert(false && "should not reach here");
#else
#define DEBUG_ASSERT_NOT_REACH ;
#endif

#ifdef DEBUG
#define DEBUG_ASSERT(cond) assert(cond)
#else
#define DEBUG_ASSERT(cond)
#endif

#endif
