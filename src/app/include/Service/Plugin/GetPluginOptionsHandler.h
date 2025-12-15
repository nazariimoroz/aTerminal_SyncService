#pragma once

#include <expected>
#include <string>
#include <variant>
#include <vector>

#include <rfl/Rename.hpp>

#include "Port/Plugin/PluginOptionsStorage.h"
#include "Util/Errors.h"

namespace Service
{
    class MessageBus;
}

namespace Service::Plugin
{
    struct GetPluginOptionsNotFountError {};

    struct GetPluginOptionsResult
    {
        Domain::PluginOptions pluginOptions;
    };

    struct GetPluginOptionsCommand
    {
        int userId;
        std::string pluginName;

        using Result = GetPluginOptionsResult;
        using Error = GetPluginOptionsNotFountError;
    };

    template <Port::Plugin::PluginOptionsStorageC PluginOptionsStorageT>
    class GetPluginOptionsHandler;

    template <Port::Plugin::PluginOptionsStorageC PluginOptionsStorageT>
    std::shared_ptr<GetPluginOptionsHandler<PluginOptionsStorageT>> makeGetPluginOptionsHandler(
        Service::MessageBus& messageBus, PluginOptionsStorageT& pluginOptionsStorage)
    {
        using HandlerT = GetPluginOptionsHandler<PluginOptionsStorageT>;
        auto self = std::shared_ptr<HandlerT>(new HandlerT(messageBus, pluginOptionsStorage));

        self->getMessageBus().template registerHandler<GetPluginOptionsCommand>(self,
                                                                                &HandlerT::execute);

        return self;
    }

    template <Port::Plugin::PluginOptionsStorageC PluginOptionsStorageT>
    class GetPluginOptionsHandler
    {
        GetPluginOptionsHandler(Service::MessageBus& messageBus,
                                PluginOptionsStorageT& pluginOptionsStorage) :
            _messageBus(messageBus), _pluginOptionsStorage(pluginOptionsStorage)
        {
        }

    public:
        GetPluginOptionsHandler(GetPluginOptionsHandler&&) = default;

        template <Port::Plugin::PluginOptionsStorageC InnerStorageT>
        friend std::shared_ptr<GetPluginOptionsHandler<InnerStorageT>> Service::Plugin::makeGetPluginOptionsHandler(
            Service::MessageBus& messageBus, InnerStorageT& pluginOptionsStorage);

    public:
        std::expected<GetPluginOptionsCommand::Result, GetPluginOptionsCommand::Error> execute(
            const GetPluginOptionsCommand& command)
        {
            const auto optionsResult =
                getPluginOptionsStorage().findByUserIdAndName(command.userId, command.pluginName);

            if (!optionsResult)
            {
                return std::unexpected(GetPluginOptionsNotFountError{});
            }

            return GetPluginOptionsResult{.pluginOptions = *optionsResult};
        }

    protected:
        Service::MessageBus& _messageBus;
        Service::MessageBus& getMessageBus() const
        {
            return _messageBus;
        }

        PluginOptionsStorageT& _pluginOptionsStorage;
        PluginOptionsStorageT& getPluginOptionsStorage() const
        {
            return _pluginOptionsStorage;
        }
    };

} // namespace Service::Plugin
