#pragma once

#include <Poco/Exception.h>

#include "rfl/Size.hpp"
#include "rfl/Validator.hpp"
#include "rfl/comparisons.hpp"

namespace Defines
{
    template <typename Callback, typename RetT, typename... ArgsT>
    concept InvocableC =
        std::invocable<Callback, ArgsT...> &&
        std::same_as<std::invoke_result_t<Callback, ArgsT...>, RetT>;

    struct ErrorDTO
    {
        std::string error;
    };

    using Password = rfl::Validator<std::string, rfl::Size<rfl::AllOf<rfl::Minimum<8>, rfl::Maximum<32>>>>;
} // namespace Defines

