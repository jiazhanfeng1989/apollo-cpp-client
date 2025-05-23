#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/url.hpp>
#include <functional>
#include <string>
#include <memory>
#include <map>

namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace urls = boost::urls;
using tcp = net::ip::tcp;

namespace apollo
{
namespace client
{
using HttpResponseCallback = std::function<void(beast::error_code ec, http::response<http::string_body>)>;
using HttpHeaders = std::map<std::string, std::string>;
using HttpResult = std::pair<http::response<http::string_body>, beast::error_code>;

class HttpClient
{
public:
    HttpClient(net::io_context& io_context);
    ~HttpClient() = default;
    HttpResult get(const std::string& url, const HttpHeaders& headers = {});
    void getAsync(const std::string& url, HttpResponseCallback callback, const HttpHeaders& headers = {});

    HttpResult post(const std::string& url,
                    const std::string& body,
                    const std::string& content_type = "application/json",
                    const HttpHeaders& headers = {});
    void postAsync(const std::string& url,
                   const std::string& body,
                   HttpResponseCallback callback,
                   const std::string& content_type = "application/json",
                   const HttpHeaders& headers = {});

    void setConnectionTimeout(int timeout_ms);
    void setRequestReadTimeout(int timeout_ms);
    void setRequestWriteTimeout(int timeout_ms);

private:
    HttpClient(const HttpClient&) = delete;             // Disable copy constructor
    HttpClient& operator=(const HttpClient&) = delete;  // Disable assignment operator
    HttpClient(HttpClient&&) = delete;                  // Disable move constructor
    HttpClient& operator=(HttpClient&&) = delete;       // Disable move assignment operator

private:
    template <class RequestBody>
    void setupRequest(http::request<RequestBody>& req, const urls::url& url, const HttpHeaders& headers);

    template <class RequestBody>
    HttpResult performRequest(http::request<RequestBody>& req, const urls::url& url);

    template <class RequestBody>
    void performRequestAsync(http::request<RequestBody> req, urls::url url, HttpResponseCallback callback);

    class AsyncSession : public std::enable_shared_from_this<AsyncSession>
    {
    public:
        AsyncSession(net::io_context& ioc,
                     HttpResponseCallback callback,
                     int connection_timeout_ms,
                     int request_read_timeout_ms,
                     int request_write_timeout_ms);
        ~AsyncSession() = default;

        void run(http::request<http::string_body> req, const urls::url& url);

    public:
        void onResolve(beast::error_code ec, tcp::resolver::results_type results);
        void onConnect(beast::error_code ec, tcp::endpoint);
        void onWrite(beast::error_code ec, std::size_t bytes_transferred);
        void onRead(beast::error_code ec, std::size_t bytes_transferred);
        void doTimeout();
        void handleTimeout(beast::error_code ec);

    private:
        tcp::resolver resolver_;
        beast::tcp_stream stream_;
        beast::flat_buffer buffer_;
        http::request<http::string_body> req_;
        http::response<http::string_body> res_;
        HttpResponseCallback callback_;
        net::steady_timer timer_;
        int connection_timeout_ms_;
        int request_read_timeout_ms_;
        int request_write_timeout_ms_;
    };

    net::io_context& io_context_;
    int connection_timeout_ms_ = 500;       // Default connection timeout in milliseconds
    int request_read_timeout_ms_ = 30000;   // Default read timeout in milliseconds
    int request_write_timeout_ms_ = 30000;  // Default write timeout in milliseconds
};

}  // namespace client
}  // namespace apollo