#pragma once

#include "apollo_internal.h"

namespace apollo
{
namespace client
{

bool fromJsonString(const std::string& jsonString, Notification& notification);
std::string toJsonString(const Notification& notification);
bool fromJsonString(const std::string& jsonString, Notifications& notifications);
std::string toJsonString(const Notifications& notifications);

std::string createNotificationsV2URL(const std::string& app_id,
                                     const std::string& apollo_url,
                                     const std::string& cluster_name,
                                     const std::string& label,
                                     const NamespaceAttributesMap& namespace_attributes);

}  // namespace client
}  // namespace apollo

#include <string>