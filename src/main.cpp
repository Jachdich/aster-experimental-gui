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
#include <vector>

#include <iostream>
#include <fstream>

#include "network.h"

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
    Message(QString unamestr, QString cont) {
        layout = new QGridLayout();
        content = new QLabel(" " + unamestr + ": " + cont);
        
        pfp = new QLabel();
        pfp->setFixedWidth(32);
        pfp->setFixedHeight(32);
        pfp->setPixmap(QPixmap("test.png").scaledToWidth(32));

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
        //Q_UNUSED(min);
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

class MainWindow : public QWidget {
    MessageContainer *cont;
    QLineEdit *input;
    QVBoxLayout *layout;
    ClientNetwork* net;
public:
    MainWindow(ClientNetwork* network) {
        net = network;

        setStyleSheet(QString::fromStdString(formatStyleSheets(R"(QWidget {
            background-color: {bg};
            color: {fg};
        }
        
        QTreeView {
            background-color: {bg};
            color: {fg};
        }
        
        QScrollBar:vertical {
            border: 2px solid {dark};
            border-right: 1px solid {dark};
            background: {bg};
            width: 15px;
            margin: 0px 0px 0px 0px;
        }
        
        QScrollBar::handle:vertical {
            background: {light};
            min-height: 20px;
            border-radius: 5px;
        }
        
        QScrollBar::add-line:vertical {
            border: 0px solid {dark};
            background: {bg};
            height: 0px;
            subcontrol-position: bottom;
            subcontrol-origin: margin;
        }
        
        QScrollBar::sub-line:vertical {
            border: 0px solid {dark};
            background: {bg};
            height: 0px;
            subcontrol-position: top;
            subcontrol-origin: margin;
        }
        
        QScrollBar:horizontal {
            border: 2px solid {dark};
            border-bottom: 1px solid {dark};
            background: {bg};
            height: 15px;
            margin: 0px 0px 0px 0px;
        }
        
        QScrollBar::handle:horizontal {
            background: {light};
            min-width: 20px;
            border-radius: 5px;
        }
        
        QScrollBar::add-line:horizontal {
            border: 0px solid black;
            background: #000000;
            width: 0px;
            subcontrol-position: right;
            subcontrol-origin: margin;
        }
        
        QScrollBar::sub-line:horizontal {
            border: 0px solid black;
            background: #000000;
            width: 0px;
            subcontrol-position: left;
            subcontrol-origin: margin;
        }
        
        QFrame {
            background-color: {dark};
            color: {fg};
        }
        
        QMenuBar {
            background-color: {bg};
            color: {fg};
        }
        
        QTabWidget {
            background-color: {bg};
            color: {fg};
        }
        
        QTabBar {
            color: {fg};
            background-color: {bg};
        }
        
        QPlainTextEdit {
            background-color: {bg};
            color: {fg};
            font-family: Courier;
        /*    font-size: 18px;*/
        }
        
        
        QPushButton {
            background-color: {bg};
            color: {fg};
        }
        
        QScrollArea {
            background-color: {bg};
            color: {fg};
        }
        
        QLabel {
            font-size: 12px;
        /*    padding: 2px; TODO make this work*/
        })")));
        
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

public slots:
    void handleButton() {
        net->sendRequest(input->text().toUtf8().constData());
        cont->addMessage(new Message("KingJellyfish", input->text()));
        input->setText("");
    }

    void handleNetwork(QString data) {
        json msg = json::parse(data.toUtf8().constData());
        //std::cout << data.toUtf8().constData() << "\n";
        if (!msg["res"].is_null()) {
            uint32_t pos = 0;
            for (auto &elem : msg["res"]) {
                cont->insertMessage(pos++, new Message(
                    QString::fromStdString(elem["user"]["name"].get<std::string>()), //TODO make other one like this
                    QString::fromStdString(elem["content"].get<std::string>())));
            }
        } else if (!msg["content"].is_null()) {
            cont->addMessage(new Message(
                QString::fromStdString(msg["user"]["name"].get<std::string>()),
                QString::fromStdString(msg["content"].get<std::string>())));
        } else {
            //???
            //ignore for now
        }
    }
};

int main(int argc, char *argv[]) {
    ClientNetwork network;
    network.connect("127.0.0.1", 2345);
    
    QApplication app(argc, argv);
    MainWindow window(&network);

    return app.exec();
}
