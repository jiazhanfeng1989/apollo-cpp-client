#include "apollo/apollo_client.h"
#include "apollo_client_impl.h"
#include "apollo_utility.h"

namespace apollo
{
namespace client
{
ClientPtr makeApolloClient(const std::string& apollo_url, const std::string& app_id, Opts&& opts, LoggerPtr LoggerPtr)
{
    if (!isValidUrl(apollo_url))
    {
        throw std::invalid_argument("apollo client apollo_url format not supported: " + apollo_url);
    }

    if (app_id.empty())
    {
        throw std::invalid_argument("apollo client app_id cannot be empty");
    }

    if (opts.namespaces_.empty())
    {
        throw std::invalid_argument("apollo client at least one namespace must be specified in opts.namespaces_");
    }

    for (const auto& ns : opts.namespaces_)
    {
        if (ns.empty())
        {
            throw std::invalid_argument("apollo client namespace cannot be empty in opts.namespaces_");
        }
    }

    if (opts.cluster_name_.empty())
    {
        throw std::invalid_argument("apollo client cluster name cannot be empty in opts");
    }

    if (opts.connection_timeout_ms_ <= 0)
    {
        throw std::invalid_argument("apollo client connection timeout must be greater than 0 in opts");
    }

    if (opts.request_read_timeout_ms_ <= 60000)  // 60 seconds
    {
        throw std::invalid_argument("apollo client request read timeout must be greater than 60 seconds in opts");
    }

    if (opts.request_write_timeout_ms_ <= 0)
    {
        throw std::invalid_argument("apollo client request write timeout must be greater than 0 in opts");
    }
    return std::make_shared<ApolloClientImpl>(apollo_url, app_id, std::move(opts), std::move(LoggerPtr));
}
}  // namespace client
}  // namespace apollo