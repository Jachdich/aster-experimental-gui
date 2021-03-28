#include "serverbutton.h"

#include "mainwindow.h"
#include "servermodel.h"

ServerButton::ServerButton(ServerModel* server, MainWindow* parent) {
    //setText(QString::fromStdString(name));
    this->server = server;
    this->parent = parent;
    setIcon(server->pfp);
    setIconSize(server->pfp.rect().size());
    setCheckable(true);
    connect(this, &QAbstractButton::toggled, this, &ServerButton::handleClick);
}


void ServerButton::handleClick(bool n) {
    if (n) {
        parent->handleServerClick(this);
    } else {
        blockSignals(true);
        setChecked(true);
        blockSignals(false);
    }
    //TODO make this a signal/slot?
}
