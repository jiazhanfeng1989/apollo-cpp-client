#include "apollo_utility.h"
#include "apollo_internal.h"
#include "nlohmann/json.hpp"
#include <boost/url.hpp>

namespace apollo
{
namespace client
{
static constexpr auto notification_v2_path = "/notifications/v2";

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
    params.append({"appId", app_id});
    params.append({"cluster", cluster_name});
    params.append({"notifications", notifications_str});

    if (!label.empty())
    {
        params.append({"label", label});
    }
    return u.buffer();
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

}  // namespace client
}  // namespace apollo
