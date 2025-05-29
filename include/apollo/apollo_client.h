/**
 * @file apollo_client.h
 * @brief Apollo Configuration Center Client Library
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once
#include "apollo_types.h"

namespace apollo
{
namespace client
{
class ApolloClient;
using ClientPtr = std::shared_ptr<ApolloClient>;
using NotificationCallback =
    std::function<void(const NamespaceType& n, const Configures& olds, const Configures& news, Changes&& changes)>;
using NotificationCallbackPtr = std::weak_ptr<NotificationCallback>;

/**
 * @class ApolloClient
 * @brief Interface for the Apollo configuration center client
 *
 * This interface allows applications to access and monitor configuration
 * data stored in Apollo Configuration Center. It provides methods for
 * retrieving configurations and receiving notifications when configuration
 * changes occur.
 */
class ApolloClient
{
public:
    /**
     * @brief Virtual destructor to ensure proper cleanup in derived classes
     */
    virtual ~ApolloClient() = default;

    /**
     * @brief Starts a long polling thread for configuration updates
     *
     * Starts a background thread that periodically polls the Apollo server for
     * configuration updates. When changes are detected, the notification callback
     * (if set) will be invoked from the background thread.
     *
     * @param long_polling_interval_ms Interval in milliseconds between polling requests.
     * If set to 0 or negative, long polling thread will not be started and configuration will not be updated.
     *
     * @note This method is non-blocking; it starts the polling thread and returns immediately.
     */
    virtual void startLongPolling(int long_polling_interval_ms = long_poller_interval_default) = 0;

    /**
     * @brief Stops the long polling thread
     *
     * Stops the background polling thread if it's running. After calling this method,
     * no further configuration updates will be received and no callbacks will be triggered.
     * This method blocks until the polling thread has terminated.
     */
    virtual void stopLongPolling() = 0;

    /**
     * @brief Retrieves configuration values from the ApolloClient cache
     *
     * Returns the configuration key-value pairs for the specified namespace.
     * The values are retrieved from the ApolloClient's local cache, which is updated
     * by the long polling thread.
     *
     * @param s_namespace The namespace to retrieve configurations from
     * @return A map of configuration key-value pairs
     *
     * @note Returns an empty map if the namespace is not in the configured namespaces list
     *       or if the namespace has no configurations. If long polling has not been started,
     *       this will return only the initial configuration loaded at ApolloClient creation.
     */
    virtual Configures getConfigures(const NamespaceType& s_namespace) = 0;

    /**
     * @brief Sets a callback for configuration change notifications
     *
     * Registers a callback function that will be invoked whenever configuration changes
     * are detected. The callback receives the namespace that changed, the old and new
     * configurations, and a detailed list of changes (added, modified, deleted items).
     *
     * @param notificationCallback A weak pointer to the callback function. Using a weak
     *        pointer ensures that the client doesn't hold a reference to an object that
     *        might be destroyed.
     *
     * @note It's recommended to set the callback before calling startLongPolling() to avoid
     *       missing any changes. The callback should be thread-safe as it's called from a
     *       background thread. This function can be called repeatedly to change the callback.
     */
    virtual void setNotificationsListener(NotificationCallbackPtr notificationCallback) = 0;
};

/**
 * @brief Creates a new Apollo client instance
 *
 * The method will connect to the Apollo server and load configurations for initialization when created.
 *
 * @param apollo_url Apollo server URL (e.g., "http://apollo-service:8080" or "http://apollo-server.com")
 * @param app_id Application ID registered in Apollo Configuration Center
 * @param opts Client options including cluster name, namespaces, and label
 * @param logger Optional logger for diagnostic messages (nullptr for no logging)
 *
 * @return A shared pointer to the created Apollo client instance
 * @throws std::runtime_error If initialization fails (e.g., invalid URL or app_id)
 *
 * @note HTTP is the only supported protocol; HTTPS is not currently supported.
 */
ClientPtr makeApolloClient(const std::string& apollo_url,
                           const std::string& app_id,
                           Opts&& opts = Opts(),
                           LoggerPtr logger = nullptr);

}  // namespace client
}  // namespace apollo