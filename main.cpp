#include <csignal>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/asio/ip/tcp.hpp>

// Exit
volatile std::sig_atomic_t killed = 0;

using namespace boost;

using boost::asio::ip::tcp;

int main() {

  std::cout << "Starting server..." << std::endl;

  try {
    // Create service
    boost::asio::io_service io;

    // Init acceptor
    tcp::acceptor acceptor(io,
        tcp::endpoint(tcp::v4(), 9986),
        tcp::acceptor::reuse_address(true));
    tcp::socket isocket(io);

    // Setup signal
    std::signal(SIGINT, [] (int) {exit(0);});

    // Acceptor function
    std::function<void (void)> do_accept = [&] () {
      acceptor.async_accept(
          isocket,
          [&] (const boost::system::error_code& error) {
          if(!error) {
            // Client accepted, read until closed
            std::cout << "Connection opened from: " << isocket.remote_endpoint().address() << std::endl;
            std::cout << "Printing incoming data:" << std::endl;
            for (;;) {
              boost::array <char, 255> recv_buf;
              boost::system::error_code err;

              std::size_t len = isocket.read_some(boost::asio::buffer(recv_buf),
                  err);
              if(err == boost::asio::error::eof) {
                std::cout << "Connection Closed" << std::endl;
                // Close the socket
                isocket.close();
                break;
              } else if(err)
                throw boost::system::system_error(err);

              std::cout.write(recv_buf.data(), len);
            }
            std::cout << "Repriming acceptor" << std::endl;
            do_accept();
            std::cout << "primed" << std::endl;
          } else
            throw::boost::system::system_error(error);
        }
      );
    };

    do_accept();

    io.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
