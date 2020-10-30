#pragma once

#include <cstdio>

#define ASSERT(expr) {                                                  \
  if (!(expr)) {                                                        \
    fprintf(stderr, "Assertion '" #expr "' failed at %s, line %d.\n", __FILE__, __LINE__); \
    exit(EXIT_FAILURE);                                                 \
  } \
}
