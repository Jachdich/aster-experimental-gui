#include <string>
#include <iostream>
#include "network.h"
#include <thread>

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
    this->port = port;
    addr = address;
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
    if (!successfullyConnected) { emit onlineChanged(true); }
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
        //if (ec.
        std::cerr << "Error whilst reading: " <<  ec.message() << "\n";
        //readUntil();
        if (successfullyConnected) { emit onlineChanged(false); }
        successfullyConnected = false;\
        //cleanUp();
        asio::error_code ec;
        socket.shutdown(ec);
        //ssl_ctx = asio::ssl::context(asio::ssl::context::tlsv12_client);
        //socket = asio::ssl::stream<asio::ip::tcp::socket>(ctx, ssl_ctx);
        while (1) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            asio::error_code ec;
            std::cerr << "Retrying connection to " << addr << ":" << port << "\n";
            asio::ip::tcp::resolver resolver(ctx);
            auto endpoint = resolver.resolve(addr, std::to_string(port), ec);
            if (ec) { std::cerr << "Error whilst retrying: " << ec.message() << "\n"; continue; }
            asio::connect(socket.next_layer(), endpoint, ec);
            if (ec) { std::cerr << "Error whilst retrying: " << ec.message() << "\n"; continue; }
            socket.handshake(asio::ssl::stream_base::client, ec);
            if (ec) { std::cerr << "Error whilst retrying: " << ec.message() << "\n"; continue; }
            break;
        }
        successfullyConnected = true;
        emit onlineChanged(true);
        readUntil();
    }
}
/*
void ClientNetwork::cleanUp() {
    socket.lowest_layer().cancel();
    ctx.stop();
    asio::error_code ec;
    socket.shutdown(ec);
    if (ec) {
        //actually just ignore it
    }
    ctx.restart();
}*/

void ClientNetwork::readUntil() {
    asio::async_read_until(socket, buf, '\n', [this] (std::error_code ec, std::size_t bytes_transferred) {
        handler(ec, bytes_transferred);
    });
}
/*
std::error_code ClientNetwork::try_reconnect() {
    if (successfullyConnected) {
    	cleanUp();
    	asioThread.join();
    }
    if (successfullyConnected) { emit onlineChanged(false); }
    successfullyConnected = false;
    std::error_code ec = connect(addr, port);
    return ec;
}*/
/*
void ClientNetwork::checkAlive() {
    while (true) {
        bool oldOnline = isOnline;
        std::cout << "checking online\n";
        if (net != nullptr) {
            if (net->successfullyConnected) {
                std::error_code ec = net->sendRequest("/ping");
                if (ec) {
                    isOnline = false;
                    std::cout << "not online: " << ec.message() << "\n";
                    net->try_reconnect();
                } else {
                    std::cout << "yes online\n";
                    isOnline = true; //TODO listen for pong
                }
            } else {
                net->try_reconnect();
                isOnline = false;
            }
        } else {
            isOnline = false;
        }
        if (oldOnline != isOnline) {
            emit onlineChanged(isOnline);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}*/
