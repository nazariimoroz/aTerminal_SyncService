#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Application.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>

#include "Rest/RequestRouter.h"

using namespace Poco;

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

protected:
    void defineOptions(Util::OptionSet& options) override
    {
        ServerApplication::defineOptions(options);

        using Poco::Util::Option;
        using Poco::Util::OptionCallback;

        options.addOption(
            Option("configFile", "c", "path to config file")
                .required(true)
                .repeatable(false)
                .argument("file")
                .binding("configFile")
        );
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

        logger().information("initialize");
    }

    void uninitialize() override
    {
        logger().information("uninitialize");

        ServerApplication::uninitialize();
    }

    int main(const std::vector<std::string>& args) override
    {
        logger().information("main started");

        const Net::ServerSocket svs(HTTP_PORT);
        Net::HTTPServerParams::Ptr params = new Net::HTTPServerParams;
        params->setMaxQueued(64);
        params->setMaxThreads(4);

        Net::HTTPServer server(
            new Api::RequestRouter(),
            svs,
            params
        );

        logger().information("Starting HTTP server localhost:" + std::to_string(HTTP_PORT));
        server.start();

        waitForTerminationRequest();

        logger().information("Shutting down HTTP server...");
        server.stop();

        logger().information("main finished");

        return Application::EXIT_OK;
    }
};

POCO_SERVER_MAIN(ApplicationMain)
