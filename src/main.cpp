#include <QApplication>
#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QString>
#include <QObject>
#include <QFrame>
#include <QPixmap>
#include <QScrollBar>
#include <QByteArray>
#include <QPushButton>
#include <QStyleOption>
#include <QStyle>
#include <QPainter>

#include <vector>
#include <iostream>
#include <fstream>

#include "network.h"
#include "serverbutton.h"
#include "base64.h"

using asio::ip::tcp;


void ClientNetwork::sendRequest(std::string request) {
    asio::error_code error;

    asio::write(socket, asio::buffer(request + "\n"), error);
    if (error) {
        std::cout << "ERROR SENDING: " << error.message() << "\n";
    }
}

void ClientNetwork::handleNetworkPacket(std::string data) {
    emit msgRecvd(QString::fromStdString(data));
}

void ClientNetwork::connect(std::string address, uint16_t port) {
    asio::error_code ec;

    asio::ip::tcp::resolver resolver(ctx);
    auto endpoint = resolver.resolve(address, std::to_string(port));
    asio::connect(socket.next_layer(), endpoint);
    socket.handshake(asio::ssl::stream_base::client);

    readUntil();
    std::thread asioThread = std::thread([&]() {ctx.run();});
    asioThread.detach();
}

void ClientNetwork::handler(std::error_code ec, size_t bytes_transferred) {
    std::cout << bytes_transferred << "\n";
    if (!ec) {
    
        std::string data(
                buffers_begin(buf.data()),
                buffers_begin(buf.data()) + (bytes_transferred
                  - 1));
                  
        buf.consume(bytes_transferred);
        readUntil();

        handleNetworkPacket(data);

    } else {
        std::cerr << "ERROR: " <<  ec.message() << "\n";
        readUntil();
    }
}

void ClientNetwork::readUntil() {
    asio::async_read_until(socket, buf, '\n', [this] (std::error_code ec, std::size_t bytes_transferred) {
        handler(ec, bytes_transferred);
    });
}

class Message : public QWidget {
    QLabel *content;
    QLabel *pfp;
    QGridLayout *layout;
public:
    Message(QString unamestr, QString cont, QPixmap *pfpPixmap) {
        layout = new QGridLayout();
        content = new QLabel(" " + unamestr + ": " + cont);
        
        pfp = new QLabel();
        pfp->setFixedWidth(32);
        pfp->setFixedHeight(32);
        pfp->setPixmap(pfpPixmap->scaledToWidth(32));

        content->setWordWrap(true);
        layout->setSpacing(0);
        layout->addWidget(pfp, 0, 0, 1, 1);
        layout->addWidget(content, 0, 1, 1, 1);
        setLayout(layout);
        layout->setRowStretch(1, 1);
        layout->setColumnStretch(1, 1);
    }
};

class MessageContainer : public QScrollArea {
    QVBoxLayout *layout;
    QWidget *widget;
    std::vector<Message*> messages;
public:
    MessageContainer() {
        layout = new QVBoxLayout();
        widget = new QWidget();

        QScrollBar* scrollbar = verticalScrollBar();
        connect(scrollbar, &QScrollBar::rangeChanged, this, &MessageContainer::sliderRangeChanged);

        layout->addStretch(1);
        layout->setSpacing(0);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setWidgetResizable(true);
        widget->setLayout(layout);
        this->setWidget(widget);
    }

    void addMessage(Message* msg) {
        messages.push_back(msg);
        layout->addWidget(msg);
    }

    void insertMessage(uint32_t idx, Message* msg) {
        auto itPos = messages.begin() + idx;
        messages.insert(itPos, msg);
        layout->insertWidget(idx + 1, msg);
    }

public slots:
    void sliderRangeChanged(int min, int max) {
        (void)min;
        QScrollBar *bar = verticalScrollBar();
        bar->setValue(max);
    }
};

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

std::string formatStyleSheets(std::string ssheet) {
    std::string dark = "#2a2a2a";
    std::string light = "#555555";
    std::string fg = "#cccccc";
    std::string bg = "#333333";
    return replaceAll(replaceAll(replaceAll(replaceAll(ssheet, "{fg}", fg), "{bg}", bg), "{light}", light), "{dark}", dark);
}

struct Metadata {
    uint64_t uuid;
    std::string uname;
    std::string pfp_b64;
    QPixmap *pfp;
    static Metadata from_json(json value) {
        std::string pfp_b64_json = value["pfp"].get<std::string>();
        std::vector<uint8_t> buf = base64_decode(pfp_b64_json);
        QByteArray data = QByteArray((const char*)buf.data(), (int)buf.size());
        QPixmap* pixMap = new QPixmap();
        pixMap->loadFromData(data, "PNG");
        return {value["uuid"].get<uint64_t>(),
                value["name"].get<std::string>(),
                pfp_b64_json,
                pixMap
                };
    }

