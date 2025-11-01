#pragma once

#include <Poco/Exception.h>

#include "rfl/Size.hpp"
#include "rfl/Validator.hpp"
#include "rfl/comparisons.hpp"
#include "Util/BusinessException.h"

namespace Defines
{
    POCO_DECLARE_EXCEPTION(, InternalException, Util::BusinessException)

    struct ErrorDTO
    {
        std::string error;
    };

    using Password = rfl::Validator<std::string, rfl::Size<rfl::AllOf<rfl::Minimum<8>, rfl::Maximum<32>>>>;
} // namespace Defines

