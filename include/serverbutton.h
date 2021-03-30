#ifndef __SERVERBUTTON_H
#define __SERVERBUTTON_H

#include <QPushButton>
#include <QObject>
#include <QPoint>
#include <string>

class ServerModel;
class MainWindow;
class QMenu;

class ServerButton : public QPushButton {
Q_OBJECT
public:
    ServerModel* server;
    MainWindow* parent;
    QMenu* menu;
public:
    ServerButton(ServerModel* server, MainWindow* parent);

public slots:
    void handleClick(bool);
    void onContextMenu(const QPoint&);
};
#endif
