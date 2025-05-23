#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    // Command line options
    std::string apollo_ulr;
    std::string app_id;
    std::string cluster_name = "default";
    std::vector<std::string> namespaces = {"application"};  // Default namespace
    int poll_interval = 1000;

    // Set up command line options
    po::options_description desc("Apollo C++ Client Demo Options");

    // clang-format off
    desc.add_options()
    ("help,h", "Print help message")
    ("url,u", po::value<std::string>(&apollo_ulr)->required(), "Apollo server url (required)")
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
        std::cout << "Apollo Server: " << apollo_ulr << std::endl;
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
            std::cout << "Warning: Polling interval is less than 1000 ms, this may lead to high load on the server." << std::endl;
        }
        std::cout << "Poll Interval: " << poll_interval << " ms" << std::endl;
        std::cout << "======================" << std::endl;

        // Create IO context
        boost::asio::io_context io_context;

        // TODO: Initialize Apollo client with these parameters
        // auto client = apollo::client::makeApolloClient(io_context, apollo_ulr, app_id, [&](apollo::client::Opts& opts) {
        //     opts.cluster_name_ = cluster_name;
        //     opts.long_poller_interval_ = poll_interval;
        // });

        std::cout << "Client initialized successfully!" << std::endl;
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