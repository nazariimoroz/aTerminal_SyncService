#pragma once

#include <any>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

#include "Defines.h"
#include "Util/LogicException.h"

namespace Service
{
    template <class T>
    concept CommandC = requires { typename T::Result; };

    class MessageBus
    {
    public:
        template <CommandC Cmd>
        using ResultT = typename Cmd::Result;

        template <CommandC Cmd>
        void registerHandler(std::function<ResultT<Cmd>(const Cmd&)> handler)
        {
            std::lock_guard lock(_mutex);

            const std::type_index key = typeid(Cmd);

            _handlers[key] = [handler = std::move(handler)](const void* rawCmd) -> std::any
            {
                const Cmd& cmd = *static_cast<const Cmd*>(rawCmd);
                ResultT<Cmd> r = handler(cmd);
                return std::any(std::move(r));
            };
        }

        template <CommandC Cmd, class Obj>
        void registerHandler(std::shared_ptr<Obj> obj, ResultT<Cmd> (Obj::*method)(const Cmd&))
        {
            std::lock_guard lock(_mutex);
            const std::type_index key = typeid(Cmd);

            std::weak_ptr<Obj> weak = std::move(obj);

            _handlers[key] = [weak, method](const void* rawCmd) -> std::any
            {
                auto locked = weak.lock();
                if (!locked)
                {
                    throw Util::LogicException("Handler object is no longer available");
                }

                const Cmd& cmd = *static_cast<const Cmd*>(rawCmd);
                ResultT<Cmd> r = ((*locked).*method)(cmd);
                return std::any(std::move(r));
            };
        }

        template <CommandC Cmd, class Obj>
        void registerHandler(std::shared_ptr<Obj> obj, ResultT<Cmd> (Obj::*method)(const Cmd&) const)
        {
            std::lock_guard lock(_mutex);
            const std::type_index key = typeid(Cmd);

            std::weak_ptr<Obj> weak = std::move(obj);

            _handlers[key] = [weak, method](const void* rawCmd) -> std::any
            {
                auto locked = weak.lock();
                if (!locked)
                {
                    throw Util::LogicException("Handler object is no longer available");
                }

                const Cmd& cmd = *static_cast<const Cmd*>(rawCmd);
                ResultT<Cmd> r = ((*locked).*method)(cmd);
                return std::any(std::move(r));
            };
        }

        template <CommandC Cmd>
        ResultT<Cmd> call(const Cmd& cmd)
        {
            std::function<std::any(const void*)> fn;

            {
                std::lock_guard lock(_mutex);
                auto it = _handlers.find(std::type_index(typeid(Cmd)));
                if (it == _handlers.end())
                {
                    throw Util::LogicException("No handler registered for this Command type");
                }
                fn = it->second;
            }

            std::any boxed = fn(&cmd);
            return std::any_cast<ResultT<Cmd>>(boxed);
        }

    private:
        std::mutex _mutex;
        std::unordered_map<std::type_index, std::function<std::any(const void*)>> _handlers;
    };
} // namespace Service
