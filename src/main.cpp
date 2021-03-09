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
#include <vector>

#include <iostream>
#include <fstream>

#include "network.h"
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

void init(ClientNetwork *net) {
    std::ifstream ifs("preferences.json");
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    json value = json::parse(content);
    std::string uname = value["username"].get<std::string>();
    //net->sendRequest("/nick " + uname);
    net->sendRequest("/history 0 100");
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
   //     ensureWidgetVisible(msg);
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
};

class MainWindow;
class Client {
private:
    Metadata meta;
    std::unordered_map<uint32_t, Metadata> peers; //TODO per-server in future
public:
    Client() {
        meta.pfp = new QPixmap("test.png");
    }
    void handleNetwork(QString data, MainWindow *parent);
    QString getName() {
        return QString::fromStdString(meta.uname);
    }
    QPixmap *getPfp() {
        return meta.pfp;
    }
};

class MainWindow : public QWidget {
    MessageContainer *cont;
    QLineEdit *input;
    QVBoxLayout *layout;
    ClientNetwork* net;
    
public:
    Client client;
    MainWindow(ClientNetwork* network) {
        net = network;

        std::ifstream ifs("stylesheet.qss");
        std::string ss((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));
        
        setStyleSheet(QString::fromStdString(formatStyleSheets(ss)));
        
        init(network);
        setWindowTitle("Aster experimental GUI client");
        layout = new QVBoxLayout();
        cont = new MessageContainer();
        
        input = new QLineEdit();
        layout->addWidget(cont);
        layout->addWidget(input);
        connect(input, &QLineEdit::returnPressed, this, &MainWindow::handleButton);
        connect(network, &ClientNetwork::msgRecvd, this, &MainWindow::handleNetwork);
        setLayout(layout);
        show();
    }

    void addMessage(Message *msg) {
        cont->addMessage(msg);
    }

    void insertMessage(uint32_t pos, Message *msg) {
        cont->insertMessage(pos, msg);
    }

public slots:
    void handleButton() {
        net->sendRequest(input->text().toUtf8().constData());
        cont->addMessage(new Message(client.getName(), input->text(), client.getPfp()));
        input->setText("");
    }

    void handleNetwork(QString data) {
        client.handleNetwork(data, this);
    }
};

void Client::handleNetwork(QString data, MainWindow *parent) {
    json msg = json::parse(data.toUtf8().constData());
    std::cout << data.toUtf8().constData() << "\n";
    if (!msg["history"].is_null()) {
        uint32_t pos = 0;
        for (auto &elem : msg["history"]) {
            parent->insertMessage(pos++, new Message(
                QString::fromStdString(std::to_string(elem["user"].get<uint64_t>())), //TODO make other one like this
                QString::fromStdString(elem["content"].get<std::string>()),
                peers[msg["user"].get<uint64_t>()].pfp));
        }
    } else if (!msg["command"].is_null()) {
        if (msg["command"].get<std::string>() == "set") {
            if (msg["key"].get<std::string>() == "self_uuid") {
                meta.uuid = msg["value"].get<uint64_t>();
                std::cout << meta.uuid << "\n";
            }
        } else if (msg["command"].get<std::string>() == "metadata") {
            peers.clear();
            for (auto &elem : msg["data"]) {
                peers[elem["uuid"].get<uint64_t>()] = Metadata::from_json(elem);
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
    ClientNetwork network;
    network.connect("127.0.0.1", 2345);
    
    QApplication app(argc, argv);
    MainWindow window(&network);

    return app.exec();
}


//One 3193709851413311828
//two 1851050229929352744
