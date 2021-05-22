# tcp_port_forwarder
Simple C++(17) application for TCP port forwarding

## Dependencies
* CMake
* C++ Boost Library (boost::asio)

## Build instructions
#### Linux
    $ git clone https://github.com/alexisvoila/tcp_port_forwarder.git
    $ cd tcp_port_forwarder
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make   

## Help
    Usage
        port_forwarder <listen_port> <destination_host> <destination_port>
    Example 
        tcp_port_forwarder 2000 127.0.0.1 22
        tcp_port_forwarder 8443 example.com 443
    
    To stop program pres Ctrl+C 
## Usage example
    Forwarding tcp connections from port 2000 to port 22 ssh
    $ ./tcp_port_forwarder 2000 127.0.0.1 22
    
    In another terminal run
    $ ssh -p 2000 username@127.0.0.1