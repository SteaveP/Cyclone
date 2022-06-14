#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <optional>
#include <unordered_map>

namespace Cyclone
{

using int8 = int8_t;
using uint8 = uint8_t;

using int16 = int16_t;
using uint16 = uint16_t;

using int32 = int32_t;
using uint32 = uint32_t;

using int64 = int64_t;
using uint64 = uint64_t;

using String = std::string;

template<typename T>
using Ptr = std::shared_ptr<T>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using Vector = std::vector<T>;

template<typename K, typename V, typename Hasher = std::hash<K>>
using HashMap = std::unordered_map<K, V, Hasher>;

template<typename T, uint32 COUNT>
using Array = std::array<T, COUNT>;

template<typename T>
using Optional = std::optional<T>;

} // namespace Cyclone
