//
// Created by alexisvoila on 05.12.2020.
//

#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include "session.h"

namespace net = boost::asio;
namespace sys = boost::system;
using tcp = boost::asio::ip::tcp;

/*
 * The class is responsible for accepting connections
 */
class server {

    net::io_context& ioc_;
    net::signal_set signals_;
    tcp::acceptor acceptor_;
    std::string remote_host_;
    std::string remote_service_;

public:
    server(net::io_context& ioc, std::uint16_t listen_port,
           const std::string_view& remote_host, const std::string_view& remote_service)
        : ioc_(ioc), signals_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), listen_port)),
          remote_host_(remote_host), remote_service_(remote_service)
    {
        configure_signals();
        start_wait_signals();

        start_accept();
    }

    void start_accept() {
        auto new_session = session::create(ioc_, remote_host_, remote_service_);

        acceptor_.async_accept(
            new_session->socket(),
            [this, new_session](const sys::error_code& ec) {
                if (!ec) {
                    new_session->start();
                } else {
                    std::cout << ec.message() << std::endl;
                }

                start_accept();
            });
    }

private:
    void configure_signals() {
        signals_.add(SIGINT);
        signals_.add(SIGTERM);
        signals_.add(SIGQUIT);
    }

    void start_wait_signals() {
        signals_.async_wait(
            [this](const sys::error_code& ec, int /*signo*/) {
                if (ec) {
                    std::cout << ec.message() << std::endl;
                }
                acceptor_.close();
                ioc_.stop();
            });
    }
};
#endif //SERVER_H
