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
class QHBoxLayout;
class QListWidget;
class MessageContainer;
class QListWidgetItem;
class Message;
class QLabel;
class OnlineView;

class ServerModel : public QWidget {
Q_OBJECT

    void addChannel(std::string name);
    void changeChannel(QListWidgetItem *current, QListWidgetItem *previous);
    QHBoxLayout* layout;
    QListWidget* channels;
    std::vector<QLabel*> channelWidgets;
    std::string currentChannel = "general";
public:
    MessageContainer* messages;
    OnlineView*       online;
    std::string name = "";
    std::string ip;
    uint64_t uuid = 0;
    uint16_t port;
    QPixmap pfp;
    std::string pfp_b64 = "";
    std::unordered_map<uint64_t, Metadata> peers = {};
    ClientNetwork* net;
    bool isInitialised = false;
    
    ServerModel(std::string name, std::string ip, uint16_t port, uint64_t uuid, std::string pfp_b64);
	~ServerModel();
    void handleNetwork(QString data);
    QString getName();
    QPixmap *getPfp();
    std::error_code sendRequest(std::string data);
    void addMessage(Message* msg);

    std::error_code initialise(uint64_t uuid, ClientMeta meta);
    std::error_code connect(ClientMeta meta);
    std::error_code updateMeta(ClientMeta meta);

signals:
    void initialised(ServerModel*, bool);
};

#endif
