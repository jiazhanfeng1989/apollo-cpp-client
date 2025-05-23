#include <boost/asio.hpp>
#include "apollo_client_impl.h"
#include "apollo_internal.h"
#include "apollo_utility.h"

namespace apollo
{
namespace client
{
ApolloClientImpl::ApolloClientImpl(const std::string& apollo_url, const std::string& app_id, Opts&& opts = Opts())
    : apollo_url_(apollo_url)
    , app_id_(app_id)
    , opts_(std::move(opts))
    , namespace_attributes_()
    , io_context_()
    , http_client_(io_context_)
{
    auto u = boost::urls::parse_uri(apollo_url_);
    if (u.has_error())
    {
        throw std::invalid_argument("Invalid apollo_url: " + apollo_url_);
    }

    if (u->scheme() != "http")
    {
        throw std::invalid_argument("apollo_url must start with http://");
    }

    for (const auto& ns : opts_.namespaces_)
    {
        namespace_attributes_.emplace(ns, std::make_shared<NamespaceAttributes>());
    }

    http_client_.setConnectionTimeout(opts_.connection_timeout_ms_);
    http_client_.setRequestReadTimeout(opts_.request_read_timeout_ms_);
    http_client_.setRequestWriteTimeout(opts_.request_write_timeout_ms_);
}

ApolloClientImpl::~ApolloClientImpl()
{
}

bool ApolloClientImpl::start()
{
    // Implementation for starting the Apollo client
    // This is a placeholder; actual implementation will depend on the Apollo server API
    return true;
}

void ApolloClientImpl::stop()
{
    // Implementation for stopping the Apollo client
    // This is a placeholder; actual implementation will depend on the Apollo server API
}
Configures ApolloClientImpl::getConfigures(const NAMESPACE_TYPE& s_namespace)
{
    assert(namespace_attributes_.size() > 0);
    if (namespace_attributes_.find(s_namespace) == namespace_attributes_.end())
    {
        return {};  // Return empty map if namespace is not configured
    }

    return namespace_attributes_[s_namespace]->GetConfigures();
}

void ApolloClientImpl::setNotificationsListener(std::weak_ptr<NotificationCallback> notificationCallback)
{
    notification_callback_ = std::move(notificationCallback);
}

Configures ApolloClientImpl::getConfiguresFromApolloNoCache(const NAMESPACE_TYPE& s_namespace)
{
    assert(namespace_attributes_.size() > 0);
    // Implementation for fetching configuration from Apollo server without cache
    // This is a placeholder; actual implementation will depend on the Apollo server API
    return {};
}

Notifications ApolloClientImpl::getNotificationsFromApollo()
{
    assert(namespace_attributes_.size() > 0);
    auto notification_v2_url =
        createNotificationsV2URL(app_id_, apollo_url_, opts_.cluster_name_, opts_.label_, namespace_attributes_);

    auto res = http_client_.get(notification_v2_url);

    if (res.second)
    {
        throw std::runtime_error("Failed to fetch notifications from Apollo: " + res.second.message());
    }

    if (res.first.result() != http::status::ok && res.first.result() != http::status::not_modified)
    {
        throw std::runtime_error("Failed to fetch notifications from Apollo, status: " +
                                 std::to_string(res.first.result_int()));
    }

    if (res.first.result() == http::status::not_modified)
    {
        return {};
    }

    Notifications notifications;
    if (!fromJsonString(res.first.body(), notifications))
    {
        throw std::runtime_error("Failed to parse notifications from Apollo response");
    }

    for (const auto& notification : notifications)
    {
        if (namespace_attributes_.find(notification.namespace_name_) != namespace_attributes_.end())
        {
            namespace_attributes_[notification.namespace_name_]->SetNotificationId(notification.notification_id_);
        }
    }
    return notifications;
}

}  // namespace client
}  // namespace apollo