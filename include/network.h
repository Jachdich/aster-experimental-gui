#ifndef __NETWORK_H
#define __NETWORK_H
#include <QString>
#include <QObject>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ClientNetwork : public QObject {
    Q_OBJECT
public:
    inline ClientNetwork() : ssl_ctx(asio::ssl::context::tlsv12_client), socket(ctx, ssl_ctx) {
        
    }

    inline ~ClientNetwork() {
    	socket.lowest_layer().cancel();
    	ctx.stop();
    	asioThread.join();
    	//lmao the socket is closed, I dont give a fuck about errors
    }

signals:
    void msgRecvd(QString msg);

public:
    std::error_code connect(std::string address, uint16_t port);
    asio::streambuf buf;
    asio::io_context ctx;
    asio::ssl::context ssl_ctx;
    asio::ssl::stream<asio::ip::tcp::socket> socket;
    std::thread asioThread;
    
    void readUntil();
    void handler(std::error_code ec, size_t bytes_transferred);
    void sendRequest(std::string request);
    void handleNetworkPacket(std::string data);

};
#endif
