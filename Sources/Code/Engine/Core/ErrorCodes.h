#pragma once

namespace Cyclone
{

enum class C_STATUS
{
    C_STATUS_OK = 0,
    C_STATUS_ERROR = 1,
    C_STATUS_INVALID_ARG = 2,
    C_STATUS_SHOULD_EXIT = 3,

    C_STATUS_COUNT
};

#define C_SUCCEEDED(res) ((res) == Cyclone::C_STATUS::C_STATUS_OK || (res) == Cyclone::C_STATUS::C_STATUS_SHOULD_EXIT)

} // namespace Cyclone
