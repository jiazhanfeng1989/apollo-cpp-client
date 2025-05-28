#pragma once

#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <thread>
#include "apollo/apollo_client.h"
#include "apollo/apollo_types.h"
#include "apollo_internal.h"
#include "http_client.h"

namespace apollo
{
namespace client
{
class ApolloClientImpl : public ApolloClient, public std::enable_shared_from_this<ApolloClientImpl>
{
public:
    ApolloClientImpl(const std::string& apollo_url, const std::string& app_id, Opts&& opts, LoggerPtr&& logger_);
    ~ApolloClientImpl() override;
    void startLongPolling(int long_polling_interval_ms = long_poller_interval_default) override;
    void stopLongPolling() override;
    Configures getConfigures(const NamespaceType& s_namespace) override;
    void setNotificationsListener(NotificationCallbackPtr notificationCallback) override;

private:
    ApolloClientImpl(const ApolloClientImpl&) = delete;             // Disable copy constructor
    ApolloClientImpl& operator=(const ApolloClientImpl&) = delete;  // Disable assignment operator
    ApolloClientImpl(ApolloClientImpl&&) = delete;                  // Disable move constructor
    ApolloClientImpl& operator=(ApolloClientImpl&&) = delete;  // Disable move assignment operator

    void initNamespaceAttributes();  // throw std::runtime_error if namespace attributes initialized failed
    void initConfigurationsMap();  // throw std::runtime_error if configurations map initialized failed
    void initNotificationsIdMap();  // throw std::runtime_error if notificationsId map initialized failed
    void longPollingThreadFunc();
    void setupLongPollingTimer();

private:
    int long_polling_interval_;
    std::atomic<bool> long_polling_running_{false};
    std::string app_id_;
    std::string apollo_url_;
    Opts opts_;
    NamespaceAttributesMap namespace_attributes_;
    LoggerPtr logger_;
    NotificationCallbackPtr notification_callback_;
    std::thread long_polling_thread_;
    boost::asio::io_context io_context_;
    net::steady_timer long_polling_timer_;
    HttpClient http_client_;
};
}  // namespace client
}  // namespace apollo