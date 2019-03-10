/**
 * 64-byte-aligned std containers
 */
#pragma once
#include <vector>
#include "3rdparty/aligned_allocator/aligned_allocator.hxx"

namespace rqdq {
namespace rcls {

template<typename T>
using list = std::list<T, aligned_allocator<T, 64>>;

template<typename T>
using vector = std::vector<T, aligned_allocator<T, 64>>;

}  // namespace rcls
}  // namespace rqdq
