#pragma once

#include "apollo/apollo_types.h"
#include "apollo_internal.h"

namespace apollo
{
namespace client
{

bool fromJsonString(const std::string& jsonString, Notification& notification);
std::string toJsonString(const Notification& notification);
bool fromJsonString(const std::string& jsonString, Notifications& notifications);
std::string toJsonString(const Notifications& notifications);
bool fromJsonString(const std::string& jsonString, std::string& release_key, Configures& configures);

std::string createNotificationsV2URL(const std::string& app_id,
                                     const std::string& apollo_url,
                                     const std::string& cluster_name,
                                     const std::string& label,
                                     const NamespaceAttributesMap& namespace_attributes);

std::string createNoCacheConfigsURL(const std::string& app_id,
                                    const std::string& apollo_url,
                                    const std::string& cluster_name,
                                    const NamespaceType& s_namespace,
                                    const std::string& label,
                                    const std::string& release_key,
                                    int notification_id);

std::string createNoCacheConfigsURLPath(const std::string& app_id,
                                        const std::string& cluster_name,
                                        const NamespaceType& s_namespace);

std::string createNoCacheConfigsMessages(const std::string& app_id,
                                         const std::string& cluster_name,
                                         const NamespaceType& s_namespace,
                                         int notification_id);

Changes ConfiguresDiff(const Configures& old_config, const Configures& new_config);

bool isValidUrl(const std::string& url);

template <class F, typename... Args>
void safeCall(F& func, Args&&... args)
{
    if (func)
    {
        try
        {
            func(std::forward<Args>(args)...);
        }
        catch (std::exception& e)
        {
        }
    }
}

#define APOLLO_CLIENT_IMPL_LOG(logger, level, message) \
    {                                                  \
        if (logger && logger->getLogLevel() >= level)  \
        {                                              \
            logger->log(level, message);               \
        }                                              \
    }

#define LOG_ERROR(logger, message) APOLLO_CLIENT_IMPL_LOG(logger, LogLevel::Error, message)
#define LOG_WARN(logger, message) APOLLO_CLIENT_IMPL_LOG(logger, LogLevel::Warning, message)
#define LOG_INFO(logger, message) APOLLO_CLIENT_IMPL_LOG(logger, LogLevel::Info, message)
#define LOG_DEBUG(logger, message) APOLLO_CLIENT_IMPL_LOG(logger, LogLevel::Debug, message)
}  // namespace client
}  // namespace apollo

#include <string>