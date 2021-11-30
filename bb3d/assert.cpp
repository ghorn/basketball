#include <cstdlib>
#include <mutex>

std::mutex g_exit_mutex;

[[noreturn]] void exit_thread_safe(int exit_code) {
    const std::lock_guard<std::mutex> lock(g_exit_mutex);
    exit(exit_code); // NOLINT
}
