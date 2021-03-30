#ifndef __SERVERMODEL_H
#define __SERVERMODEL_H
#include <QPixmap>
#include <QString>
#include <QWidget>

#include <unordered_map>
#include <string>

#include "metadata.h"

class ClientNetwork;
class MainWindow;
class QVBoxLayout;
class MessageContainer;
class Message;

class ServerModel : public QWidget {
Q_OBJECT
public:
    QVBoxLayout* layout;
    MessageContainer* messages;
    std::string name;
    std::string ip;
    uint64_t uuid;
    uint16_t port;
    QPixmap pfp;
    std::string pfp_b64;
    std::unordered_map<uint64_t, Metadata> peers = {};
    ClientNetwork* net;
    
    ServerModel(std::string name, std::string ip, uint16_t port, uint64_t uuid, std::string pfp_b64);
    void handleNetwork(QString data);
    QString getName();
    QPixmap *getPfp();
    void sendRequest(std::string data);
    void addMessage(Message* msg);

    bool initialise(uint64_t uuid);
    bool connect();

signals:
    void initialised(ServerModel*);
};

#endif
