#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <optional>
#include <unordered_map>
#include <set>

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
using Set = std::set<T>;

template<typename T>
using Optional = std::optional<T>;

using RawPtr = void*;


template<typename T>
constexpr inline std::remove_reference_t<T>&& MoveTemp(T&& Arg) noexcept
{
    return std::move<T>(std::forward<T>(Arg));
}

template<typename T, typename... Args>
inline Ptr<T> MakeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
template<typename T, typename... Args>
inline UniquePtr<T> MakeUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
inline String ToString(T&& Data)
{
    return std::to_string(std::forward<T>(Data));
}

} // namespace Cyclone
