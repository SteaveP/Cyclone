#pragma once

#include "Helpers.h"

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
using WeakPtr = std::weak_ptr<T>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using Vector = std::vector<T>;

template<typename First, typename Second>
using Pair = std::pair<First, Second>;

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

// supports only power of 2 alignments
template<typename T>
constexpr T AlignPow2(T address, T alignment) noexcept
{
    CASSERT(alignment != 0 && (alignment & (alignment - 1)) == 0); // check power of 2
    return (address + (alignment - 1)) & ~(alignment - 1);
}

template<typename T>
constexpr T Align(T address, T alignment) noexcept
{
    return (address + (alignment - 1)) / alignment * alignment;
}

// Conversion from and to string template values
// Can be extended by typed template specializations (see examples below)
template<typename T>
inline String ConvertToString(const T& Value)
{
    return ToString(Value);
}

template<typename T>
inline bool ConvertFromString(std::string_view Data, T& Value)
{
    return Value = std::atoi(Data.data());
}

template<>
inline bool ConvertFromString(std::string_view Data, String& Value)
{
    Value = Data;
    return true;
}

inline String ToLower(std::string_view Input)
{
    String Output(Input.size(), '\0');

    for (uint32 i = 0; i < (uint32)Input.size(); ++i)
    {
        Output[i] = std::tolower(Input[i]);
    }

    return Output;
}

} // namespace Cyclone
