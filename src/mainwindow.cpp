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
#include <QMessageBox>
#include <iostream>

MainWindow::MainWindow() {
    std::ifstream lock("preferences.lock");
    if (lock.good()) {
        safeToSave = false;
        QMessageBox msg;
        msg.setText("Another instance of aster is already running!");
        msg.exec();
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
/*
            QMessageBox msg;
            msg.setText(QString::fromStdString(
                "Error: " + error.message() + " whilst connecting to server " +
                elem["name"].get<std::string>() + " (" + elem["ip"].get<std::string>() + ":" + std::to_string(elem["port"].get<uint16_t>()) + ")"));
            msg.exec();*/
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

    setWindowTitle("Aster experimental GUI client 0.0.3a-dev");
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
    std::error_code ec = servers[selectedServer]->sendRequest(input->text().toUtf8().constData());
    if (!ec) {
        servers[selectedServer]->addMessage(new Message(servers[selectedServer]->getName(), input->text(), servers[selectedServer]->getPfp()));
        input->setText("");
    } else {
        QMessageBox msg;
        msg.setText(QString::fromStdString(
            "Error: " + ec.message() + " whilst sending message to " + servers[selectedServer]->ip + ":" + std::to_string(servers[selectedServer]->port)));
        msg.exec();
    }
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
	target->server->deleteLater();
	target->deleteLater();
}

void MainWindow::addNewServer(QString ip, uint16_t port, uint64_t uuid) {
    for (ServerModel* s : servers) {
        if (s->ip == ip.toUtf8().constData() && s->port == port) {
            QMessageBox msg;
            msg.setText("Error: Attempt to connect to the same server twice");
            msg.exec();
            return;
        }
    }
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
        //onServer
        QMessageBox msg;
        msg.setText(QString::fromStdString(
            "Error: " + error.message() + " whilst connecting to server " + ip.toUtf8().constData() + ":" + std::to_string(port)));
        msg.exec();
    }
    closeNewServerView();
}

void MainWindow::onServerInitialised(ServerModel* server, bool active) {
    servers.push_back(server);

    ServerButton *button = new ServerButton(servers[servers.size() - 1], this, active);
    serverButtonLayout->insertWidget(servers.size() - 1, button);
    serverContentLayout->insertWidget(servers.size() - 1, server);
    serverButtons.push_back(button);
    connect(button, &ServerButton::remove, this, &MainWindow::deleteServerButton);
    connect(button, &ServerButton::serverClicked, this, &MainWindow::handleServerClick);

    if (active) {
        server->sendRequest("/history 200");
    }
}

void MainWindow::openNewServerView() {
    nsv->show();
}

void MainWindow::closeNewServerView() {
    nsv->hide();
}
