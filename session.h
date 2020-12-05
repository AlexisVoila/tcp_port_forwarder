//
// Created by alexisvoila on 05.12.2020.
//

#ifndef SESSION_H
#define SESSION_H

#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace std;

namespace net = boost::asio;
namespace sys = boost::system;
using tcp = boost::asio::ip::tcp;

/*
 * The class is responsible for handling connections
 */
class session : public boost::enable_shared_from_this<session> {
    enum { max_buff_size = 0x10000 };
    using buffer_type = boost::array<uint8_t, max_buff_size>;

    tcp::resolver resolver_;
    tcp::socket src_sock_;
    tcp::socket dst_sock_;
    tcp::endpoint dst_endpoint_;
    buffer_type src_buffer_;
    buffer_type dst_buffer_;

    session(net::io_context& ios, const string& dst_host, uint16_t dest_port)
            : resolver_(ios), src_sock_(ios), dst_sock_(ios),
              dst_endpoint_(tcp::endpoint(net::ip::make_address(dst_host), dest_port)) {
    }
public:
    using pointer = boost::shared_ptr<session>;

    tcp::socket& socket() { return src_sock_; }

    static pointer create(net::io_context& io_context, const string& dst_host, uint16_t dst_port) {
        return pointer(new session(io_context, dst_host, dst_port));
    }

    void start() {
        resolver_.async_resolve(dst_endpoint_,
                                boost::bind(&session::handle_resolve, shared_from_this(),
                                            net::placeholders::error,
                                            net::placeholders::results));
    }

private:
    void handle_resolve(const sys::error_code& ec, const tcp::resolver::results_type& eps) {
        if (!ec) {
            net::async_connect(dst_sock_, eps,
                               boost::bind(&session::handle_connect, shared_from_this(),
                                           net::placeholders::error));
        } else if(ec != boost::asio::error::eof) {
            cout << ec.message() << endl;
        }
    }

    void handle_connect(const sys::error_code& ec) {
        if (!ec) {
            src_sock_.async_read_some(net::buffer(src_buffer_),
                                      boost::bind(&session::handle_src_read, shared_from_this(),
                                                  net::placeholders::error,
                                                  net::placeholders::bytes_transferred));
            dst_sock_.async_read_some(net::buffer(dst_buffer_),
                                      boost::bind(&session::handle_dst_read, shared_from_this(),
                                                  net::placeholders::error,
                                                  net::placeholders::bytes_transferred));
        } else if(ec != boost::asio::error::eof) {
            cout << ec.message() << endl;
        }
    }

    void handle_src_read(const sys::error_code& ec, size_t bytes_transferred) {
        if (!ec) {
            net::async_write(dst_sock_, net::buffer(src_buffer_.data(), bytes_transferred),
                             boost::bind(&session::handle_dst_write, shared_from_this(),
                                         net::placeholders::error));
        } else if(ec != boost::asio::error::eof) {
            cout << ec.message() << endl;
        }
    }

    void handle_dst_read(const sys::error_code& ec, size_t bytes_transferred) {
        if (!ec) {
            net::async_write(src_sock_, net::buffer(dst_buffer_.data(), bytes_transferred),
                             boost::bind(&session::handle_src_write, shared_from_this(),
                                         net::placeholders::error));
        } else if(ec != boost::asio::error::eof) {
            cout << ec.message() << endl;
        }
    }

    void handle_src_write(const sys::error_code& ec) {
        if (!ec) {
            dst_sock_.async_read_some(net::buffer(dst_buffer_),
                                      boost::bind(&session::handle_dst_read, shared_from_this(),
                                                  net::placeholders::error,
                                                  net::placeholders::bytes_transferred));
        } else if(ec != boost::asio::error::eof) {
            cout << ec.message() << endl;
        }
    }

    void handle_dst_write(const sys::error_code& ec) {
        if (!ec) {
            src_sock_.async_read_some(net::buffer(src_buffer_),
                                      boost::bind(&session::handle_src_read, shared_from_this(),
                                                  net::placeholders::error,
                                                  net::placeholders::bytes_transferred));
        } else if(ec != boost::asio::error::eof) {
            cout << ec.message() << endl;
        }
    }
};
#endif //SESSION_H
