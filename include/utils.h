#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef DEBUG
#define DEBUG_ASSERT_NOT_REACH assert(false && "should not reach here");
#else
#define DEBUG_ASSERT_NOT_REACH
#endif

#endif
