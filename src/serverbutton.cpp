#include "serverbutton.h"

#include "mainwindow.h"
#include "servermodel.h"

#include <QMenu>
#include <Qt>
#include <QPoint>

ServerButton::ServerButton(ServerModel* server, MainWindow* parent) {
    //setText(QString::fromStdString(name));
    this->server = server;
    this->parent = parent;
    setIcon(server->pfp);
    setToolTip(QString::fromStdString(server->name));
    setIconSize(server->pfp.rect().size());
    setCheckable(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QPushButton::customContextMenuRequested, this, &ServerButton::onContextMenu);

    menu = new QMenu(this);
    //self.popMenu.addAction(QtGui.QAction('test0', self))
    //self.popMenu.addAction(QtGui.QAction('test1', self))
    //self.popMenu.addSeparator()
    menu->addAction(new QAction("e", this));

    connect(this, &QAbstractButton::toggled, this, &ServerButton::handleClick);
}

void ServerButton::onContextMenu(const QPoint &point) {
    menu->exec(mapToGlobal(point));
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
