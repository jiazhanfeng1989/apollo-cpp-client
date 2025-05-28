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

TEST_CASE("releaseKey-configures-from-json")
{
    std::string json = R"({"appId":"100004458","cluster":"default","namespaceName":"application","configurations":{"portal.elastic.document.type":"biz","portal.elastic.cluster.name":"hermes-es-fws"},"releaseKey":"20170430092936-dee2d58e74515ff3"})";
    std::string release_key;
    Configures configures;
    bool success = fromJsonString(json, release_key, configures);
    CHECK(success);
    CHECK(release_key == "20170430092936-dee2d58e74515ff3");
    CHECK(configures.size() == 2);
    CHECK(configures["portal.elastic.document.type"] == "biz");
    CHECK(configures["portal.elastic.cluster.name"] == "hermes-es-fws");
}

TEST_CASE("create-notifications-v2-url")
{
    NamespaceAttributesMap namespace_attributes =
        {{"test_namespace1", std::make_shared<NamespaceAttributes>("test_release_key1", 12)},
         {"test_namespace2", std::make_shared<NamespaceAttributes>("", -1)}};

    std::string apollo_url = "http://apollo-server.com";
    std::string cluster_name = "default";
    std::string label = "test_label";
    std::string app_id = "test_app";

    std::string url = createNotificationsV2URL(app_id, apollo_url, cluster_name, label, namespace_attributes);
    std::string expected_url(R"(http://apollo-server.com/notifications/v2?appId=test_app&cluster=default&notifications=%255B%257B%2522namespaceName%2522%253A%2522test_namespace1%2522%252C%2522notificationId%2522%253A12%257D%252C%257B%2522namespaceName%2522%253A%2522test_namespace2%2522%252C%2522notificationId%2522%253A-1%257D%255D&label=test_label)");
    CHECK(url == expected_url);
}

TEST_CASE("create-no-cache-configs-url")
{
    std::string apollo_url = "http://apollo-server.com";
    std::string cluster_name = "default";
    std::string label = "test_label";
    std::string app_id = "test_app";

    std::string url1 =
        createNoCacheConfigsURL(app_id, apollo_url, cluster_name, "test_namespace1", label, "test_release_key1", 12);
    std::string expected_url1 =
        R"(http://apollo-server.com/configs/test_app/default/test_namespace1?label=test_label&releaseKey=test_release_key1&messages=%257B%2522details%2522%253A%257B%2522test_app%252Bdefault%252Btest_namespace1%2522%253A12%257D%257D)";
    CHECK(url1 == expected_url1);

    std::string url2 =
        createNoCacheConfigsURL(app_id, apollo_url, cluster_name, "test_namespace2", label, "", -1);
    std::string expected_url2 =
        R"(http://apollo-server.com/configs/test_app/default/test_namespace2?label=test_label)";
    CHECK(url2 == expected_url2);
}

TEST_CASE("create-no-cache-configs-messages")
{
    std::string app_id = "app";
    std::string cluster_name = "default";
    NamespaceType s_namespace = "test";
    int notification_id = 11;

    std::string message = createNoCacheConfigsMessages(app_id, cluster_name, s_namespace, notification_id);
    std::string expected_message(R"({"details":{"app+default+test":11}})");
    CHECK(message == expected_message);
}

TEST_CASE("logger")
{
    // Create a mock logger
    class MockLogger : public ILogger
    {
    public:
        MockLogger() = default;
        ~MockLogger() override = default;
        LogLevel getLogLevel() const override
        {
            return log_level_;
        }
        void setLogLevel(LogLevel level) override
        {
            log_level_ = level;
        }
        void log(LogLevel level, const std::string& message) override
        {
            std::cout << message << std::endl;
            log_messages_.emplace_back(message);
        }

        const std::vector<std::string>& getLogMessages() const
        {
            return log_messages_;
        }

    private:
        LogLevel log_level_ = LogLevel::Debug;
        std::vector<std::string> log_messages_;
    };

    auto logger = std::make_shared<MockLogger>();
    logger->setLogLevel(LogLevel::Warning);  // Set log level to Warning for testing

    LOG_ERROR(logger, "This is an error message");
    LOG_WARN(logger, "This is a warning message");
    LOG_INFO(logger, "This is an info message");
    LOG_DEBUG(logger, "This is a debug message");

    CHECK(logger->getLogMessages().size() == 2);  // Only Warning and Error should be logged
    CHECK(logger->getLogMessages()[0] == "This is an error message");
    CHECK(logger->getLogMessages()[1] == "This is a warning message");
}
