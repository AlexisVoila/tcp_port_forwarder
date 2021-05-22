//
// Created by alexisvoila on 05.12.2020.
//

#ifndef SESSION_H
#define SESSION_H

#include <iostream>
#include <array>

#include <boost/asio.hpp>

namespace net = boost::asio;
namespace sys = boost::system;
using tcp = boost::asio::ip::tcp;

/*
 * The class is responsible for handling connections
 */
class session : public std::enable_shared_from_this<session> {
    enum { max_buff_size = 0x10000 };
    using buffer_type = std::array<std::uint8_t, max_buff_size>;

    tcp::resolver resolver_;
    tcp::socket local_sock_;
    tcp::socket remote_sock_;

    std::string remote_host_;
    std::string remote_service_;
    std::string remote_ep_;
    std::string remote_resolved_ep_;

    std::string client_ep_;

    buffer_type local_buffer_{};
    buffer_type remote_buffer_{};


    session(net::io_context& ios, const std::string_view& remote_host, const std::string_view& remote_service)
        : resolver_(ios), local_sock_(ios), remote_sock_(ios),
          remote_host_{remote_host}, remote_service_{remote_service} {
        remote_ep_ = remote_host_ + ':' + remote_service_;
    }

public:
    ~session() {
        std::cout << "connection from " << client_ep_ << " to " << remote_ep_ << remote_resolved_ep_ << " closed\n";
    }

    using pointer = std::shared_ptr<session>;

    tcp::socket& socket() { return local_sock_; }

    static pointer create(net::io_context& io_context,
                          const std::string_view& remote_host, const std::string_view& remote_port) {
        return pointer(new session(io_context, remote_host, remote_port));
    }

    void start() {
        tcp::endpoint from_ep{local_sock_.remote_endpoint()};
        client_ep_ = from_ep.address().to_string() + ':' + std::to_string(from_ep.port());
        std::cout << "accepted connection from " << client_ep_ << std::endl;
        do_resolve();
    }

private:
    void do_resolve() {
        resolver_.async_resolve(
            remote_host_, remote_service_,
            [this, self{shared_from_this()}](const sys::error_code& ec, const tcp::resolver::results_type& eps) {
                if (!ec) {
                    do_connect(eps);
                } else {
                    std::cout << '[' << remote_ep_ << "] " << ec.message() << std::endl;
                }
            });
    }

    void do_connect(const tcp::resolver::results_type& eps) {
        net::async_connect(
            remote_sock_, eps,
            [this, self{shared_from_this()}] (const sys::error_code& ec, const tcp::endpoint& /*ep*/) {
                tcp::endpoint to_ep{remote_sock_.remote_endpoint()};
                remote_resolved_ep_ = '(' + to_ep.address().to_string() + ':' + std::to_string(to_ep.port()) + ')';
                std::cout << "connection from " << client_ep_ << " to "
                          << remote_ep_ << remote_resolved_ep_ << " established\n";
                if (!ec) {
                    do_read_from_local();
                    do_read_from_remote();
                } else {
                    std::cout << ec.message() << std::endl;
                }
            });
    }

    void do_read_from_local() {
        local_sock_.async_read_some(
            net::buffer(local_buffer_),
            [this, self{shared_from_this()}](const sys::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    do_write_to_remote(bytes_transferred);
                } else if(ec != boost::asio::error::eof) {
                    std::cout << ec.message() << std::endl;
                }
            });
    }

    void do_read_from_remote() {
        remote_sock_.async_read_some(
            net::buffer(remote_buffer_),
            [this, self{shared_from_this()}](const sys::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    do_write_to_local(bytes_transferred);
                } else if(ec != boost::asio::error::eof) {
                    std::cout << ec.message() << std::endl;
                }
            });
    }

    void do_write_to_remote(std::size_t bytes_transferred) {
        net::async_write(
            remote_sock_, net::buffer(local_buffer_.data(), bytes_transferred),
            [this, self{shared_from_this()}] (const sys::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    do_read_from_local();
                } else if(ec != boost::asio::error::eof) {
                    std::cout << ec.message() << std::endl;
                }
            });
    }

    void do_write_to_local(std::size_t bytes_transferred) {
        net::async_write(
            local_sock_, net::buffer(remote_buffer_.data(), bytes_transferred),
            [this, self{shared_from_this()}] (const sys::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    do_read_from_remote();
                } else if(ec != boost::asio::error::eof) {
                    std::cout << ec.message() << std::endl;
                }
            });
    }
};
#endif //SESSION_H
