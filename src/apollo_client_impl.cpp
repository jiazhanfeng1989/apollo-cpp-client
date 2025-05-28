#include <boost/asio.hpp>
#include "apollo_client_impl.h"
#include "apollo_internal.h"
#include "apollo_utility.h"

namespace apollo
{
namespace client
{
ApolloClientImpl::ApolloClientImpl(const std::string& apollo_url, const std::string& app_id, Opts&& opts, LoggerPtr&& logger)
    : long_polling_interval_(0)
    , long_polling_running_(false)
    , app_id_(app_id)
    , apollo_url_(apollo_url)
    , opts_(std::move(opts))
    , namespace_attributes_()
    , logger_(std::move(logger))
    , long_polling_thread_()
    , io_context_()
    , long_polling_timer_(io_context_)
    , http_client_(io_context_)
{
    http_client_.setConnectionTimeout(opts_.connection_timeout_ms_);
    http_client_.setRequestReadTimeout(opts_.request_read_timeout_ms_);
    http_client_.setRequestWriteTimeout(opts_.request_write_timeout_ms_);

    initNamespaceAttributes();
    initConfigurationsMap();
    initNotificationsIdMap();
}

ApolloClientImpl::~ApolloClientImpl()
{
    stopLongPolling();

    if (!io_context_.stopped())
    {
        io_context_.stop();
    }
}

void ApolloClientImpl::startLongPolling(int long_polling_interval_ms)
{
    if (long_polling_interval_ms <= 0)
    {
        return;
    }

    bool expected = false;
    if (long_polling_running_.compare_exchange_strong(expected, true))
    {
        long_polling_interval_ = long_polling_interval_ms;
        setupLongPollingTimer();

        long_polling_thread_ =
            std::thread([shared_this = shared_from_this()]() { shared_this->io_context_.run(); });
        LOG_INFO(logger_, "Starting long polling with interval: " + std::to_string(long_polling_interval_ms) + " ms");
    }

    return;
}

void ApolloClientImpl::stopLongPolling()
{
    bool expected = true;
    if (long_polling_running_.compare_exchange_strong(expected, false))
    {
        if (long_polling_thread_.joinable())
        {
            io_context_.stop();
            long_polling_thread_.join();
        }
    }
    return;
}

Configures ApolloClientImpl::getConfigures(const NamespaceType& s_namespace)
{
    assert(namespace_attributes_.size() > 0);
    if (namespace_attributes_.find(s_namespace) == namespace_attributes_.end())
    {
        return {};  // Return empty map if namespace is not configured
    }

    return namespace_attributes_[s_namespace]->GetConfigures();
}

void ApolloClientImpl::setNotificationsListener(NotificationCallbackPtr notificationCallback)
{
    notification_callback_ = notificationCallback;
}

void ApolloClientImpl::initNamespaceAttributes()
{
    for (const auto& ns : opts_.namespaces_)
    {
        if (ns == "")
        {
            throw std::invalid_argument("Namespace cannot be empty in opts.namespaces_");
        }

        namespace_attributes_.emplace(ns, std::make_shared<NamespaceAttributes>());
    }
}

void ApolloClientImpl::initConfigurationsMap()
{
    assert(namespace_attributes_.size() > 0);
    for (auto& p : namespace_attributes_)
    {
        auto url = createNoCacheConfigsURL(app_id_,
                                           apollo_url_,
                                           opts_.cluster_name_,
                                           p.first,
                                           opts_.label_,
                                           p.second->GetReleaseKey(),
                                           p.second->GetNotificationId());

        LOG_INFO(logger_, "get configurations from Apollo, namespace: " + p.first + ", url: " + url);
        auto res = http_client_.get(url);
        if (res.second)
        {
            throw std::runtime_error("Failed to fetch configurations from Apollo: " + res.second.message());
        }

        if (res.first.result() != http::status::ok)
        {
            throw std::runtime_error("Failed to fetch configurations from Apollo, status: " +
                                     std::to_string(res.first.result_int()));
        }

        std::string release_key;
        Configures configures;

        if (!fromJsonString(res.first.body(), release_key, configures))
        {
            throw std::runtime_error("Failed to parse configurations from Apollo response");
        }
        p.second->SetReleaseKey(std::move(release_key));
        p.second->SetConfigures(std::move(configures));
        LOG_INFO(logger_, "get configurations from Apollo successfully, namespace:" + p.first);
    }
}

void ApolloClientImpl::initNotificationsIdMap()
{
    assert(namespace_attributes_.size() > 0);
    auto url = createNotificationsV2URL(app_id_, apollo_url_, opts_.cluster_name_, opts_.label_, namespace_attributes_);

    auto res = http_client_.get(url);

    LOG_INFO(logger_, "init notifications map from Apollo url: " + url);
    if (res.second)
    {
        throw std::runtime_error("Failed to fetch notifications from Apollo: " + res.second.message());
    }

    if (res.first.result() != http::status::ok)
    {
        throw std::runtime_error("Failed to fetch notifications from Apollo, status: " +
                                 std::to_string(res.first.result_int()));
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

    LOG_INFO(logger_, "init notifications map from Apollo successfully");
}

void ApolloClientImpl::longPollingThreadFunc()
{
    assert(namespace_attributes_.size() > 0);
    auto url = createNotificationsV2URL(app_id_, apollo_url_, opts_.cluster_name_, opts_.label_, namespace_attributes_);
    LOG_DEBUG(logger_, "long polling url: " + url);

    auto res = http_client_.get(url);
    if (res.second)
    {
        LOG_WARN(logger_, "long polling failed, url: " + url + " message: " + res.second.message());
        setupLongPollingTimer();
        return;
    }

    if (res.first.result() == http::status::not_modified)
    {
        setupLongPollingTimer();
        return;
    }

    if (res.first.result() != http::status::ok)
    {
        LOG_WARN(logger_,
                 "long polling failed, url: " + url + " status: " + std::to_string(res.first.result_int()));
        setupLongPollingTimer();
        return;
    }

    Notifications notifications;
    if (!fromJsonString(res.first.body(), notifications))
    {
        LOG_WARN(logger_, "long polling parse notifications failed, url: " + url);
        setupLongPollingTimer();
        return;
    }

    for (const auto& notification : notifications)
    {
        auto attribute_it = namespace_attributes_.find(notification.namespace_name_);
        if (attribute_it == namespace_attributes_.end())
        {
            continue;
        }

        auto no_cache_url = createNoCacheConfigsURL(app_id_,
                                                    apollo_url_,
                                                    opts_.cluster_name_,
                                                    notification.namespace_name_,
                                                    opts_.label_,
                                                    attribute_it->second->GetReleaseKey(),
                                                    notification.notification_id_);
        auto no_cache_res = http_client_.get(no_cache_url);
        if (no_cache_res.second)
        {
            LOG_WARN(logger_, "update configuration failed, url: " + no_cache_url);
            continue;
        }

        if (no_cache_res.first.result() != http::status::ok)
        {
            continue;
        }

        std::string new_release_key;
        Configures new_configures;

        if (!fromJsonString(no_cache_res.first.body(), new_release_key, new_configures))
        {
            LOG_WARN(logger_, "parse configures failed, url: " + no_cache_url);
            continue;
        }

        auto old_configures = attribute_it->second->GetConfigures();

        auto callback = notification_callback_.lock();
        if (callback)
        {
            safeCall(*callback,
                     notification.namespace_name_,
                     old_configures,
                     new_configures,
                     ConfiguresDiff(old_configures, new_configures));
        }

        attribute_it->second->SetReleaseKey(std::move(new_release_key));
        attribute_it->second->SetConfigures(std::move(new_configures));
        attribute_it->second->SetNotificationId(notification.notification_id_);
    }

    setupLongPollingTimer();
    return;
}

void ApolloClientImpl::setupLongPollingTimer()
{
    long_polling_timer_.expires_after(std::chrono::milliseconds(long_polling_interval_));
    long_polling_timer_.async_wait(
        [shared_this = shared_from_this()](const boost::system::error_code& ec)
        {
            if (ec == boost::asio::error::operation_aborted)
            {
                // Timer was cancelled, likely due to io_context stopping
                return;
            }

            if (shared_this->long_polling_running_)
            {
                shared_this->longPollingThreadFunc();
            }
        });
}

}  // namespace client
}  // namespace apollo