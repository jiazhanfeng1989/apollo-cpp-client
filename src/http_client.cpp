#include "http_client.h"
#include <boost/asio/strand.hpp>
#include <limits>
#include "boost/asio/error.hpp"
#include "boost/beast/core/error.hpp"

namespace apollo
{
namespace client
{

HttpClient::HttpClient(net::io_context& io_context)
    : io_context_(io_context)
{
}

HttpResult HttpClient::get(const std::string& url_str, const HttpHeaders& headers)
{
    auto r = urls::parse_uri(url_str);
    if (!r)
    {
        return {http::response<http::string_body>{}, beast::error_code{net::error::invalid_argument}};
    }

    urls::url url = r.value();
    if (url.empty())
    {
        return {http::response<http::string_body>{}, beast::error_code{net::error::invalid_argument}};
    }

    http::request<http::string_body> req{http::verb::get, url.path(), 11};
    setupRequest(req, url, headers);
    return performRequest(req, url);
}

void HttpClient::getAsync(const std::string& url_str, HttpResponseCallback callback, const HttpHeaders& headers)
{
    auto r = urls::parse_uri(url_str);
    if (!r)
    {
        callback(beast::error_code{net::error::invalid_argument}, http::response<http::string_body>{});
        return;
    }
    urls::url url = r.value();
    http::request<http::string_body> req{http::verb::get, url.path(), 11};
    setupRequest(req, url, headers);
    performRequestAsync(std::move(req), std::move(url), std::move(callback));
}

HttpResult HttpClient::post(const std::string& url_str,
                            const std::string& body,
                            const std::string& content_type,
                            const HttpHeaders& headers)
{
    auto r = urls::parse_uri(url_str);
    if (!r)
    {
        return {http::response<http::string_body>{}, beast::error_code{net::error::invalid_argument}};
    }

    urls::url url = r.value();
    http::request<http::string_body> req{http::verb::post, url.path(), 11};
    req.body() = body;
    req.prepare_payload();
    req.set(http::field::content_type, content_type);
    setupRequest(req, url, headers);

    return performRequest(req, url);
}

void HttpClient::postAsync(const std::string& url_str,
                           const std::string& body,
                           HttpResponseCallback callback,
                           const std::string& content_type,
                           const HttpHeaders& headers)
{
    auto r = urls::parse_uri(url_str);
    if (!r)
    {
        callback(beast::error_code{net::error::invalid_argument}, http::response<http::string_body>{});
        return;
    }
    urls::url url = r.value();

    http::request<http::string_body> req{http::verb::post, url.path(), 11};
    req.body() = body;
    req.prepare_payload();
    req.set(http::field::content_type, content_type);
    setupRequest(req, url, headers);

    performRequestAsync(std::move(req), std::move(url), std::move(callback));
}

void HttpClient::setConnectionTimeout(int timeout_ms)
{
    connection_timeout_ms_ = timeout_ms;
}

void HttpClient::setRequestReadTimeout(int timeout_ms)
{
    request_read_timeout_ms_ = timeout_ms;
}

void HttpClient::setRequestWriteTimeout(int timeout_ms)
{
    request_write_timeout_ms_ = timeout_ms;
}

template <class RequestBody>
void HttpClient::setupRequest(http::request<RequestBody>& req,
                              const urls::url& url,
                              const std::map<std::string, std::string>& headers)
{
    std::string target = url.path();
    if (!url.query().empty())
    {
        target += "?" + std::string(url.query());
    }
    req.target(target);

    std::string host = url.host();
    if (url.has_port())
    {
        host += ":" + std::string(url.port());
    }
    req.set(http::field::host, host);

    req.set(http::field::user_agent, "ApolloClient/1.0");

    for (const auto& p : headers)
    {
        req.set(p.first, p.second);
    }
}

template <class RequestBody>
std::pair<http::response<http::string_body>, beast::error_code> HttpClient::performRequest(http::request<RequestBody>& req,
                                                                                           const urls::url& url)
{
    beast::error_code ec;
    http::response<http::string_body> res;

    if (url.scheme() == "https")
    {
        return {res, beast::error_code(net::error::no_protocol_option)};
    }

    std::string host = url.host();
    std::string port = url.has_port() ? std::string(url.port()) : "80";

    tcp::resolver resolver(io_context_);
    auto results = resolver.resolve(host, port, ec);
    if (ec)
    {
        return {res, beast::error_code(net::error::host_unreachable)};
    }

    beast::tcp_stream stream(io_context_);
    stream.expires_after(std::chrono::milliseconds(connection_timeout_ms_));
    stream.connect(results, ec);
    if (ec)
    {
        return {res, beast::error_code(net::error::host_unreachable)};
    }

    stream.expires_after(std::chrono::milliseconds(request_write_timeout_ms_));
    http::write(stream, req, ec);
    if (ec)
    {
        return {res, ec};
    }

    beast::flat_buffer buffer;
    stream.expires_after(std::chrono::milliseconds(request_read_timeout_ms_));
    http::read(stream, buffer, res, ec);
    if (ec)
    {
        return {res, ec};
    }

    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
    if (ec && ec != beast::errc::not_connected)
    {
        return {res, ec};
    }

    ec = {};
    return {res, ec};
}

template <class RequestBody>
void HttpClient::performRequestAsync(http::request<RequestBody> req, urls::url url, HttpResponseCallback callback)
{
    auto session = std::make_shared<AsyncSession>(io_context_,
                                                  std::move(callback),
                                                  connection_timeout_ms_,
                                                  request_read_timeout_ms_,
                                                  request_write_timeout_ms_);
    session->run(std::move(req), url);
}

HttpClient::AsyncSession::AsyncSession(net::io_context& ioc,
                                       HttpResponseCallback callback,
                                       int connection_timeout_ms,
                                       int request_read_timeout_ms,
                                       int request_write_timeout_ms)
    : resolver_(ioc)
    , stream_(ioc)
    , callback_(std::move(callback))
    , timer_(ioc)
    , connection_timeout_ms_(connection_timeout_ms)
    , request_read_timeout_ms_(request_read_timeout_ms)
    , request_write_timeout_ms_(request_write_timeout_ms)
{
}

void HttpClient::AsyncSession::run(http::request<http::string_body> req, const urls::url& url)
{
    req_ = std::move(req);

    std::string host = url.host();
    std::string port = url.has_port() ? std::string(url.port()) : "80";

    if (url.scheme() == "https")
    {
        callback_(beast::error_code(net::error::no_protocol_option), res_);
        return;
    }

    doTimeout();

    resolver_.async_resolve(host, port, beast::bind_front_handler(&AsyncSession::onResolve, shared_from_this()));
}

void HttpClient::AsyncSession::onResolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec)
    {
        callback_(beast::error_code(net::error::host_unreachable), res_);
        return;
    }

