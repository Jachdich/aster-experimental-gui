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
    std::string addr;
    uint16_t port;
public:
    inline ClientNetwork() : ssl_ctx(asio::ssl::context::tlsv12_client), socket(ctx, ssl_ctx) {
        
    }

    inline ~ClientNetwork() {
    	if (successfullyConnected) {
    	    socket.lowest_layer().cancel();
    	    ctx.stop();
    	    asioThread.join();
    	}
    }

signals:
    void msgRecvd(QString msg);
    void onlineChanged(bool online);

public:
    std::error_code connect(std::string address, uint16_t port);
    asio::streambuf buf;
    asio::io_context ctx;
    asio::ssl::context ssl_ctx;
    asio::ssl::stream<asio::ip::tcp::socket> socket;
    std::thread asioThread;
    bool successfullyConnected = false;
    
    void readUntil();
    std::error_code try_reconnect();
    void handler(std::error_code ec, size_t bytes_transferred);
    std::error_code sendRequest(std::string request);
    void handleNetworkPacket(std::string data);

};
#endif
