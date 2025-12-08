#pragma once

namespace Utils
{
    template <class... Ts>
    struct Cbs : Ts...
    {
        using Ts::operator()...;
    };

    template <class... Ts>
    Cbs(Ts...) -> Cbs<Ts...>;
}
