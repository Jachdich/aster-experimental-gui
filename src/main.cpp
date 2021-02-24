#include <QApplication>
#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QString>
#include <QObject>

#include <vector>

#include <iostream>
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

class Message : public QWidget {
    QLabel *content;
    QGridLayout *layout;
public:
    Message(QString cont) {
        layout = new QGridLayout();
        content = new QLabel(cont);
        layout->addWidget(content, 0, 0);
        setLayout(layout);
    }
};

class MessageContainer : public QWidget {
    QVBoxLayout *layout;
    std::vector<Message*> messages;
public:
    MessageContainer() {
        layout = new QVBoxLayout();
        setLayout(layout);
    }

    void addMessage(Message* msg) {
        messages.push_back(msg);
        layout->addWidget(msg);
    }
};

class MainWindow : public QWidget {
    MessageContainer *cont;
    QLineEdit *input;
    QScrollArea *scroll;
    QVBoxLayout *layout;
    ClientNetwork* net;
public:
    MainWindow(ClientNetwork* network) {
        net = network;
        setWindowTitle("Aster experimental GUI client");
        layout = new QVBoxLayout();
        cont = new MessageContainer();

        scroll = new QScrollArea();
        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setWidgetResizable(false);
        
        scroll->setWidget(cont);
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
        input->setText("");
    }

    void handleNetwork(QString msg) {
        cont->addMessage(new Message(msg));
    }
};

int main(int argc, char *argv[]) {
    ClientNetwork network;
    network.connect("192.168.1.131", 2345);
    
    QApplication app(argc, argv);
    MainWindow window(&network);

    return app.exec();
}
