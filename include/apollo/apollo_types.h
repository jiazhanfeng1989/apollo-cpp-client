/**
 * @file apollo_types.h
 * @brief Apollo Client Library Type Definitions
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once

#include <map>
#include <list>
#include <vector>
#include <string>

namespace apollo
{
namespace client
{

/** @brief Type alias for configuration keys */
using KEY_TYPE = std::string;

/** @brief Type alias for configuration values */
using VALUE_TYPE = std::string;

/** @brief Type alias for namespace identifiers */
using NAMESPACE_TYPE = std::string;

/**
 * @enum ChangeType
 * @brief Enum class representing the type of change in configuration
 */
enum class ChangeType
{
    ChangeTypeAdd,    /**< A new configuration item has been added */
    ChangeTypeUpdate, /**< An existing configuration item has been updated */
    ChangeTypeDelete  /**< An existing configuration item has been deleted */
};

/**
 * @struct Change
 * @brief Structure representing a change in configuration
 */
struct Change
{
    ChangeType change_type; /**< The type of change (add, update, delete) */
    KEY_TYPE key;           /**< The key of the configuration item */
    VALUE_TYPE value;       /**< The value of the configuration item */
};

/** @brief List of configuration changes */
using Changes = std::list<Change>;

/** @brief Map of key-value pairs representing a namespace's configuration */
using Configures = std::map<KEY_TYPE, VALUE_TYPE>;

/**
 * @struct Opts
 * @brief Options for configuring the Apollo client
 * @note Please ensure that the request_read_timeout_ms_ to the server is greater than 60 seconds for long polling to work correctly.
 */
struct Opts
{
    std::string cluster_name_ = "default"; /**< The cluster name */
    std::string label_ = "";               /**< The label for the configuration */
    int long_poller_interval_ = 1000;      /**< The interval for long polling in milliseconds */
    std::vector<std::string> namespaces_ = {"application"}; /**< The namespaces to subscribe to */
    int connection_timeout_ms_ = 500; /**< The timeout for establishing a connection in milliseconds */
    int request_read_timeout_ms_ = 120000; /**< The timeout for read HTTP response in milliseconds*/
    int request_write_timeout_ms_ = 3000;  /**< The timeout for sent HTTP request in milliseconds */
};

}  // namespace client
}  // namespace apollo