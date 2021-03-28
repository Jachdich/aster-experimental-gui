#include "mainwindow.h"
#include "messagecontainer.h"
#include "newserverview.h"
#include "message.h"
#include "servermodel.h"
#include "serverbutton.h"

#include <fstream>

//#include <QWidget>
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

    serverLayout = new QHBoxLayout();
    stackedLayout = new QStackedLayout();

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

    addServerButton = new QPushButton("+", this);
    serverLayout->addWidget(addServerButton);

    setWindowTitle("Aster experimental GUI client");
    layout = new QVBoxLayout();
    stackedLayout = new QStackedLayout();
            
    cont = new MessageContainer();
    nsv  = new NewServerView();
    stackedLayout->addWidget(cont);
    stackedLayout->addWidget(nsv);
    
    input = new QLineEdit();
    layout->addLayout(serverLayout);
    layout->addLayout(stackedLayout);
    layout->addWidget(input);
    connect(input, &QLineEdit::returnPressed, this, &MainWindow::handleButton);
    connect(addServerButton, &QPushButton::clicked, this, &MainWindow::addServer);
    setLayout(layout);
    input->setFocus();
    show();
}

void MainWindow::addMessage(Message *msg) {
    cont->addMessage(msg);
}

void MainWindow::insertMessage(uint32_t pos, Message *msg) {
    cont->insertMessage(pos, msg);
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
    cont->addMessage(new Message(servers[selectedServer]->getName(), input->text(), servers[selectedServer]->getPfp()));
    input->setText("");
}

void MainWindow::addServer() {
    addNewServerSelected = !addNewServerSelected;
    stackedLayout->setCurrentIndex(addNewServerSelected);
}
