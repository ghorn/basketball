#pragma once

#include <cstdio>    // for fprintf, stderr
#include <cstdlib>   // for exit, EXIT_FAILURE

[[noreturn]] void exit_thread_safe(int exit_code);

#define ASSERT(expr) {                                                  \
  if (!(expr)) {                                                        \
    fprintf(stderr, "Assertion '" #expr "' failed at %s, line %d.\n", __FILE__, __LINE__); \
    exit_thread_safe(EXIT_FAILURE);                                                 \
  } \
}
