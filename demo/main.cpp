#include <iostream>
#include <memory>
#include <string>
#include "apollo/apollo_client.h"
#include <boost/program_options.hpp>
#include <thread>

namespace po = boost::program_options;
namespace ac = apollo::client;

class ConsoleLogger : public ac::ILogger
{
public:
    ConsoleLogger() = default;
    ~ConsoleLogger() override = default;

    ac::LogLevel getLogLevel() const override
    {
        return log_level_;
    }

    void setLogLevel(ac::LogLevel level) override
    {
        log_level_ = level;
    }

    void log(ac::LogLevel level, const std::string& message) override
    {
        switch (level)
        {
            case ac::LogLevel::Error:
                std::cerr << "ERROR: " << message << std::endl;
                break;
            case ac::LogLevel::Warning:
                std::cerr << "WARNING: " << message << std::endl;
                break;
            case ac::LogLevel::Info:
                std::cout << "INFO: " << message << std::endl;
                break;
            case ac::LogLevel::Debug:
                std::cout << "DEBUG: " << message << std::endl;
                break;
            default:
                break;
        }
    }

private:
    ac::LogLevel log_level_ = ac::LogLevel::Info;  // Default log level
};

int main(int argc, char* argv[])
{
    // Command line options
    std::string apollo_url;
    std::string app_id;
    std::string cluster_name = "default";
    std::vector<std::string> namespaces = {"application"};  // Default namespace
    int poll_interval = 1000;

    // Set up command line options
    po::options_description desc("Apollo C++ Client Demo Options");

    // clang-format off
    desc.add_options()
    ("help,h", "Print help message")
    ("url,u", po::value<std::string>(&apollo_url)->required(), "Apollo server url (required)")
    ("appId,a", po::value<std::string>(&app_id)->required(), "Apollo application ID (required)")
    ("cluster,c", po::value<std::string>(&cluster_name), "Apollo cluster name (default: default)")
    ("namespaces,n", po::value<std::vector<std::string>>(&namespaces)->multitoken(), "Apollo namespaces multiple (default: application)")
    ("interval,t", po::value<int>(&poll_interval), "Polling interval in milliseconds (default: 1000)");
    // clang-format on

    try
    {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        // Display help if requested
        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }

        // Check for required options after help
        po::notify(vm);

        // Print the configuration
        std::cout << "Apollo C++ Client Demo" << std::endl;
        std::cout << "======================" << std::endl;
        std::cout << "Apollo Server: " << apollo_url << std::endl;
        std::cout << "App ID: " << app_id << std::endl;
        std::cout << "Cluster: " << cluster_name << std::endl;

        for (const auto& ns : namespaces)
        {
            std::cout << "namespace: " << ns << std::endl;
        }

        if (poll_interval <= 0)
        {
            throw std::invalid_argument("Polling interval must be greater than 0");
        }
        if (poll_interval < 1000)
        {
            std::cout << "Warning: Polling interval is less than 1000 ms, this may lead to high load on the server."
                      << std::endl;
        }
        std::cout << "Poll Interval: " << poll_interval << " ms" << std::endl;
        std::cout << "======================" << std::endl;

        // Initialize the Apollo client
        ac::Opts opts;
        opts.cluster_name_ = cluster_name;
        opts.namespaces_ = namespaces;

        auto console_logger = std::make_shared<ConsoleLogger>();
        console_logger->setLogLevel(ac::LogLevel::Debug);
        auto notification_callback = [console_logger](const ac::NamespaceType& n,
                                                      const ac::Configures& olds,
                                                      const ac::Configures& news,
                                                      ac::Changes&& changes)
        {
            console_logger->log(ac::LogLevel::Info, "Configuration changed for namespace: " + n);
            for (auto ns : changes)
            {
                switch (ns.change_type_)
                {
                    case ac::ChangeType::Added:
                        console_logger->log(ac::LogLevel::Info, "namespace:" + n + " Added: " + ns.key_ + " = " + ns.value_);
                        break;
                    case ac::ChangeType::Updated:
                        console_logger->log(ac::LogLevel::Info, "namespace:" + n + " Modified: " + ns.key_ + " = " + ns.value_);
                        break;
                    case ac::ChangeType::Deleted:
                        console_logger->log(ac::LogLevel::Info, "namespace:" + n + " Deleted: " + ns.key_);
                        break;
                }
            }
        };
        auto notification_callback_ptr = std::make_shared<ac::NotificationCallback>(notification_callback);

        try
        {
            auto client = ac::makeApolloClient(apollo_url, app_id, std::move(opts), console_logger);
            client->setNotificationsListener(notification_callback_ptr);
            client->startLongPolling(poll_interval);
            std::cout << "Apollo client initialized and started long polling." << std::endl;

            for (auto& ns : namespaces)
            {
                auto configures = client->getConfigures(ns);
                std::cout << "Configurations for namespace '" << ns << "':" << std::endl;
                for (const auto& kv : configures)
                {
                    std::cout << "  " << kv.first << ": " << kv.second << std::endl;
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error initializing Apollo client: " << e.what() << std::endl;
            return 1;
        }

        // Keep the main thread alive using a simple blocking approach
        std::cout << "Press Ctrl+C to exit..." << std::endl;
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    catch (const po::error& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << desc << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}