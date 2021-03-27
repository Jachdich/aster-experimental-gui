#ifndef __SERVERBUTTON_H
#define __SERVERBUTTON_H
#include <QPushButton>
#include <QPixmap>
#include <QObject>
#include <QPaintEvent>
#include <string>

class ServerModel;
class MainWindow;
class ServerButton : public QPushButton {
Q_OBJECT
public:
    ServerModel* server;
    MainWindow* parent;
public:
    ServerButton(ServerModel* server, MainWindow* parent);

public slots:
    void handleClick(bool);
};
#endif
