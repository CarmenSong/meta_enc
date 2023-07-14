#include <iostream>
#define LOG(severity) std::cout
#define PLOG(severity) std::cerr

#define CHECK(x) \
    likely((x)) || std::cerr << "Check failed: " #x << " "

#define likely(x) __builtin_expect(!!(x), 1)
