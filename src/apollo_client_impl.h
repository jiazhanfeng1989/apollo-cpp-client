#pragma once

#include <memory>
#include <string>
#include <boost/asio.hpp>
#include "apollo/apollo_client.h"
#include "apollo/apollo_types.h"
#include "apollo_internal.h"
#include "http_client.h"

namespace apollo
{
namespace client
{
class ApolloClientImpl : public ApolloClient
{
public:
    ApolloClientImpl(const std::string& apollo_url, const std::string& app_id, Opts&& opts);
    ~ApolloClientImpl() override;
    bool start() override;
    void stop() override;
    Configures getConfigures(const NAMESPACE_TYPE& s_namespace) override;
    void setNotificationsListener(std::weak_ptr<NotificationCallback> notificationCallback) override;

private:
    ApolloClientImpl(const ApolloClientImpl&) = delete;             // Disable copy constructor
    ApolloClientImpl& operator=(const ApolloClientImpl&) = delete;  // Disable assignment operator
    ApolloClientImpl(ApolloClientImpl&&) = delete;                  // Disable move constructor
    ApolloClientImpl& operator=(ApolloClientImpl&&) = delete;  // Disable move assignment operator

    Notifications getNotificationsFromApollo();
    Configures getConfiguresFromApolloNoCache(const NAMESPACE_TYPE& s_namespace);

private:
    std::string app_id_;
    std::string apollo_url_;
    Opts opts_;
    NamespaceAttributesMap namespace_attributes_;
    std::weak_ptr<NotificationCallback> notification_callback_;
    boost::asio::io_context io_context_;
    HttpClient http_client_;
};
}  // namespace client
}  // namespace apollo