#include "mainwindow.h"
#include "messagecontainer.h"
#include "newserverview.h"
#include "message.h"
#include "main.h"
#include "servermodel.h"
#include "serverbutton.h"
#include "settingsmenu.h"

#include <chrono>
#include <fstream>
#include <filesystem>

#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QFocusEvent>
#include <QAction>
#include <QSplitter>
#include <iostream>

MainWindow::MainWindow() {
    std::ifstream lock(prefpath + pathsep + "preferences.lock");
    if (lock.good()) {
        safeToSave = false;
        QMessageBox msg;
        msg.setText("Another instance of aster is already running!");
        msg.exec();
        return;
    }

    std::ofstream file { prefpath + pathsep + "preferences.lock" };
    file.close();
    
    std::ifstream ifs_preferences(prefpath + pathsep + "preferences.json");
    std::string defaultPfp = "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";
    default_server_pfp = defaultPfp;
    //Yes, I did just insert a base64 image into the code
    //do you have a problem?

    json value;
    value = json::object();
    value["servers"] = json::array();
    value["uname"] = "";
    value["passwd"] = "";
    value["pfp"] = defaultPfp;
    value["chan_split"] = 128;
    value["online_split"] = 196;

    if (ifs_preferences.good()) {
        std::string content((std::istreambuf_iterator<char>(ifs_preferences)),
                            (std::istreambuf_iterator<char>()));
        try {
            json parsed_value = json::parse(content);
            for (const auto &j : parsed_value.items()) {
                value[j.key()] = j.value();
            }
        } catch (json::parse_error &e) {
            //ignore lmao
        }
    }

    chan_split = value["chan_split"].get<int>();
    online_split = value["online_split"].get<int>();

    meta.uname   = QString::fromStdString(value["uname"].get<std::string>());
    meta.pfp_b64 = QString::fromStdString(value["pfp"].get<std::string>());
    meta.passwd  = QString::fromStdString(value["passwd"].get<std::string>());

    serverButtonLayout = new QHBoxLayout();
    serverContentLayout = new QStackedLayout();

    for (auto &elem: value["servers"]) {
        ServerModel* server = new ServerModel(this,
            elem["name"].get<std::string>(),
            elem["ip"].get<std::string>(),
            elem["port"].get<uint16_t>(),
            elem["uuid"].get<uint64_t>(),
            elem["pfp"].get<std::string>(),
            chan_split,
            online_split
        );
        //connect(server, &ServerModel::initialised, this, &MainWindow::onServerInitialised);
        connect(server, &ServerModel::splitChanged, this, &MainWindow::splitChanged);

        servers.push_back(server);
        ServerButton *button = new ServerButton(servers[servers.size() - 1], this, false);
        serverButtonLayout->insertWidget(servers.size() - 1, button);
        serverContentLayout->insertWidget(servers.size() - 1, server);
        serverButtons.push_back(button);
        connect(button, &ServerButton::remove, this, &MainWindow::deleteServerButton);
        connect(button, &ServerButton::serverClicked, this, &MainWindow::handleServerClick);
        connect(server->net, &ClientNetwork::onlineChanged, button, &ServerButton::onlineChanged);
        
        std::error_code error = server->connect(meta);
        if (error) {
            std::cout << "Error for some reason: " << error.message() << "\n";
            //HEHE IGNORE IT LOL
        }
    }

    serverButtonLayout->addStretch(1);
    serverButtonLayout->setSpacing(0);

    addServerButton = new QPushButton(this);
    settingsButton  = new QPushButton(this);

    addServerButton->setIcon(QIcon(QString::fromStdString(respath + pathsep + "add.png")));
    addServerButton->setToolTip("Add server");

    settingsButton->setIcon(QIcon(QString::fromStdString(respath + pathsep + "settings.png")));
    settingsButton->setToolTip("Settings");

    serverButtonLayout->addWidget(addServerButton);
    serverButtonLayout->addWidget(settingsButton);

    setWindowTitle("Aster experimental GUI client 0.0.6a-dev");
    layout = new QVBoxLayout(this);

    nsv = new NewServerView();
    settings = new SettingsMenu(&meta);

    input = new QLineEdit();
    layout->addLayout(serverButtonLayout);
    layout->addLayout(serverContentLayout);
    layout->addWidget(input);
    connect(input, &QLineEdit::returnPressed, this, &MainWindow::handleButton);
    connect(addServerButton, &QPushButton::clicked, this, &MainWindow::openNewServerView);
    connect(settingsButton,  &QPushButton::clicked, this, &MainWindow::openSettings);
    connect(nsv, &NewServerView::backPressed, this, &MainWindow::closeNewServerView);
    connect(nsv, &NewServerView::connectPressed, this, &MainWindow::addNewServer);

    connect(settings, &SettingsMenu::unameChanged, this, [this](QString newVal) { this->meta.uname = newVal; this->updateMeta(); });
    connect(settings, &SettingsMenu::passwdChanged, this, [this](QString newVal) { this->meta.passwd = newVal; this->updateMeta(); });
    connect(settings, &SettingsMenu::pfpChanged, this, [this](QString newVal) { this->meta.pfp_b64 = newVal; this->updateMeta(); });
    setLayout(layout);
    input->setFocus();
    show();
}

