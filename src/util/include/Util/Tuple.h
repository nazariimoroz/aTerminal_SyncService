#pragma once
#include <tuple>
#include <functional>

namespace Utils
{
    template<class T, std::size_t N>
    concept HasTupleElementC =
      requires(T t) {
        typename std::tuple_element_t<N, std::remove_const_t<T>>;
        { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
      };

    template<class T>
    concept TupleLikeC = !std::is_reference_v<T>
      && requires(T t) {
        typename std::tuple_size<T>::type;
        requires std::derived_from<
          std::tuple_size<T>,
          std::integral_constant<std::size_t, std::tuple_size_v<T>>
        >;
      } && []<std::size_t... N>(std::index_sequence<N...>) {
          return (HasTupleElementC<T, N> && ...);
      }(std::make_index_sequence<std::tuple_size_v<T>>());

    template<class F, class Tuple, std::size_t... I>
    consteval bool tuple_predicate_impl(std::index_sequence<I...>)
    {
        using T = std::remove_reference_t<Tuple>;
        return (std::predicate<
                    F&,
                    std::tuple_element_t<I, T>
                > && ...);
    }

    template<class F, class Tuple>
    concept TuplePredicateC =
        tuple_predicate_impl<F, Tuple>(
            std::make_index_sequence<
                std::tuple_size_v<std::remove_reference_t<Tuple>>
            >{}
        );

    template<TupleLikeC TupleT, std::invocable FunT>
    constexpr void TupleForEach(TupleT&& t, FunT&& f) {
        std::apply([&](auto&&... xs) {
            (f(std::forward<decltype(xs)>(xs)), ...);
        }, std::forward<TupleT>(t));
    }

    template<TupleLikeC TupleT, class FunT>
    requires TuplePredicateC<FunT, TupleT>
    constexpr void TupleForEachWithContinue(const TupleT& t, FunT&& f) {
        std::apply([&](auto&&... xs) {
            ( (!std::invoke(f, std::forward<decltype(xs)>(xs))) || ... );
        }, t);
    }
}
