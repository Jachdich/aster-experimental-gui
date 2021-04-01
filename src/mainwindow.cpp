#include "mainwindow.h"
#include "messagecontainer.h"
#include "newserverview.h"
#include "message.h"
#include "servermodel.h"
#include "serverbutton.h"
#include "errorpopup.h"

#include <fstream>

#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedLayout>
#include <iostream>

MainWindow::MainWindow() {
    std::ifstream lock("preferences.lock");
    if (lock.good()) {
        safeToSave = false;
        ErrorPopup* popup = new ErrorPopup("Another instance of aster is already running!");
        connect(popup, &ErrorPopup::dismissed, [=]() { delete popup; });
        popup->show();
        return;
    }

    std::ofstream file { "preferences.lock" };
    file.close();
    
    std::ifstream ifs_preferences("preferences.json");
    json value;
    if (ifs_preferences.good()) {
        std::string content((std::istreambuf_iterator<char>(ifs_preferences)),
                            (std::istreambuf_iterator<char>()));
        try {
            value = json::parse(content);
        } catch (json::parse_error &e) {
            value = json::object();
            value["servers"] = json::array();
        }
    }

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
        //servers.push_back(server);
        connect(server, &ServerModel::initialised, this, &MainWindow::onServerInitialised);
        std::error_code error = server->connect();
        if (error) {
            std::cout << "Error\n";
            delete server;
            ErrorPopup* popup = new ErrorPopup(QString::fromStdString(
                "Error: " + error.message() + " whilst connecting to server " +
                elem["name"].get<std::string>() + " (" + elem["ip"].get<std::string>() + ":" + std::to_string(elem["port"].get<uint16_t>()) + ")"));
            connect(popup, &ErrorPopup::dismissed, [=]() { delete popup; });
            popup->show();
        }
        /*
        ServerButton *button = new ServerButton(servers[servers.size() - 1], this);
        serverButtonLayout->addWidget(button);
        serverContentLayout->addWidget(server);
        serverButtons.push_back(button);*/
    }

    serverButtonLayout->addStretch(1);
    serverButtonLayout->setSpacing(0);

    addServerButton = new QPushButton("+", this);
    serverButtonLayout->addWidget(addServerButton);

    setWindowTitle("Aster experimental GUI client");
    layout = new QVBoxLayout();
            
    nsv = new NewServerView();
    
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
    std::cout << safeToSave << "\n";
    if (!safeToSave) return;
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
    std::ifstream lock("preferences.lock");
    if (lock.good()) {
        std::remove("preferences.lock");
    } else {
        //we got some serious avengers level threat if the file has been removed before the program has terminated
    }
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
	if (input->text().remove(' ').isEmpty()) {
		return;
	}
    servers[selectedServer]->sendRequest(input->text().toUtf8().constData());
    servers[selectedServer]->addMessage(new Message(servers[selectedServer]->getName(), input->text(), servers[selectedServer]->getPfp()));
    input->setText("");
}

void MainWindow::deleteServerButton(ServerButton* target) {
	std::cout << target << "\n";
	for (ServerButton* b : this->serverButtons) {
		std::cout << "bVec: " << b << "\n";
	}
	servers.erase(std::remove(this->servers.begin(), this->servers.end(), target->server));
	serverButtons.erase(std::remove(this->serverButtons.begin(), this->serverButtons.end(), target));
	serverButtonLayout->removeWidget(target);
	serverContentLayout->removeWidget(target->server);
	// delete target->server;
	// delete target; //TODO not a good idea
	//TODO there is a memory leak l
}

void MainWindow::addNewServer(QString ip, uint16_t port, uint64_t uuid) {
    ServerModel* server = new ServerModel(
        "",
        ip.toUtf8().constData(),
        port,
        uuid, 
        ""
    );
    connect(server, &ServerModel::initialised, this, &MainWindow::onServerInitialised);
    std::error_code error = server->initialise(uuid);
    if (error) {
        std::cout << "Error\n";
        delete server;
        ErrorPopup* popup = new ErrorPopup(QString::fromStdString(
            "Error: " + error.message() + " whilst connecting to server " + ip.toUtf8().constData() + ":" + std::to_string(port)));
        connect(popup, &ErrorPopup::dismissed, [=]() { delete popup; });
        popup->show();
    }
    closeNewServerView();
}

void MainWindow::onServerInitialised(ServerModel* server) {
    servers.push_back(server);
    
    ServerButton *button = new ServerButton(servers[servers.size() - 1], this);
    serverButtonLayout->insertWidget(servers.size() - 1, button);
    serverContentLayout->insertWidget(servers.size() - 1, server);
    serverButtons.push_back(button);
    std::cout << "Creating button " << button << "\n";
    connect(button, &ServerButton::remove, this, &MainWindow::deleteServerButton);
    server->sendRequest("/history 200");
}

void MainWindow::openNewServerView() {
    nsv->show();
}

void MainWindow::closeNewServerView() {
    nsv->hide();
}
