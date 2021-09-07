#ifndef __SERVERMODEL_H
#define __SERVERMODEL_H
#include <QPixmap>
#include <QString>
#include <QWidget>

#include <unordered_map>
#include <string>
#include <thread>

#include "metadata.h"
#include "voiceclient.h"

class ClientNetwork;
class MainWindow;
class QHBoxLayout;
class QListWidget;
class MessageContainer;
class QListWidgetItem;
class Message;
class QLabel;
class OnlineView;
class QSplitter;

class ServerModel : public QWidget {
Q_OBJECT

    void addChannel(std::string name);
    void changeChannel(QListWidgetItem *current, QListWidgetItem *previous);
    QHBoxLayout* layout;
    QListWidget* channels;

    std::vector<QLabel*> channelWidgets;
    std::string currentChannel = "general";
    std::thread aliveThread;

public:
    QSplitter *splitter;
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
    bool isInBackground = true;
    
    ServerModel(QWidget *parent, std::string name, std::string ip, uint16_t port, uint64_t uuid, std::string pfp_b64, int sa, int sb);
	~ServerModel();
    void handleNetwork(QString data);
    QString getName();
    QPixmap *getPfp();
    std::error_code sendRequest(std::string data);
    void addMessage(Message* msg);
    const Metadata &getMeta();

    void splitterMoved(int pos, int index);

    std::error_code initialise(uint64_t uuid, ClientMeta meta);
    std::error_code connect(ClientMeta meta);
    std::error_code updateMeta(ClientMeta meta);

signals:
    void initialised(ServerModel*, bool);
    void splitChanged(int sa, int sb);
};

#endif
