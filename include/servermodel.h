#ifndef __SERVERMODEL_H
#define __SERVERMODEL_H
#include <QPixmap>
#include <QString>

#include <unordered_map>
#include <string>

#include "metadata.h"

class ClientNetwork;
class MainWindow;

struct ServerModel {
    std::string name;
    std::string ip;
    uint64_t uuid;
    uint16_t port;
    QPixmap pfp;
    std::unordered_map<uint64_t, Metadata> peers = {};
    ClientNetwork* net;
    
    ServerModel(std::string name, std::string ip, uint64_t uuid, uint16_t port, std::string pfp_b64);
    void handleNetwork(QString data, MainWindow *parent);
    QString getName();
    QPixmap *getPfp();
    void sendRequest(std::string data);
};

#endif
