#include "mainwindow.h"
#include "messagecontainer.h"
#include "newserverview.h"
#include "message.h"
#include "servermodel.h"
#include "serverbutton.h"

#include <fstream>

#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedLayout>

MainWindow::MainWindow() {
    std::ifstream ifs_preferences("preferences.json");
    std::string content((std::istreambuf_iterator<char>(ifs_preferences)),
                        (std::istreambuf_iterator<char>()));
    json value = json::parse(content);

    serverButtonLayout = new QHBoxLayout();
    serverContentLayout = new QStackedLayout();

    for (auto &elem: value["servers"]) {
        ServerModel* server = new ServerModel(
            elem["name"].get<std::string>(),
            elem["ip"].get<std::string>(),
            elem["port"].get<uint16_t>(),
            elem["uuid"].get<uint64_t>(),
            elem["pfp"].get<std::string>()
        );
        servers.push_back(server);
        server->connect();
            
        ServerButton *button = new ServerButton(servers[servers.size() - 1], this);
        serverButtonLayout->addWidget(button);
        serverContentLayout->addWidget(server);
        serverButtons.push_back(button);
    }

    serverButtonLayout->addStretch(1);
    serverButtonLayout->setSpacing(0);

    addServerButton = new QPushButton("+", this);
    serverButtonLayout->addWidget(addServerButton);

    setWindowTitle("Aster experimental GUI client");
    layout = new QVBoxLayout();
            
    nsv = new NewServerView();
    serverContentLayout->addWidget(nsv);
    
    input = new QLineEdit();
    layout->addLayout(serverButtonLayout);
    layout->addLayout(serverContentLayout);
    layout->addWidget(input);
    connect(input, &QLineEdit::returnPressed, this, &MainWindow::handleButton);
    connect(addServerButton, &QPushButton::clicked, this, &MainWindow::openNewServerView);
    connect(nsv, &NewServerView::backPressed, this, &MainWindow::closeNewServerView);
    connect(nsv, &NewServerView::connectPressed, this, &MainWindow::addNewServer);
    setLayout(layout);
    input->setFocus();
    if (serverButtons.size() > 0) {
        handleServerClick(serverButtons[0]);
    }
    show();
}

void MainWindow::save() {
    json result = json::object();
    result["servers"] = json::array();
    for (ServerModel* server : servers) {
        json obj = json::object();
        obj["name"] = server->name;
        obj["ip"] = server->ip;
        obj["port"] = server->port;
        obj["uuid"] = server->uuid;
        obj["pfp"] = server->pfp_b64;
        result["servers"].push_back(obj);
    }
    std::ofstream os("preferences.json");
    os << result.dump();
}

void MainWindow::handleServerClick(ServerButton* button) {
    for (size_t i = 0; i < serverButtons.size(); i++) {
        ServerButton* b = serverButtons[i];
        if (b != button) {
            b->blockSignals(true);
            b->setChecked(false);
            b->blockSignals(false);
        } else {
            //sneaky trick to find the index in the same loop
            selectedServer = i;
            serverContentLayout->setCurrentIndex(selectedServer);
        }
    }
}

void MainWindow::handleButton() {
    servers[selectedServer]->sendRequest(input->text().toUtf8().constData());
    servers[selectedServer]->addMessage(new Message(servers[selectedServer]->getName(), input->text(), servers[selectedServer]->getPfp()));
    input->setText("");
}

void MainWindow::addNewServer(QString ip, uint16_t port) {
    ServerModel* server = new ServerModel(
        "",
        ip.toUtf8().constData(),
        port,
        -1, 
        ""
    );
    connect(server, &ServerModel::initialised, this, &MainWindow::onServerInitialised);
    server->initialise();
}

void MainWindow::onServerInitialised(ServerModel* server) {
    servers.push_back(server);
    
    ServerButton *button = new ServerButton(servers[servers.size() - 1], this);
    serverButtonLayout->insertWidget(servers.size() - 1, button);
    serverContentLayout->insertWidget(servers.size() - 1, server);
    serverButtons.push_back(button);
}

void MainWindow::openNewServerView() {
    serverContentLayout->setCurrentIndex(servers.size());
}

void MainWindow::closeNewServerView() {
    serverContentLayout->setCurrentIndex(selectedServer);
}
