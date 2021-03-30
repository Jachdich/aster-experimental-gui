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

signals:
    void msgRecvd(QString msg);

public:
    bool connect(std::string address, uint16_t port);
    asio::streambuf buf;
    asio::io_context ctx;
    asio::ssl::context ssl_ctx;
    asio::ssl::stream<asio::ip::tcp::socket> socket;
    
    void readUntil();
    void handler(std::error_code ec, size_t bytes_transferred);
    void sendRequest(std::string request);
    void handleNetworkPacket(std::string data);

};
#endif
