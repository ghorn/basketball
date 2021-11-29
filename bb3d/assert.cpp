#include <cstdlib>
#include <mutex>

std::mutex exit_mutex;

[[noreturn]] void exit_thread_safe(int exit_code) {
    exit_mutex.lock();
    exit(exit_code);
}

