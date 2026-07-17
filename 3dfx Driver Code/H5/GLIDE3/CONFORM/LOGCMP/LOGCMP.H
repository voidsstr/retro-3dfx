
#ifndef _SIFT_H_
#define _SIFT_H_

#include "loginfo.h"

#define ASSERT(_x) \
  if (!(_x)) { \
    printf("Assertion failed, line %d. Exiting.\n", __LINE__); \
    fprintf(stderr, "Assertion failed, line %d. Exiting.\n", __LINE__); \
  }

#ifndef BIT
#define BIT(n)  (1UL<<(n))
#endif

#define FLAGS_NONE 0
#define FLAGS_IGNORE_TEST_ARGS BIT(0)


#define TEST_STARTED 1
#define TEST_DONE    0

extern int Debug;

int parse_log(Log *log);
void compare_logs(unsigned int flags, Log *log1, Log *log2);
void report(Log *log1, Log *log2);

#endif // not _SIFT_H_
