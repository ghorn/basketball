#pragma once

#include <cstdio>    // for fprintf, stderr
#include <cstdlib>   // for exit, EXIT_FAILURE

#define ASSERT(expr) {                                                  \
  if (!(expr)) {                                                        \
    fprintf(stderr, "Assertion '" #expr "' failed at %s, line %d.\n", __FILE__, __LINE__); \
    std::exit(EXIT_FAILURE);                                            \
  } \
}
