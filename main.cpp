#include <boost/lexical_cast.hpp>
#include "server.h"

void print_usage() {
    cout << "Usage: \n\tport_forwarder <listen_port> <destination_host> <destination_port>\n";
    cout << "Example: \n\ttcp_port_forwarder 2000 127.0.0.1 22\n";
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        print_usage();
        return 0;
    }

    uint16_t src_port{0}, dst_port{0};
    try {
        src_port = boost::lexical_cast<uint16_t>(argv[1]);
        dst_port = boost::lexical_cast<uint16_t>(argv[3]);
    } catch (const boost::bad_lexical_cast::exception& ex) {
        cerr << "Invalid program arguments\n";
        print_usage();
        return -1;
    }

    try {
        net::io_context io_context;
        server srv(io_context, src_port, argv[2], dst_port);
        io_context.run();
    } catch (const exception& ex) {
        cerr << ex.what() << endl;
    }

    return 0;
}