void MainWindow::splitChanged(int chan, int online) {
    chan_split = chan;
    online_split = online;
    for (ServerModel* server : this->servers) {
        server->splitter->setSizes({chan, INT_MAX, online});
    }
}

void MainWindow::focusInEvent() {
    servers[selectedServer]->isInBackground = false;
}
void MainWindow::focusOutEvent() {
    servers[selectedServer]->isInBackground = true;
}

void MainWindow::updateMeta() {
    for (ServerModel* server : this->servers) {
        if (server->isInitialised) {
            std::error_code ec = server->updateMeta(meta);
            if (ec) 
                std::cout << ec.message() << "\n";
        }
    }
}

void MainWindow::save() {
    std::cout << safeToSave << "\n";
    if (!safeToSave) return;
    json result = json::object();
    result["uname"] = meta.uname.toUtf8().constData();
    result["passwd"] = meta.passwd.toUtf8().constData();
    result["pfp"] = meta.pfp_b64.toUtf8().constData();
    result["chan_split"] = chan_split;
    result["online_split"] = online_split;
    
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
    std::ofstream os(prefpath + pathsep + "preferences.json");
    os << result.dump();
    std::ifstream lock(prefpath + pathsep + "preferences.lock");
    if (lock.good()) {
	    lock.close();
        std::filesystem::remove(std::filesystem::path(prefpath + pathsep + "preferences.lock"));
    } else {
        std::cout << "Something horrible happened, and preferences.lock was removed while the program is running! RISK OF CORRUPTION!\n";
        //we got some serious avengers level threat if the file has been removed before the program has terminated
    }
}

void MainWindow::handleServerClick(ServerButton* button) {
    for (size_t i = 0; i < serverButtons.size(); i++) {
        ServerButton* b = serverButtons[i];
        if (b != button) {
            b->blockSignals(true);
            b->setChecked(false);
            b->server->isInBackground = true;
            b->blockSignals(false);
        } else {
            //sneaky trick to find the index in the same loop
            selectedServer = i;
            serverContentLayout->setCurrentIndex(selectedServer);
            b->server->isInBackground = false;
        }
    }
}

void MainWindow::handleButton() {
	if (input->text().remove(' ').isEmpty()) {
		return;
	}
    std::error_code ec = servers[selectedServer]->sendRequest(input->text().toUtf8().constData());
    if (!ec) {
        int64_t p1 = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch()).count();
        servers[selectedServer]->addMessage(new Message(this, servers[selectedServer]->getMeta(), input->text(), servers[selectedServer]->getPfp(), p1));
        input->setText("");
    } else {
        QMessageBox msg;
        msg.setText(QString::fromStdString(
            "Error: " + ec.message() + " whilst sending message to " + servers[selectedServer]->ip + ":" + std::to_string(servers[selectedServer]->port)));
        msg.exec();
    }
}

void MainWindow::deleteServerButton(ServerButton* target) {
	servers.erase(std::remove(this->servers.begin(), this->servers.end(), target->server));
	serverButtons.erase(std::remove(this->serverButtons.begin(), this->serverButtons.end(), target));
	serverButtonLayout->removeWidget(target);
	serverContentLayout->removeWidget(target->server);
	target->server->deleteLater();
	target->deleteLater();
}

void MainWindow::addNewServer(QString ip, uint16_t port, uint64_t uuid) {
	if (meta.uname == "" || meta.passwd == "") {
		QMessageBox msg;
		msg.setText("Please choose a username and password in the settings menu first!");
		msg.exec();
		return;
	}
    for (ServerModel* s : servers) {
        if (s->ip == ip.toUtf8().constData() && s->port == port) {
            QMessageBox msg;
            msg.setText("Error: Attempt to connect to the same server twice");
            msg.exec();
            return;
        }
    }
    ServerModel* server = new ServerModel(this,
        "",
        ip.toUtf8().constData(),
        port,
        uuid,
        default_server_pfp,
        chan_split,
        online_split
    );
    connect(server, &ServerModel::splitChanged, this, &MainWindow::splitChanged);
    
    servers.push_back(server);
    ServerButton *button = new ServerButton(servers[servers.size() - 1], this, false);
    serverButtonLayout->insertWidget(servers.size() - 1, button);
    serverContentLayout->insertWidget(servers.size() - 1, server);
    serverButtons.push_back(button);
    connect(button, &ServerButton::remove, this, &MainWindow::deleteServerButton);
    connect(button, &ServerButton::serverClicked, this, &MainWindow::handleServerClick);
    connect(server->net, &ClientNetwork::onlineChanged, button, &ServerButton::onlineChanged);
    
    std::error_code error = server->initialise(uuid, meta);

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
    
}

void MainWindow::openNewServerView() {
    nsv->show();
}

void MainWindow::closeNewServerView() {
    nsv->hide();
}

void MainWindow::openSettings() {
    settings->show();
}

void MainWindow::closeSettings() {
    settings->hide();
}
