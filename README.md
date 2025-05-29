# Apollo C++ Client

# Overview
A C++ Client library for the [Apollo](https://github.com/apolloconfig/apollo) configuration center.

## Features
- Support for Apollo long polling and notification mechanisms
- Support for Apollo configuration updates
- Support for Apollo gray release by label
- Thread-safe access to configuration values

## TODO Features
- Implement HTTPS support.

## Prerequisites
- C++14 compatible compiler
- CMake 3.11 or higher
- Boost 1.84 or higher, using the following Boost libraries:
  - boost::asio for networking
  - boost::url for URL parsing
- nlohmann_json 3.11.2 or higher
- doctest 2.4 or higher (for testing, optional)
- Doxygen 1.14.0 or higher (for documentation generation, optional)

## Building
```bash
mkdir build && cd build

# This flag generates a `compile_commands.json` file for integration with tools like clangd.
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# Use `make -jN` to speed up the build process, where `N` is the number of parallel jobs.
make -j4

# Now build the docs target, which generates the documentation.
# This requires Doxygen version 1.14.0 to be installed on your system.
# If you don't have Doxygen installed, you can skip this step.
# ref: https://github.com/doxygen/doxygen
# docs output will be built in the build/docs directory
cmake --build . --target docs
```

## Usage
```c++
#include "apollo/apollo_client.h"

auto notification_callback_ptr = std::make_shared<apollo::client::NotificationCallback>(
    [](const apollo::client::NamespaceType& n,
        const apollo::client::Configures& olds,
        const apollo::client::Configures& news,
        apollo::client::Changes&& changes)
    {
        std::cout << "configuration changed for namespace: " << n << std::endl;
        for (auto ns : changes)
        {
            switch (ns.change_type_)
            {
                case apollo::client::ChangeType::Added:
                    std::cout << "namespace:" << n << " Added: " << ns.key_ << " = "
                                << ns.value_ << std::endl;
                    break;
                case apollo::client::ChangeType::Updated:
                    std::cout << "namespace:" << n << " Modified: " << ns.key_ << " = "
                                << ns.value_ << std::endl;
                    break;
                case apollo::client::ChangeType::Deleted:
                    std::cout << "namespace:" << n << " Deleted: " << ns.key_ << std::endl;
                    break;
            }
        }
    });

apollo::client::ClientPtr client;

apollo::client::Opts opts;
opts.cluster_name_ = "cluster";                    // Apollo cluster name
opts.namespaces_ = {"config1", "config2"};         // Apollo namespaces to subscribe to
std::string app_id = "test";                       // Apollo application ID
std::string apollo_url = "http://localhost:8080";  // Apollo server URL

try
{
    // Create an Apollo client instance with the specified options.
    // The method will connect to the Apollo server and load configurations for initialization when created.
    // Throws std::runtime_error If initialization fails (e.g., invalid URL or app_id, network issues).
    // client should be hold by caller, otherwise it will be destroyed immediately.
    client = apollo::client::makeApolloClient(apollo_url, app_id, std::move(opts));

    // Set the notification callback to handle configuration changes.
    client->setNotificationsListener(notification_callback_ptr);

    // Starts a background thread that periodically polls the Apollo server for configuration updates.
    // This method is non-blocking; it starts the polling thread and returns immediately.
    client->startLongPolling();
}
catch (const std::exception& e)
{
    throw std::runtime_error("Error initializing Apollo client: " + std::string(e.what()));
}

// now the client is running and listening for configuration changes,you can use it anywhere in your application,and client is thread-safe.


// Get a configuration from the specified namespace.
// Returns an empty map if the namespace is not in the configured namespaces list
// If long polling has not been started, this will return only the initial configuration loaded at ApolloClient creation.
// no exception is thrown.
auto configures = client->getConfigures("config1");
...
...
...
client->stopLongPolling(); // Stop the long polling thread.
```

# 💡 Tips
- [boost usage](./docs/boost_usage.md) provides detailed instructions on how to use Boost libraries with this project.
- [nlohmann_json](./docs/nlohmann_json_usage.md) provides detailed instructions on how to use nlohmann_json with this project.
- [doctest usage](./docs/doctest_usage.md) provides detailed instructions on how to use doctest for unit testing in this project.
