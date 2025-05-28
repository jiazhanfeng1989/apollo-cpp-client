#include "apollo_utility.h"
#include "apollo_internal.h"
#include "boost/url/encode.hpp"
#include "nlohmann/json.hpp"
#include <boost/url.hpp>
#include <sstream>
#include <string>

namespace apollo
{
namespace client
{
static constexpr auto notification_v2_path = "/notifications/v2";
static constexpr auto no_cache_configs_path = "/configs";

std::string createNotificationsV2URL(const std::string& app_id,
                                     const std::string& apollo_url,
                                     const std::string& cluster_name,
                                     const std::string& label,
                                     const NamespaceAttributesMap& namespace_attributes)
{
    Notifications notifications;
    notifications.reserve(namespace_attributes.size());
    for (const auto& p : namespace_attributes)
    {
        Notification notification;
        notification.namespace_name_ = p.first;
        notification.notification_id_ = p.second->GetNotificationId();
        notifications.push_back(notification);
    }

    std::string notifications_str = toJsonString(notifications);

    boost::urls::url u(apollo_url);
    u.set_path(notification_v2_path);

    auto params = u.params();
    params.append({"appId", boost::urls::encode(app_id, boost::urls::unreserved_chars)});
    params.append({"cluster", boost::urls::encode(cluster_name, boost::urls::unreserved_chars)});
    params.append({"notifications", boost::urls::encode(notifications_str, boost::urls::unreserved_chars)});

    if (!label.empty())
    {
        params.append({"label", boost::urls::encode(label, boost::urls::unreserved_chars)});
    }
    return u.buffer();
}

std::string createNoCacheConfigsURL(const std::string& app_id,
                                    const std::string& apollo_url,
                                    const std::string& cluster_name,
                                    const NamespaceType& s_namespace,
                                    const std::string& label,
                                    const std::string& release_key,
                                    int notification_id)
{
    boost::urls::url u(apollo_url);
    u.set_path(createNoCacheConfigsURLPath(app_id, cluster_name, s_namespace));

    auto params = u.params();
    if (!label.empty())
    {
        params.append({"label", boost::urls::encode(label, boost::urls::unreserved_chars)});
    }

    if (release_key.empty())
    {
        return u.buffer();
    }
    params.append({"releaseKey", boost::urls::encode(release_key, boost::urls::unreserved_chars)});

    if (notification_id == -1)
    {
        return u.buffer();
    }

    auto messages = createNoCacheConfigsMessages(app_id, cluster_name, s_namespace, notification_id);
    params.append({"messages", boost::urls::encode(messages, boost::urls::unreserved_chars)});

    return u.buffer();
}

std::string createNoCacheConfigsMessages(const std::string& app_id,
                                         const std::string& cluster_name,
                                         const NamespaceType& s_namespace,
                                         int notification_id)
{
    std::stringstream ss;
    ss << "{\"details\":{\"" << app_id << '+' << cluster_name << '+' << s_namespace
       << "\":" << notification_id << "}}";

    return ss.str();
}

std::string createNoCacheConfigsURLPath(const std::string& app_id,
                                        const std::string& cluster_name,
                                        const NamespaceType& s_namespace)
{
    std::stringstream ss;
    ss << no_cache_configs_path << "/" << app_id << "/" << cluster_name << "/" << s_namespace;
    return ss.str();
}

bool fromJsonString(const std::string& jsonString, Notification& notification)
{
    try
    {
        auto j = nlohmann::json::parse(jsonString);
        j.at("namespaceName").get_to(notification.namespace_name_);
        j.at("notificationId").get_to(notification.notification_id_);
        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

std::string toJsonString(const Notification& notification)
{
    nlohmann::json j = {{"namespaceName", notification.namespace_name_},
                        {"notificationId", notification.notification_id_}};
    return j.dump();
}

bool fromJsonString(const std::string& jsonString, Notifications& notifications)
{
    try
    {
        auto j = nlohmann::json::parse(jsonString);
        notifications.clear();

        for (const auto& item : j)
        {
            Notification notification;
            item.at("namespaceName").get_to(notification.namespace_name_);
            item.at("notificationId").get_to(notification.notification_id_);
            notifications.push_back(notification);
        }
        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

std::string toJsonString(const Notifications& notifications)
{
    nlohmann::json j = nlohmann::json::array();

    for (const auto& notification : notifications)
    {
        nlohmann::json item = {{"namespaceName", notification.namespace_name_},
                               {"notificationId", notification.notification_id_}};
        j.push_back(item);
    }

    return j.dump();
}

bool fromJsonString(const std::string& jsonString, std::string& release_key, Configures& configures)
{
    try
    {
        auto j = nlohmann::json::parse(jsonString);
        j.at("releaseKey").get_to(release_key);
        j.at("configurations").get_to(configures);
        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool isValidUrl(const std::string& url)
{
    auto u = boost::urls::parse_uri(url);
    if (u.has_error())
    {
        return false;
    }

    if (u->scheme() != "http")
    {
        return false;
    }

    if (!url.empty() && url.back() == '/')
    {
        return false;
    }

    return true;
}

Changes ConfiguresDiff(const Configures& old_config, const Configures& new_config)
{
    Changes changes;

    // Check for added or updated items
    for (const auto& pair : new_config)
    {
        auto it = old_config.find(pair.first);
        if (it == old_config.end())
        {
            // New item added
            changes.emplace_back(ChangeType::Added, pair.first, pair.second);
        }
        else if (it->second != pair.second)
        {
            // Existing item updated
            changes.emplace_back(ChangeType::Updated, pair.first, pair.second);
        }
    }

    // Check for deleted items
    for (const auto& pair : old_config)
    {
        if (new_config.find(pair.first) == new_config.end())
        {
            // Item deleted
            changes.emplace_back(ChangeType::Deleted, pair.first, pair.second);
        }
    }

    return changes;
}

}  // namespace client
}  // namespace apollo
