#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <atomic>
#include <map>
#include <memory>
#include "apollo/apollo_types.h"

namespace apollo
{
namespace client
{

struct Notification
{
    std::string namespace_name_;
    int notification_id_;
};
using Notifications = std::vector<Notification>;

class NamespaceAttributes
{
public:
    NamespaceAttributes(const std::string& release_key = "", int initial_notification_id = -1)
        : release_key_mutex_()
        , release_key_(release_key)
        , notification_id_(initial_notification_id)
    {
    }
    ~NamespaceAttributes() = default;

    inline const std::string& GetReleaseKey() const
    {
        std::unique_lock<std::mutex> lock(release_key_mutex_);
        return release_key_;
    }

    inline void SetConfigures(Configures&& configures)
    {
        std::unique_lock<std::mutex> lock(configures_mutex_);
        configures_ = std::move(configures);
    }

    inline Configures GetConfigures() const
    {
        std::unique_lock<std::mutex> lock(configures_mutex_);
        return configures_;
    }

    inline void SetReleaseKey(std::string&& release_key)
    {
        std::unique_lock<std::mutex> lock(release_key_mutex_);
        release_key_ = std::move(release_key);
    }

    inline int GetNotificationId() const
    {
        // Using std::memory_order_relaxed here because no synchronization or ordering with other threads is required.
        return notification_id_.load(std::memory_order_relaxed);
    }

    inline void SetNotificationId(int notification_id)
    {
        // Using std::memory_order_relaxed here because the notification_id_ is only used
        // for atomic updates and does not require synchronization with other variables.
        notification_id_.store(notification_id, std::memory_order_relaxed);
    }

private:
    std::string release_key_;               // The release key for the namespace
    mutable std::mutex release_key_mutex_;  // Mutex to protect access to the release key
    Configures configures_;                 // Configuration key-value pairs for the namespace
    mutable std::mutex configures_mutex_;   // Mutex to protect access to the configuration
    std::atomic_int notification_id_;       // Atomic notification ID for thread-safe access
};

using NamespaceAttributesPtr = std::shared_ptr<NamespaceAttributes>;
using NamespaceAttributesMap = std::map<NamespaceType, NamespaceAttributesPtr>;

}  // namespace client
}  // namespace apollo