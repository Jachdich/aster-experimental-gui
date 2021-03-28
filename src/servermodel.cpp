#include "servermodel.h"
#include "mainwindow.h"
#include "network.h"
#include "message.h"
#include "base64.h"
#include <QByteArray>
#include <vector>
#include <string>
#include <iostream>

ServerModel::ServerModel(std::string name, std::string ip, uint64_t uuid, uint16_t port, std::string pfp_b64) {
    this->name = name;
    this->ip = ip;
    this->uuid = uuid;
    this->port = port;
    std::vector<uint8_t> buf = base64_decode(pfp_b64);
    QByteArray data = QByteArray((const char*)buf.data(), (int)buf.size());
    pfp.loadFromData(data, "PNG");
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

void ServerModel::handleNetwork(QString data, MainWindow *parent) {
    json msg = json::parse(data.toUtf8().constData());
    std::cout << data.toUtf8().constData() << "\n";
    if (!msg["history"].is_null()) {
        uint32_t pos = 0;
        for (auto &elem : msg["history"]) {
            parent->insertMessage(pos++, new Message(
                QString::fromStdString(peers[elem["user"].get<uint64_t>()].uname), //TODO make other one like this
                QString::fromStdString(elem["content"].get<std::string>()),
                peers[elem["user"].get<uint64_t>()].pfp));
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
        }
    } else if (!msg["content"].is_null()) {
        parent->addMessage(new Message(
            QString::fromStdString(peers[msg["user"].get<uint64_t>()].uname),
            QString::fromStdString(msg["content"].get<std::string>()),
            peers[msg["user"].get<uint64_t>()].pfp));
    } else {
        //???
        //ignore for now
    }
}
