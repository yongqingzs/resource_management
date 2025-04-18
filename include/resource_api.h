#pragma once

#include <chrono>
#include "resource_node.h"
#include "resource_registry.h"
#include "resource_indexer.h"

template<typename Func>
long long measureTime(Func func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}
