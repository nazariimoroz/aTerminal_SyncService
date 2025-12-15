#pragma once

#include <expected>
#include <format>
#include <string>

#include <rfl/Rename.hpp>

#include "Port/Plugin/PluginOptionsStorage.h"
#include "Port/Plugin/PluginOptionsUpdateableStorage.h"
#include "Util/Errors.h"

namespace Service
{
    class MessageBus;
}

namespace Service::Plugin
{
    struct PostPluginOptionsResult { };

    struct PostPluginOptionsCommand
    {
        int userId;
        std::string pluginName;
        bool isEnabled;
        bool isFavorite;
        std::unordered_map<std::string, std::string> fields;

        using Result = PostPluginOptionsResult;
        using Error = Error::MutStrError;
    };

    template <Port::Plugin::PluginOptionsUpdateableStorageC PluginOptionsStorageT>
    class PostPluginOptionsHandler;

    template <Port::Plugin::PluginOptionsUpdateableStorageC PluginOptionsStorageT>
    std::shared_ptr<PostPluginOptionsHandler<PluginOptionsStorageT>> makePostPluginOptionsHandler(
        Service::MessageBus& messageBus, PluginOptionsStorageT& pluginOptionsStorage)
    {
        using HandlerT = PostPluginOptionsHandler<PluginOptionsStorageT>;
        auto self = std::shared_ptr<HandlerT>(new HandlerT(messageBus, pluginOptionsStorage));

        self->getMessageBus().template registerHandler<PostPluginOptionsCommand>(self,
                                                                                &HandlerT::execute);

        return self;
    }

    template <Port::Plugin::PluginOptionsUpdateableStorageC PluginOptionsStorageT>
    class PostPluginOptionsHandler
    {
        PostPluginOptionsHandler(Service::MessageBus& messageBus,
                                 PluginOptionsStorageT& pluginOptionsStorage) :
            _messageBus(messageBus), _pluginOptionsStorage(pluginOptionsStorage)
        {
        }

    public:
        PostPluginOptionsHandler(PostPluginOptionsHandler&&) = default;

        template <Port::Plugin::PluginOptionsUpdateableStorageC InnerStorageT>
        friend std::shared_ptr<PostPluginOptionsHandler<InnerStorageT>> Service::Plugin::makePostPluginOptionsHandler(
            Service::MessageBus& messageBus, InnerStorageT& pluginOptionsStorage);

    public:
        std::expected<PostPluginOptionsCommand::Result, PostPluginOptionsCommand::Error> execute(
            const PostPluginOptionsCommand& command)
        {
            auto uow = getPluginOptionsStorage().beginWork();

            auto existing = getPluginOptionsStorage().findByUserIdAndName(command.userId, command.pluginName);

            Domain::PluginOptions po;
            if (existing)
            {
                po = *existing;
            }
            else
            {
                po.setUserId(command.userId);
                po.setPluginName(command.pluginName);
            }

            po.setEnabled(command.isEnabled);
            po.setFavorite(command.isFavorite);
            po.setFields(command.fields);

            getPluginOptionsStorage().update(po);

            uow.commit();

            return PostPluginOptionsResult{};
        }

    protected:
        Service::MessageBus& _messageBus;
        Service::MessageBus& getMessageBus() const { return _messageBus; }

        PluginOptionsStorageT& _pluginOptionsStorage;
        PluginOptionsStorageT& getPluginOptionsStorage() const { return _pluginOptionsStorage; }
    };

} // namespace Service::Plugin
