#pragma once
#include <Windows.h>
#include <cstdint>
#include "../../utils/dynamic_import.hpp"

namespace aether::cs2::tier0 {
    inline dynamic_import<"tier0.dll", "Plat_GetRegisteredModules", HMODULE*()> Plat_GetRegisteredModules;
}
