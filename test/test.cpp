#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "http_client.h"
#include <boost/asio.hpp>
#include "apollo_utility.h"

using namespace apollo::client;
TEST_CASE("httpclient-sync-get")
{
    boost::asio::io_context io_context;
    HttpClient client(io_context);
    std::string url = "http://httpbin.org/get";

    auto result = client.get(url);
    CHECK(!result.second);
    CHECK(result.first.result() == http::status::ok);
}

TEST_CASE("httpclient-not-support-ssl")
{
    boost::asio::io_context io_context;
    HttpClient client(io_context);
    std::string url = "https://httpbin.org/get";

    auto result = client.get(url);
    CHECK(result.second);
    CHECK(result.second.value() == boost::asio::error::no_protocol_option);
    CHECK(result.second.message() == "Protocol not available");
}

TEST_CASE("httpclient-invalid-url")
{
    boost::asio::io_context io_context;
    HttpClient client(io_context);
    std::string url = "invalid-url";

    auto result = client.get(url);
    CHECK(result.second);
    CHECK(result.second.value() == boost::asio::error::invalid_argument);
    CHECK(result.second.message() == "Invalid argument");
}

TEST_CASE("httpclient-invalid-host")
{
    boost::asio::io_context io_context;
    HttpClient client(io_context);
    std::string url = "http://invalid-host:12345";

    auto result = client.get(url);
    CHECK(result.second);
    CHECK(result.second.value() == boost::asio::error::host_unreachable);
    std::cout << result.second.message() << std::endl;
    CHECK(result.second.message() == "No route to host");
}

TEST_CASE("httpclient-async-get")
{
    boost::asio::io_context io_context;
    HttpClient client(io_context);
    std::string url = "http://httpbin.org/get";

    bool callback_called = false;
    client.getAsync(url,
                    [&](beast::error_code ec, http::response<http::string_body> res)
                    {
                        CHECK(!ec);
                        CHECK(res.result() == http::status::ok);
                        callback_called = true;
                    });

    io_context.run();
    CHECK(callback_called);
}

TEST_CASE("notification-to-json")
{
    Notification notification;
    notification.namespace_name_ = "test_namespace";
    notification.notification_id_ = 123;
    std::string json = toJsonString(notification);
    CHECK(json == R"({"namespaceName":"test_namespace","notificationId":123})");
}

TEST_CASE("notification-from-json")
{
    std::string json = R"({"namespaceName":"test_namespace","notificationId":123})";
    Notification notification;
    bool success = fromJsonString(json, notification);
    CHECK(success);
    CHECK(notification.namespace_name_ == "test_namespace");
    CHECK(notification.notification_id_ == 123);
}

TEST_CASE("notifications-to-json")
{
    Notifications notifications = {{"test_namespace1", 1}, {"test_namespace2", 2}};
    std::string json = toJsonString(notifications);
    CHECK(json == R"([{"namespaceName":"test_namespace1","notificationId":1},{"namespaceName":"test_namespace2","notificationId":2}])");
}

TEST_CASE("notifications-from-json")
{
    std::string json = R"([{"namespaceName":"test_namespace1","notificationId":1},{"namespaceName":"test_namespace2","notificationId":2}])";
    Notifications notifications;
    bool success = fromJsonString(json, notifications);
    CHECK(success);
    CHECK(notifications.size() == 2);
    CHECK(notifications[0].namespace_name_ == "test_namespace1");
    CHECK(notifications[0].notification_id_ == 1);
    CHECK(notifications[1].namespace_name_ == "test_namespace2");
    CHECK(notifications[1].notification_id_ == 2);
}

TEST_CASE("create-notifications-v2-url")
{
    NamespaceAttributesMap namespace_attributes =
        {{"test_namespace1", std::make_shared<NamespaceAttributes>("test_release_key1", -1)},
         {"test_namespace2", std::make_shared<NamespaceAttributes>("", -1)}};

    std::string apollo_url = "http://apollo-server.com";
    std::string cluster_name = "default";
    std::string label = "test_label";
    std::string app_id = "test_app";

    auto url = createNotificationsV2URL(app_id, apollo_url, cluster_name, label, namespace_attributes);
    auto expected_url = R"(http://apollo-configservice-na.stg.k8s.mypna.com/notifications/v2?appId=NAVConfig&cluster=EU&notifications=%5B%7B%22namespaceName%22:%22direction%22,%22notificationId%22:-1%7D,%7B%22namespaceName%22:%22evplanner%22,%22notificationId%22:-1%7D%5D)";
    CHECK(url == expected_url);
}
