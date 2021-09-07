#ifndef __MESSAGECONTAINER_H
#define __MESSAGECONTAINER_H

#include <vector>
#include <QWidget>
#include <thread>
#include <string>
#include "voiceclient.h"
#include "../include/network.h"

class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QPushButton;
class QLabel;
class QWidget;
class Message;
class ServerModel;

class VcContainer : public QWidget {
public:
    std::vector<QLabel*> vcing;
    QHBoxLayout *layout;
    QPushButton *join;
    QPushButton *leave;
    std::thread clientthread, netthread, soundthread;
    void joinVoice();
    void leaveVoice();
    VoiceClient *vc = NULL;
    asio::io_context ctx;
    ServerModel *server;
    std::string channel = "general";

    VcContainer(QWidget *parent, ServerModel *net);
    ~VcContainer();
public slots:
    void changeChannel(const std::string &newChannel);
};


class MessageContainer : public QWidget {
    QVBoxLayout *msglayout;
    QVBoxLayout *layout;
    QWidget *widget;
    QScrollArea *scroll;
    std::vector<Message*> messages;
    
public:
    VcContainer *vc;
    MessageContainer(QWidget* parent, ServerModel *server);
    ~MessageContainer();

    void addMessage(Message* msg);
    void clear();
    void insertMessage(uint32_t idx, Message* msg);
    void setVoice(bool voice);

public slots:
    void sliderRangeChanged(int min, int max);
};

#endif