    void update(json value) {
        uuid    = value["uuid"].get<uint64_t>();
        uname   = value["name"].get<std::string>();
        pfp_b64 = value["pfp"].get<std::string>();
        
        std::vector<uint8_t> buf = base64_decode(pfp_b64);
        QByteArray data = QByteArray((const char*)buf.data(), (int)buf.size());
        pfp->loadFromData(data, "PNG");
    }
};

struct ServerModel {
    std::string name;
    std::string ip;
    uint64_t uuid;
    uint16_t port;
    QPixmap pfp;
    std::unordered_map<uint64_t, Metadata> peers = {};
    ClientNetwork* net;
    ServerModel(std::string name, std::string ip, uint64_t uuid, uint16_t port, std::string pfp_b64) {
        this->name = name;
        this->ip = ip;
        this->uuid = uuid;
        this->port = port;
        std::vector<uint8_t> buf = base64_decode(pfp_b64);
        QByteArray data = QByteArray((const char*)buf.data(), (int)buf.size());
        pfp.loadFromData(data, "PNG");
    }
    void handleNetwork(QString data, MainWindow *parent);
    QString getName() {
        return QString::fromStdString(peers[uuid].uname);
    }
    QPixmap *getPfp() {
        std::cout << uuid << "\n";
        return peers[uuid].pfp;
    }

    void sendRequest(std::string data) {
        this->net->sendRequest(data);
    }
};

ServerButton::ServerButton(ServerModel* server, MainWindow* parent) {
    //setText(QString::fromStdString(name));
    this->server = server;
    this->parent = parent;
    setIcon(server->pfp);
    setIconSize(server->pfp.rect().size());
    setCheckable(true);
    connect(this, &QAbstractButton::toggled, this, &ServerButton::handleClick);
}

class MainWindow : public QWidget {
    MessageContainer *cont;
    QLineEdit *input;
    QVBoxLayout *layout;
    QHBoxLayout *serverLayout;
    std::vector<ServerButton*> serverButtons;
    std::vector<ServerModel*> servers;
    size_t selectedServer;
public:
    MainWindow() {

        std::ifstream ifs_preferences("preferences.json");
        std::string content((std::istreambuf_iterator<char>(ifs_preferences)),
                            (std::istreambuf_iterator<char>()));
        json value = json::parse(content);

        serverLayout = new QHBoxLayout();

        for (auto &elem: value["servers"]) {
            servers.push_back(new ServerModel(
                elem["name"].get<std::string>(),
                elem["ip"].get<std::string>(),
                elem["port"].get<uint16_t>(),
                elem["uuid"].get<uint64_t>(),
                elem["pfp"].get<std::string>())
            );
                
            ServerButton *button = new ServerButton(servers[servers.size() - 1], this);
            serverLayout->addWidget(button);
            serverButtons.push_back(button);
        }

        serverLayout->addStretch(1);
        serverLayout->setSpacing(0);

        setWindowTitle("Aster experimental GUI client");
        layout = new QVBoxLayout();
        cont = new MessageContainer();
        
        input = new QLineEdit();
        layout->addLayout(serverLayout);
        layout->addWidget(cont);
        layout->addWidget(input);
        connect(input, &QLineEdit::returnPressed, this, &MainWindow::handleButton);
        //
        setLayout(layout);
        input->setFocus();
        show();

    }

    void addMessage(Message *msg) {
        cont->addMessage(msg);
    }

    void insertMessage(uint32_t pos, Message *msg) {
        cont->insertMessage(pos, msg);
    }

    void handleServerClick(ServerButton* button) {
        for (size_t i = 0; i < serverButtons.size(); i++) {
            ServerButton* b = serverButtons[i];
            if (b != button) {
                b->blockSignals(true);
                b->setChecked(false);
                b->blockSignals(false);
            } else {
                //sneaky trick to find the index in the same loop
                selectedServer = i;
            }
        }
    }

public slots:
    void handleButton() {
        servers[selectedServer]->sendRequest(input->text().toUtf8().constData());
        cont->addMessage(new Message(servers[selectedServer]->getName(), input->text(), servers[selectedServer]->getPfp()));
        input->setText("");
    }

    void handleNetwork(QString data) {
        servers[selectedServer]->handleNetwork(data, this);
    }
};

void ServerButton::handleClick(bool n) {
    if (n) {
        parent->handleServerClick(this);
    } else {
        blockSignals(true);
        setChecked(true);
        blockSignals(false);
    }
    //TODO make this a signal/slot?
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

int main(int argc, char *argv[]) {
    
    QApplication app(argc, argv);
    std::ifstream ifs("stylesheet.qss");
    std::string ss((std::istreambuf_iterator<char>(ifs)),
                   (std::istreambuf_iterator<char>()));
    
    app.setStyleSheet(QString::fromStdString(formatStyleSheets(ss)));
    MainWindow window();

    return app.exec();
}
