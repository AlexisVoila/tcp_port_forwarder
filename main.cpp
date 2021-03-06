#include <charconv>
#include "server.h"

void print_usage() {
    std::cout << "Usage: \n\tport_forwarder <listen_port> <destination_host> <destination_port>\n";
    std::cout << "Example 1: \n\ttcp_port_forwarder 2000 127.0.0.1 22\n";
    std::cout << "Example 2: \n\ttcp_port_forwarder 8443 google.com https\n";
}

std::uint16_t get_port_from_cmd_line(const char* arg) {
    std::uint16_t number{0};
    std::string_view arg_str{arg};
    std::from_chars(arg_str.begin(), arg_str.end(), number);
    return number;
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        print_usage();
        return 0;
    }

    std::uint16_t src_port = get_port_from_cmd_line(argv[1]);
    if (!src_port) {
        std::cerr << "Invalid listen_port argument\n";
        print_usage();
        return 0;
    }

    try {
        net::io_context io_context;
        server srv(io_context, src_port, argv[2], argv[3]);
        io_context.run();
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}

