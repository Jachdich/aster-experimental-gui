#include "servermodel.h"
#include "mainwindow.h"
#include "network.h"
#include "message.h"
#include "smallprofile.h"
#include "messagecontainer.h"
#include "onlineview.h"
#include "base64.h"
#include "metadata.h"
#include "channelwidget.h"
#include <QByteArray>
#include <QHBoxLayout>
#include <QObject>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <Qt>
#include <vector>
#include <string>
#include <iostream>

ServerModel::ServerModel(QWidget *parent, std::string name, std::string ip, uint16_t port, uint64_t uuid, std::string pfp_b64)
    : QWidget(parent) {
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
    channels->setFixedWidth(128);
    messages = new MessageContainer(this);
    online   = new OnlineView(this);
    online->setFixedWidth(196);

    layout->addWidget(channels);
    layout->addWidget(messages);
    layout->addWidget(online);
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
    currentChannel = current->text().toUtf8().constData();
    messages->clear();
    net->sendRequest("/join " + currentChannel);
    net->sendRequest("/history 200");
    QWidget* currentWidget = channels->itemWidget(current);
    QWidget* previousWidget = channels->itemWidget(previous);
    channels->itemWidget(current)->setProperty("unread", false);
    channels->itemWidget(current)->setProperty("selected", true);
    if (previous != nullptr) {
    	channels->itemWidget(previous)->setProperty("selected", false);
    	previousWidget->style()->polish(previousWidget);
    }
    currentWidget->style()->polish(currentWidget);
}


QString ServerModel::getName() {
    return QString::fromStdString(peers[uuid].uname);
}

const Metadata &ServerModel::getMeta() {
    return peers[uuid];
}

QPixmap *ServerModel::getPfp() {
    return peers[uuid].pfp;
}

std::error_code ServerModel::sendRequest(std::string data) {
    return this->net->sendRequest(data);
}

std::error_code ServerModel::initialise(uint64_t uuid, ClientMeta meta) {
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
        net->sendRequest("/online");
        updateMeta(meta);
    } else {
        emit initialised(this, false);
    }
    return ret;
}

std::error_code ServerModel::connect(ClientMeta meta) {
    std::error_code ret = net->connect(ip, port);
    if (!ret) {
        net->sendRequest("/login " + std::to_string(uuid));
        net->sendRequest("/get_all_metadata");
        net->sendRequest("/get_channels");
        net->sendRequest("/online");
        updateMeta(meta);
    } else {
        emit initialised(this, false);
    }
    return ret;
}

std::error_code ServerModel::updateMeta(ClientMeta meta) {
    std::error_code ec;
    ec = net->sendRequest(("/nick " + meta.uname).toUtf8().constData());
    if (ec) return ec;
    ec = net->sendRequest(("/passwd " + meta.passwd).toUtf8().constData());
    if (ec) return ec;
    ec = net->sendRequest(("/pfp " + meta.pfp_b64).toUtf8().constData());
    return ec;
}

void ServerModel::addMessage(Message* msg) {
    messages->addMessage(msg);
}

void ServerModel::addChannel(std::string name) {
	QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(name), channels);
	QLabel* l = new QLabel(QString::fromStdString(name)); //TODO this is a memory leak
	channelWidgets.push_back(l);
	l->setProperty("unread", false);
	l->setProperty("selected", false);
    channels->addItem(item);
    channels->setItemWidget(item, l);
}

void ServerModel::handleNetwork(QString data) {
    json msg = json::parse(data.toUtf8().constData());
    std::cout << data.toUtf8().constData() << "\n";
    if (!msg["history"].is_null()) {
        uint32_t pos = 0;
        for (auto &elem : msg["history"]) {
            messages->insertMessage(pos++, new Message(this,
                            peers[elem["author_uuid"].get<uint64_t>()],
                            QString::fromStdString(elem["content"].get<std::string>()),
                            peers[elem["author_uuid"].get<uint64_t>()].pfp,
                            elem["date"].get<int64_t>()));
        }
    } else if (!msg["command"].is_null()) {
        if (msg["command"].get<std::string>() == "set") {
            if (msg["key"].get<std::string>() == "self_uuid") {
                uuid = msg["value"].get<uint64_t>();
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
        } else if (msg["command"].get<std::string>() == "unread") {
        	std::string channel = msg["channel"].get<std::string>();
        	for (QLabel* l : channelWidgets) { //TODO really inefficient lol
        		if (l->text().toUtf8().constData() == channel) {
        			l->setProperty("unread", true);
    				l->style()->polish(l);
        		}
        	}
        } else if (msg["command"].get<std::string>() == "online") {
            online->clear();
            for (auto &elem : msg["data"]) {
                if (peers.count(elem.get<uint64_t>()) == 0) continue;
                Metadata &m = peers[elem.get<uint64_t>()];
                online->addProfile(new SmallProfile(QString::fromStdString(m.uname), m.pfp));
            }
        }
        if (!isInitialised) {
            if (uuid != 0 && name != "" && pfp_b64 != "") {
                emit initialised(this, true);
                isInitialised = true;
            }
        }
    } else if (!msg["content"].is_null()) {
        messages->addMessage(new Message(this,
            peers[msg["author_uuid"].get<uint64_t>()],
            QString::fromStdString(msg["content"].get<std::string>()),
            peers[msg["author_uuid"].get<uint64_t>()].pfp,
            msg["date"].get<int64_t>()));
    } else {
        //???
        //ignore for now
    }
}
