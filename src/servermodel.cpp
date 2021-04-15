#include "servermodel.h"
#include "mainwindow.h"
#include "network.h"
#include "message.h"
#include "messagecontainer.h"
#include "base64.h"
#include <QByteArray>
#include <QHBoxLayout>
#include <QObject>
#include <QListWidget>
#include <QListWidgetItem>
#include <vector>
#include <string>
#include <iostream>

ServerModel::ServerModel(std::string name, std::string ip, uint16_t port, uint64_t uuid, std::string pfp_b64) {
    this->name = name;
    this->ip = ip;
    this->uuid = uuid;
    this->port = port;
    this->pfp_b64 = pfp_b64;
    std::vector<uint8_t> buf = base64_decode(pfp_b64);
    QByteArray data = QByteArray((const char*)buf.data(), (int)buf.size());
    pfp.loadFromData(data, "PNG");
    QPixmap temp = pfp.scaled(32, 32);
    pfp = temp;
    net = new ClientNetwork();
    layout = new QHBoxLayout(this);
    channels = new QListWidget(this);
    channels->setFixedWidth(248);
    messages = new MessageContainer(this);
    layout->addWidget(channels);
    layout->addWidget(messages);
    setLayout(layout);
    QObject::connect(net, &ClientNetwork::msgRecvd, this, &ServerModel::handleNetwork);
    QObject::connect(channels, &QListWidget::currentItemChanged, this, &ServerModel::changeChannel);
}

ServerModel::~ServerModel() {
	delete net;
	delete layout;
	delete messages;
}

void ServerModel::changeChannel(QListWidgetItem *current, QListWidgetItem *previous) {
    std::string name = current->text().toUtf8().constData();
    messages->clear();
    net->sendRequest("/join " + name);
    net->sendRequest("/history 200");
}


QString ServerModel::getName() {
    return QString::fromStdString(peers[uuid].uname);
}
QPixmap *ServerModel::getPfp() {
    std::cout << uuid << "\n";
    return peers[uuid].pfp;
}

std::error_code ServerModel::sendRequest(std::string data) {
    return this->net->sendRequest(data);
}

std::error_code ServerModel::initialise(uint64_t uuid) {
    std::error_code ret = net->connect(ip, port);
    if (!ret) {
        if (uuid == 0) {
            net->sendRequest("/register");
        } else {
            net->sendRequest("/login " + std::to_string(uuid));
        }
        net->sendRequest("/get_icon");
        net->sendRequest("/get_name");
        net->sendRequest("/get_all_metadata");
        net->sendRequest("/get_channels");
    } else {
        emit initialised(this, false);
    }
    return ret;
}

std::error_code ServerModel::connect() {
    std::error_code ret = net->connect(ip, port);
    if (!ret) {
        net->sendRequest("/login " + std::to_string(uuid));
        net->sendRequest("/get_all_metadata");
        net->sendRequest("/get_channels");
    } else {
        emit initialised(this, false);
    }
    return ret;
}

void ServerModel::addMessage(Message* msg) {
    messages->addMessage(msg);
}

void ServerModel::addChannel(std::string name) {
    channels->addItem(new QListWidgetItem(QString::fromStdString(name), channels));
}

void ServerModel::handleNetwork(QString data) {
    json msg = json::parse(data.toUtf8().constData());
    std::cout << data.toUtf8().constData() << "\n";
    if (!msg["history"].is_null()) {
        uint32_t pos = 0;
        for (auto &elem : msg["history"]) {
            messages->insertMessage(pos++, new Message(
                            QString::fromStdString(peers[elem["author_uuid"].get<uint64_t>()].uname),
                            QString::fromStdString(elem["content"].get<std::string>()),
                            peers[elem["author_uuid"].get<uint64_t>()].pfp));
            std::cout << elem["author_uuid"].get<uint64_t>() << "\n";
        }
    } else if (!msg["command"].is_null()) {
        if (msg["command"].get<std::string>() == "set") {
            if (msg["key"].get<std::string>() == "self_uuid") {
                uuid = msg["value"].get<uint64_t>();
                std::cout << "Set uuid to " << uuid << "\n";
            }
        } else if (msg["command"].get<std::string>() == "metadata") {
            for (auto &elem : msg["data"]) {
                uint64_t elem_uuid = elem["uuid"].get<uint64_t>();
                if (peers.count(elem_uuid) == 0) {
                    peers[elem_uuid] = Metadata::from_json(elem);
                } else {
                    peers[elem_uuid].update(elem);
                }
                
            }
        } else if (msg["command"].get<std::string>() == "get_icon") {
            std::vector<uint8_t> buf = base64_decode(msg["data"].get<std::string>());
            QByteArray data = QByteArray((const char*)buf.data(), (int)buf.size());
            pfp.loadFromData(data, "PNG");
            QPixmap temp = pfp.scaled(32, 32);
            pfp = temp;
            pfp_b64 = msg["data"].get<std::string>();
        } else if (msg["command"].get<std::string>() == "get_name") {
            name = msg["data"].get<std::string>();
        } else if (msg["command"].get<std::string>() == "get_channels") {
            for (auto &elem : msg["data"]) {
                addChannel(elem.get<std::string>());
            }
        }
        if (!isInitialised) {
            if (uuid != 0 && name != "" && pfp_b64 != "") {
                emit initialised(this, true);
                isInitialised = true;
            }
        }
    } else if (!msg["content"].is_null()) {
        messages->addMessage(new Message(
            QString::fromStdString(peers[msg["author_uuid"].get<uint64_t>()].uname),
            QString::fromStdString(msg["content"].get<std::string>()),
            peers[msg["author_uuid"].get<uint64_t>()].pfp));
    } else {
        //???
        //ignore for now
    }
}
