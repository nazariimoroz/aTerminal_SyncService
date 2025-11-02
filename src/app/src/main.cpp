#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/ServerApplication.h>

#include "Infra/InMemoryUserStorage.h"
#include "Rest/Controller/AuthController.h"
#include "Rest/RequestRouter.h"
#include "Rest/Resolver/AuthControllerResolver.h"
#include "Service/JwtHandler.h"
#include "Service/MessageBus.h"
#include "Service/User/LoginUserHandler.h"
#include "Service/User/RegisterUserHandler.h"
#include "Util/Crypto/PasswordHasher.h"

class ApplicationMain final : public Poco::Util::ServerApplication
{
public:
    ApplicationMain()
    {
        setUnixOptions(true);
    }
    ~ApplicationMain() override = default;

protected:
    std::uint32_t HTTP_PORT;
    std::string JWT_SECRET;

protected:
    void defineOptions(Poco::Util::OptionSet& options) override
    {
        ServerApplication::defineOptions(options);

        using Poco::Util::Option;
        using Poco::Util::OptionCallback;

        options.addOption(Option("configFile", "c", "path to config file")
                              .required(true)
                              .repeatable(false)
                              .argument("file")
                              .binding("configFile"));
    }

    void initialize(Application& self) override
    {
        ServerApplication::initialize(self);

        if (config().hasProperty("configFile"))
        {
            std::string cfgPath = config().getString("configFile");
            loadConfiguration(cfgPath);
        }

        /** Set config params */
        HTTP_PORT = config().getInt("http.port", 8080);
        JWT_SECRET = config().getString("jwt.secret_key", "testtesttest");

        logger().information("initialize");
    }

    void uninitialize() override
    {
        logger().information("uninitialize");

        ServerApplication::uninitialize();
    }

    int main(const std::vector<std::string>& args) override
    {
        /** Init Utils */
        const auto passwordHasher = std::make_shared<Util::Crypto::PasswordHasher>();

        /** Init Infra */
        const auto userStorage = std::make_shared<Infra::InMemoryUserStorage>();

        /** Init Services */
        const auto messageBus = std::make_shared<Service::MessageBus>();
        const auto jwtHandler = Service::JwtHandler::make(JWT_SECRET, messageBus);
        const auto registerUserHandler = Service::User::RegisterUserHandler::make(messageBus, userStorage, passwordHasher);
        const auto loginUserHandler = Service::User::LoginUserHandler::make(messageBus, userStorage, passwordHasher);

        /** Init REST Server */
        const auto restSoket = Poco::Net::ServerSocket(HTTP_PORT);
        const auto restParams = new Poco::Net::HTTPServerParams;
        restParams->setMaxQueued(64);
        restParams->setMaxThreads(4);

        const std::vector<std::shared_ptr<Rest::ControllerResolver>> restControllerResolvers =
        {
            std::make_shared<Rest::Resolver::AuthControllerResolver>(messageBus, logger())
        };

        const auto restRequestRouter = new Rest::RequestRouter(restControllerResolvers);
        auto restServer = Poco::Net::HTTPServer(restRequestRouter, restSoket, restParams);

        logger().information("Starting REST server localhost:" + std::to_string(HTTP_PORT));
        restServer.start();

        waitForTerminationRequest();

        logger().information("Shutting down HTTP server...");
        restServer.stop();

        return Application::EXIT_OK;
    }
};

POCO_SERVER_MAIN(ApplicationMain)
