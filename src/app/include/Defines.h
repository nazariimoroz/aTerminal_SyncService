#pragma once
#include "rfl/Size.hpp"
#include "rfl/Validator.hpp"
#include "rfl/comparisons.hpp"

namespace Defines
{
    using namespace rfl;

    struct ErrorDTO
    {
        std::string error;
    };

    using Password = Validator<std::string, Size<AllOf<Minimum<8>, Maximum<32>>>>;
} // namespace Defines