    stream_.expires_after(std::chrono::milliseconds(connection_timeout_ms_));
    stream_.async_connect(results, beast::bind_front_handler(&AsyncSession::onConnect, shared_from_this()));
}

void HttpClient::AsyncSession::onConnect(beast::error_code ec, tcp::endpoint)
{
    if (ec)
    {
        callback_(beast::error_code(net::error::host_unreachable), res_);
        return;
    }

    stream_.expires_after(std::chrono::milliseconds(connection_timeout_ms_));
    http::async_write(stream_, req_, beast::bind_front_handler(&AsyncSession::onWrite, shared_from_this()));
}

void HttpClient::AsyncSession::onWrite(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
    {
        callback_(ec, res_);
        return;
    }

    stream_.expires_after(std::chrono::milliseconds(request_write_timeout_ms_));
    http::async_read(stream_, buffer_, res_, beast::bind_front_handler(&AsyncSession::onRead, shared_from_this()));
}

void HttpClient::AsyncSession::onRead(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    timer_.cancel();

    beast::error_code close_ec;
    stream_.socket().shutdown(tcp::socket::shutdown_both, close_ec);

    if (close_ec && close_ec != beast::errc::not_connected)
    {
        ec = close_ec;
    }

    callback_(ec, res_);
}

void HttpClient::AsyncSession::doTimeout()
{
    auto total_timeout_ms = connection_timeout_ms_ + request_read_timeout_ms_ + request_write_timeout_ms_;
    if (total_timeout_ms <= 0 || total_timeout_ms > std::numeric_limits<int>::max())
    {
        total_timeout_ms = std::numeric_limits<int>::max();
    }

    timer_.expires_after(std::chrono::milliseconds(total_timeout_ms));
    timer_.async_wait(beast::bind_front_handler(&AsyncSession::handleTimeout, shared_from_this()));
}

void HttpClient::AsyncSession::handleTimeout(beast::error_code ec)
{
    if (ec != boost::asio::error::operation_aborted && timer_.expiry() <= net::steady_timer::clock_type::now())
    {
        stream_.close();
    }
}

}  // namespace client
}  // namespace apollo