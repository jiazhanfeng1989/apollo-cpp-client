/**
 * @file apollo_client.h
 * @brief Apollo Configuration Center Client Library
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once

#include <memory>
#include "apollo_types.h"

namespace apollo
{
namespace client
{
class ApolloClient;
using ClientPtr = std::shared_ptr<ApolloClient>;
using NotificationCallback = std::function<void(const Changes& changes)>;

/**
 * @class ApolloClient
 * @brief Interface for Apollo configuration center client
 *
 * This interface allows applications to access and monitor configuration
 * data stored in Apollo Configuration Center.
 */
class ApolloClient
{
public:
    virtual ~ApolloClient() = default;

    /**
     * @brief Starts the client and loads initial configuration
     *
     * Loads configurations for all specified namespaces into cache and
     * starts a background thread for polling configuration updates.
     * If the polling interval is set to zero or negative, no background
     * polling will occur, and the configuration will never be updated.
     *
     * @return true if startup succeeded, false otherwise
     */
    virtual bool start() = 0;

    /**
     * @brief Stops the client and releases resources
     *
     * Stops the background polling thread if running and clears
     * the configuration cache.
     */
    virtual void stop() = 0;

    /**
     * @brief Retrieves configuration values from cache
     *
     * Returns configuration for the specified namespace.
     * The client must be started before calling this method.
     *
     * @param s_namespace The namespace to retrieve configurations from
     * @return A map of configuration key-value pairs
     *
     * @note Returns empty map if the namespace is not in the configured
     *       namespaces list or if the client hasn't been started.
     */
    virtual Configures getConfigures(const NAMESPACE_TYPE& s_namespace) = 0;

    /**
     * @brief Sets a callback for configuration change notifications
     *
     * When configurations change, the callback function will be invoked
     * with details about the changes.
     *
     * @param notificationCallback Callback to receive change notifications
     *
     * @note It's recommended to set the callback before calling start(), otherwise may miss some changes.
     *       The callback should be thread-safe as it may be called from a background thread.
     *       If the callback is not set, no notifications will be sent.
     *       This function can be called repeatedly to change the callback.
     */
    virtual void setNotificationsListener(std::weak_ptr<NotificationCallback> notificationCallback) = 0;
};

/**
 * @brief Creates a new Apollo client instance
 *
 * Factory function to create and configure an Apollo client.
 *
 * @param apollo_url Apollo server URL (e.g., "http://apollo-service:8080")
 * @param app_id Application ID registered in Apollo
 * @param opts Client options including cluster name, namespaces, and polling interval
 *
 * @return A shared pointer to the created Apollo client
 * @throws std::runtime_error If initialization fails
 * @note apollo_url does not support https currently.
 */
ClientPtr makeApolloClient(const std::string& apollo_url, const std::string& app_id, Opts&& opts = Opts());

}  // namespace client
}  // namespace apollo