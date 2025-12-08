#pragma once

#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <typeindex>
#include <unordered_map>
#include <expected>
#include <type_traits>

#include "Defines.h"
#include "Util/LogicException.h"

namespace Service
{
    template <class T>
    concept CommandC = requires 
    { 
        typename T::Result;
        typename T::Error; 
    };

    class MessageBus
    {
    public:
        template <CommandC Cmd>
        using ResultT = typename Cmd::Result;

        template <CommandC Cmd>
        using ErrorT = typename Cmd::Error;

        template <CommandC Cmd, bool HasError = !std::is_void_v<ErrorT<Cmd>>>
        struct ExpectedSelector;

        template <CommandC Cmd>
        struct ExpectedSelector<Cmd, false> {
            using type = ResultT<Cmd>;
        };

        template <CommandC Cmd>
        struct ExpectedSelector<Cmd, true> {
            using type = std::expected<ResultT<Cmd>, ErrorT<Cmd>>;
        };

        template <CommandC Cmd>
        using ExpectedT = typename ExpectedSelector<Cmd>::type;

        template <CommandC Cmd, class Obj>
        using MethodT = ExpectedT<Cmd> (std::remove_reference_t<Obj> ::*) (const Cmd&);

        template <CommandC Cmd, class Obj>
        using ConstMethodT = ExpectedT<Cmd> (std::remove_reference_t<Obj> ::*) (const Cmd&) const;
        

        template <CommandC Cmd, Defines::InvocableC<ExpectedT<Cmd>, const Cmd&> CallbackT>
        void registerHandler(CallbackT&& callback)
        {
            std::unique_lock lock(_mutex);
            const std::type_index key = typeid(Cmd);

            _handlers[key] = [callback = std::forward<CallbackT>(callback)](const void* rawCmd) -> std::any
            {
                const Cmd& cmd = *static_cast<const Cmd*>(rawCmd);
                ExpectedT<Cmd> r = std::invoke(callback, cmd);
                return std::any(std::move(r));
            };
        }

        template <CommandC Cmd, class Obj>
        void registerHandler(Obj& obj, MethodT<Cmd, Obj> method)
        {
            std::unique_lock lock(_mutex);
            const std::type_index key = typeid(Cmd);

            _handlers[key] = [&obj, method](const void* rawCmd) -> std::any
            {
                const Cmd& cmd = *static_cast<const Cmd*>(rawCmd);
                ExpectedT<Cmd> r = std::invoke(method, &obj, cmd);
                return std::any(std::move(r));
            };
        }

        template <CommandC Cmd, class Obj>
        void registerHandler(Obj& obj, ConstMethodT<Cmd, Obj> method)
        {
            std::unique_lock lock(_mutex);
            const std::type_index key = typeid(Cmd);

            _handlers[key] = [&obj, method](const void* rawCmd) -> std::any
            {
                const Cmd& cmd = *static_cast<const Cmd*>(rawCmd);
                ExpectedT<Cmd> r = std::invoke(method, &obj, cmd);
                return std::any(std::move(r));
            };
        }

        template <CommandC Cmd>
        ExpectedT<Cmd> call(const Cmd& cmd)
        {
            std::function<std::any(const void*)> fn;

            {
                std::shared_lock lock(_mutex);
                auto it = _handlers.find(std::type_index(typeid(Cmd)));
                if (it == _handlers.end())
                {
                    throw Util::LogicException("No handler registered for this Command type");
                }
                fn = it->second;
            }

            std::any boxed = std::invoke(fn, &cmd);
            return std::any_cast<ExpectedT<Cmd>>(boxed);
        }

    private:
        std::shared_mutex _mutex;
        std::unordered_map<std::type_index, std::function<std::any(const void*)>> _handlers;
    };
} // namespace Service
