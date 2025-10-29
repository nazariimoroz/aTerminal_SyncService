#include "Service/MessageBus.h"

std::unique_ptr<Service::MessageBus> Service::MessageBus::_messageBusInstance = nullptr;

void Service::MessageBus::setInstance(std::unique_ptr<MessageBus> messageBus)
{
    _messageBusInstance = std::move(messageBus);
}
Service::MessageBus& Service::MessageBus::instance()
{
    if (!_messageBusInstance)
        throw std::runtime_error("MessageBus instance is nullptr");

    return *_messageBusInstance;
}
