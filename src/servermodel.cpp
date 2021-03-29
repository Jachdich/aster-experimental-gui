#include "servermodel.h"
#include "mainwindow.h"
#include "network.h"
#include "message.h"
#include "messagecontainer.h"
#include "base64.h"
#include <QByteArray>
#include <QVBoxLayout>
#include <QObject>
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
    layout = new QVBoxLayout();
    messages = new MessageContainer();
    layout->addWidget(messages);
    setLayout(layout);
    QObject::connect(net, &ClientNetwork::msgRecvd, this, &ServerModel::handleNetwork);
}

QString ServerModel::getName() {
    return QString::fromStdString(peers[uuid].uname);
}
QPixmap *ServerModel::getPfp() {
    std::cout << uuid << "\n";
    return peers[uuid].pfp;
}

void ServerModel::sendRequest(std::string data) {
    this->net->sendRequest(data);
}

void ServerModel::initialise() {
    net->connect(ip, port);
    net->sendRequest("/register");
    net->sendRequest("/get_all_metadata");
    net->sendRequest("/history 200");
    net->sendRequest("/get_icon");
    net->sendRequest("/get_name");
}

void ServerModel::connect() {
    net->connect(ip, port);
    net->sendRequest("/login " + std::to_string(uuid));
    net->sendRequest("/get_all_metadata");
    net->sendRequest("/history 200");
}

void ServerModel::addMessage(Message* msg) {
    messages->addMessage(msg);
}

void ServerModel::handleNetwork(QString data) {
    json msg = json::parse(data.toUtf8().constData());
//    std::cout << data.toUtf8().constData() << "\n";
    if (!msg["history"].is_null()) {
        uint32_t pos = 0;
        for (auto &elem : msg["history"]) {
            messages->insertMessage(pos++, new Message(
                            QString::fromStdString(peers[elem["author_uuid"].get<uint64_t>()].uname),
                            QString::fromStdString(elem["content"].get<std::string>()),
                            peers[elem["author_uuid"].get<uint64_t>()].pfp));
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
            emit initialised(this);
            //TODO not guarenteed t0 be initialised
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