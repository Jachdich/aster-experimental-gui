#include "serverbutton.h"

#include "mainwindow.h"
#include "main.h"
#include "servermodel.h"
#include "base64.h"

#include <QMenu>
#include <Qt>
#include <QPoint>
#include <QAction>
#include <QGridLayout>
#include <QLineEdit>
#include <QPainter>
#include <vector>

ServerButton::ServerButton(ServerModel* server, MainWindow* parent, bool active) {
    this->server = server;
    this->parent = parent;
    this->active = active;

    if (active) {
        setOnline();
    } else {
        setOffline();
    }

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QPushButton::customContextMenuRequested, this, &ServerButton::onContextMenu);

    menu = new QMenu(this);

    rem  = new QAction("Remove server",   this);
    del  = new QAction("Delete account",  this);
    menu->addAction(rem);
    menu->addAction(del);

    connect(del,  &QAction::triggered, this, &ServerButton::deleteAccount);
    connect(rem,  &QAction::triggered, this, &ServerButton::removeServer);
    connect(this, &QAbstractButton::toggled, this, &ServerButton::handleClick);
}

ServerButton::~ServerButton() {
    delete menu;
    delete rem;
    delete del;
}

void ServerButton::setOnline() {
    setIcon(server->pfp);
    setToolTip(QString::fromStdString(server->name));
    setIconSize(server->pfp.rect().size());
    setCheckable(true);
    active = true;
}

void ServerButton::setOffline() {
    //setIcon(QIcon(QString::fromStdString(respath + pathsep + "server_offline.png")));
    //setToolTip("This server is offline");
    setToolTip(QString::fromStdString(server->name + "(" + server->ip + ":" + std::to_string(server->port) + " offline)"));
    setIconSize(server->pfp.rect().size());
    //setCheckable(false);

    QPixmap icon(server->pfp);
    QPixmap overlay(QString::fromStdString(respath + pathsep + "server_offline.png"));
    QPainter painter(&icon);
    painter.drawPixmap(0, 0, overlay);
    setIcon(icon);
    active = false;
}

void ServerButton::deleteAccount() {
    if (!active) return;
	server->sendRequest("/delete " + std::to_string(server->uuid));
	removeServer();
}

void ServerButton::removeServer() {
	emit remove(this);
}

void ServerButton::onContextMenu(const QPoint &point) {
    menu->exec(mapToGlobal(point));
}

void ServerButton::handleClick(bool n) {
    if (!active) {
        blockSignals(true);
        setChecked(false);
        blockSignals(false);
        return;
    }
    if (n) {
       emit serverClicked(this);
    } else {
        blockSignals(true);
        setChecked(true);
        blockSignals(false);
    }
    //TODO make this a signal/slot?
}

void ServerButton::onlineChanged(bool online) {
    if (online) { setOnline(); }
    else { setOffline(); }
}
