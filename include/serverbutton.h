#ifndef __SERVERBUTTON_H
#define __SERVERBUTTON_H

#include <QPushButton>
#include <QObject>
#include <QPoint>
#include <string>

class ServerModel;
class MainWindow;
class QMenu;
class QAction;
class QLineEdit;
class QGridLayout;
class QCloseEvent;

class ServerButton : public QPushButton {
Q_OBJECT
public:
    ServerModel* server;
    MainWindow* parent;
    QMenu* menu;
    QAction* rem;
    QAction* del;
    bool active;
    void setOffline();
    void setOnline();
public:
    ServerButton(ServerModel* server, MainWindow* parent, bool active);
    ~ServerButton();

signals:
	void remove(ServerButton*);
	void serverClicked(ServerButton*);

public slots:
    void handleClick(bool);
    void onContextMenu(const QPoint&);
    void removeServer();
    void deleteAccount();
    void onlineChanged(bool online);
};

#endif

