/**
 * @file apollo_types.h
 * @brief Apollo Client Library Type Definitions
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once

#include <map>
#include <memory>
#include <vector>
#include <string>

namespace apollo
{
namespace client
{

constexpr auto long_poller_interval_default = 1000; /**< Default long poller interval in milliseconds */

/** @brief Type alias for namespace identifiers */
using NamespaceType = std::string;

/**
 * @enum ChangeType
 * @brief Enum class representing the type of change in configuration
 */
enum class ChangeType
{
    Added,   /**< A new configuration item has been added */
    Updated, /**< An existing configuration item has been updated */
    Deleted  /**< An existing configuration item has been deleted */
};

/**
 * @struct Change
 * @brief Structure representing a change in configuration
 */
struct Change
{
    ChangeType change_type_; /**< The type of change (add, update, delete) */
    std::string key_;        /**< The key of the configuration item */
    std::string value_;      /**< The value of the configuration item */

    Change(ChangeType type, std::string&& key, std::string&& value)
        : change_type_(type)
        , key_(std::move(key))
        , value_(std::move(value))
    {
    }

    Change(ChangeType type, const std::string& key, const std::string& value)
        : change_type_(type)
        , key_(key)
        , value_(value)
    {
    }
    ~Change() = default;
};

/** @brief List of configuration changes */
using Changes = std::vector<Change>;

/** @brief Map of key-value pairs representing a namespace's configuration */
using Configures = std::map<std::string, std::string>;

/**
 * @struct Opts
 * @brief Options for configuring the Apollo client
 * @note Please ensure that the request_read_timeout_ms_ to the server is greater than 60 seconds for long polling to work correctly.
 */
struct Opts
{
    std::string cluster_name_ = "default"; /**< The cluster name */
    std::string label_ = "";               /**< The label for the configuration */
    std::vector<NamespaceType> namespaces_ = {"application"}; /**< The namespaces to subscribe to */
    int connection_timeout_ms_ = 500; /**< The timeout for establishing a connection in milliseconds */
    int request_read_timeout_ms_ = 120000; /**< The timeout for read HTTP response in milliseconds*/
    int request_write_timeout_ms_ = 3000;  /**< The timeout for sent HTTP request in milliseconds */
};

enum class LogLevel
{
    Disabled,
    Error,
    Warning,
    Info,
    Debug,
};

class ILogger
{
public:
    virtual ~ILogger() = default;

    virtual LogLevel getLogLevel() const = 0;
    virtual void setLogLevel(LogLevel level) = 0;
    virtual void log(LogLevel level, const std::string& message) = 0;
};
using LoggerPtr = std::shared_ptr<ILogger>;

}  // namespace client
}  // namespace apollo