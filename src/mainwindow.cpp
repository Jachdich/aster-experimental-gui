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
    show();
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
    server->initialise();

    //while (!server->initialised); //TODO VERY BAD IDEA!

    servers.push_back(server);
    
    ServerButton *button = new ServerButton(servers[servers.size() - 1], this);
    serverButtonLayout->insertWidget(-2, button);
    serverContentLayout->insertWidget(-1, server);
    serverButtons.push_back(button);
}

void MainWindow::openNewServerView() {
    serverContentLayout->setCurrentIndex(-1);
}

void MainWindow::closeNewServerView() {
    serverContentLayout->setCurrentIndex(selectedServer);
}
