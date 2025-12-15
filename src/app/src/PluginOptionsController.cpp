#include "Rest/Controller/PluginOptionsController.h"

#include <Poco/Logger.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <expected>

#include "Defines.h"
#include "Rest/Middleware/AuthMiddleware.h"
#include "Service/MessageBus.h"
#include "Service/Plugin/GetPluginOptionsHandler.h"
#include "Service/Plugin/PostPluginOptionsHandler.h"
#include "ctre.hpp"
#include "rfl/fields.hpp"
#include "rfl/from_generic.hpp"
#include "rfl/json/read.hpp"
#include "rfl/json/write.hpp"
#include "rfl/to_generic.hpp"


Rest::Controller::PluginOptionsController::PluginOptionsController(Middleware::AuthMiddleware& authMiddleware,
                                                                   Service::MessageBus& messageBus,
                                                                   Poco::Logger& logger) :
    _authMiddleware(authMiddleware), _messageBus(messageBus), _logger(logger)
{
}

void Rest::Controller::PluginOptionsController::handleRequest(Poco::Net::HTTPServerRequest& request,
                                                              Poco::Net::HTTPServerResponse& response)
{
    response.setContentType("application/json");

    auto amResult = getAuthMiddleware().execute(request, response);
    if (!amResult)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
        response.send() << rfl::json::write(Defines::ErrorDTO{std::string(amResult.error().errorMessage)});
        return;
    }

    const int userId = amResult->id;

    const auto m = request.getMethod();

    if (m == "GET")
    {
        handleGetRequest(userId, request, response);
        return;
    }
    if (m == "POST")
    {
        handlePostRequest(userId, request, response);
        return;
    }

    response.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
    response.send() << rfl::json::write(Defines::ErrorDTO("Method Not Allowed"));
}

void Rest::Controller::PluginOptionsController::handleGetRequest(int userId, Poco::Net::HTTPServerRequest& request,
                                                                 Poco::Net::HTTPServerResponse& response)
{
    const std::unordered_map<std::string, std::string> qp = std::invoke([&]
    {
        std::unordered_map<std::string, std::string> toRet;
        auto params = Poco::URI(request.getURI()).getQueryParameters();

        for (auto& [name, value] : params)
        {
            toRet.emplace(std::move(name), std::move(value));
        }
        return toRet;
    });

    const auto dto = std::invoke([&]() -> std::expected<PluginOptionsGetRequest, std::string>
    {
        auto obj = rfl::to_generic(PluginOptionsGetRequest{}).to_object().value();
        for (const rfl::MetaField& f : rfl::fields<PluginOptionsGetRequest>())
        {
            auto iter = qp.find(f.name());
            if (iter == qp.end())
            {
                return std::unexpected(f.name() + " was not passed");
            }

            obj[f.name()] = iter->second;
        }
        return rfl::from_generic<PluginOptionsGetRequest>(obj).value();
    });

    if (!dto)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.send() << rfl::json::write(Defines::ErrorDTO(dto.error()));
        return;
    }

    /** Main auth logic */
    Service::Plugin::GetPluginOptionsCommand getPluginOptionCommand {};
    getPluginOptionCommand.userId = userId;
    getPluginOptionCommand.pluginName = dto->pluginName.value();
    const auto getPluginOptionResult = getMessageBus().call(getPluginOptionCommand);
    if (!getPluginOptionResult)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        response.send();
        return;
    }

    const auto& pluginOptions = getPluginOptionResult->pluginOptions;

    /** Response */
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.send() << rfl::json::write(PluginOptionsGetResponse{
        .pluginName = pluginOptions.getPluginName(),
        .isEnabled = pluginOptions.isEnabled(),
        .isFavorite = pluginOptions.isFavorite(),
        .fields = pluginOptions.getFields(),
    });
}

void Rest::Controller::PluginOptionsController::handlePostRequest(int userId, Poco::Net::HTTPServerRequest& request,
                                                                  Poco::Net::HTTPServerResponse& response)
{
    std::stringstream bodyBuffer;
    Poco::StreamCopier::copyStream(request.stream(), bodyBuffer);
    const std::string rawBody = bodyBuffer.str();

    auto dto = rfl::json::read<PluginOptionsPostRequest>(rawBody);
    if (!dto)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.send() << rfl::json::write(Defines::ErrorDTO(dto.error().what()));
        return;
    }

    Service::Plugin::PostPluginOptionsCommand postPluginOptionsCommand {};
    postPluginOptionsCommand.userId = userId;
    postPluginOptionsCommand.pluginName = dto->pluginName.value();
    postPluginOptionsCommand.isEnabled = dto->isEnabled.value();
    postPluginOptionsCommand.isFavorite = dto->isFavorite.value();
    postPluginOptionsCommand.fields = dto->fields.value();
    const auto postPluginOptionResult = getMessageBus().call(postPluginOptionsCommand);
    if (!postPluginOptionResult)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        getLogger().error("PluginOptionsController::handlePostRequest: %s", postPluginOptionResult.error().errorMessage);
        response.send();
        return;
    }

    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.send();
}
