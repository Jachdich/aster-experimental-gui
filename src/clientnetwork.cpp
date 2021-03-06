#include <string>
#include <iostream>
#include "network.h"

using asio::ip::tcp;

std::error_code ClientNetwork::sendRequest(std::string request) {
    asio::error_code error;

    asio::write(socket, asio::buffer(request + "\n"), error);
    return error;
}

void ClientNetwork::handleNetworkPacket(std::string data) {
    emit msgRecvd(QString::fromStdString(data));
}

std::error_code ClientNetwork::connect(std::string address, uint16_t port) {
    //std::cout << address << ":" << port << "\n";
    asio::error_code ec;

    asio::ip::tcp::resolver resolver(ctx);
    auto endpoint = resolver.resolve(address, std::to_string(port), ec);
    if (ec) return ec;
    asio::connect(socket.next_layer(), endpoint, ec);
    if (ec) return ec;
    socket.handshake(asio::ssl::stream_base::client, ec);
    if (ec) return ec; 

    readUntil();
    asioThread = std::thread([&]() {ctx.run();});
    successfullyConnected = true;
    return ec;
}

void ClientNetwork::handler(std::error_code ec, size_t bytes_transferred) {
    std::cout << bytes_transferred << "\n";
    if (!ec) {
    
        std::string data(
                buffers_begin(buf.data()),
                buffers_begin(buf.data()) + (bytes_transferred
                  - 1));
                  
        buf.consume(bytes_transferred);
        readUntil();

        handleNetworkPacket(data);

    } else {
        std::cerr << "ERROR: " <<  ec.message() << "\n";
        readUntil();
    }
}

void ClientNetwork::readUntil() {
    asio::async_read_until(socket, buf, '\n', [this] (std::error_code ec, std::size_t bytes_transferred) {
        handler(ec, bytes_transferred);
    });
}
