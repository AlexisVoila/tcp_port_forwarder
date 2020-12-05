//
// Created by alexisvoila on 05.12.2020.
//

#ifndef SERVER_H
#define SERVER_H

#include "session.h"

/*
 * The class is responsible for accepting connections
 */
class server {
    net::io_context& ioc_;
    net::signal_set signals_;
    tcp::acceptor acceptor_;
    string dst_host_;
    uint16_t dst_port_;
public:
    server(net::io_context& ioc, uint16_t listen_port, const string& dst_host, uint16_t dest_port)
            : ioc_(ioc), signals_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), listen_port)),
              dst_host_(dst_host), dst_port_(dest_port)
    {
        configure_signals();
        start_wait_signals();

        start_accept();
    }

    void start_accept() {
        auto new_session = session::create(ioc_, dst_host_, dst_port_);

        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&server::handle_accept, this,
                                           new_session,
                                           net::placeholders::error));
    }

private:
    void configure_signals() {
        signals_.add(SIGINT);
        signals_.add(SIGTERM);
#if defined(SIGQUIT)
        signals_.add(SIGQUIT);
#endif
    }

    void start_wait_signals() {
        signals_.async_wait(
                [this](const sys::error_code& ec, int /*signo*/)
                {
                    if (ec) {
                        cout << ec.message() << endl;
                    }
                    acceptor_.close();
                    ioc_.stop();
                });
    }

    void handle_accept(const boost::shared_ptr<session>& new_session, const sys::error_code& ec) {
        if (!ec) {
            new_session->start();
        } else {
            cout << ec.message() << endl;
        }

        start_accept();
    }
};
#endif //SERVER_H
