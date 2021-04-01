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
    QAction* nick;
    QAction* pfp;
    QAction* rem;
    QAction* del;
public:
    ServerButton(ServerModel* server, MainWindow* parent);
    ~ServerButton();

signals:
	void remove(ServerButton*);

public slots:
    void handleClick(bool);
    void onContextMenu(const QPoint&);
    void changeNick();
    void changePfp();
    void removeServer();
    void deleteAccount();
};

class NickChange : public QWidget {
Q_OBJECT
    QLineEdit* edit;
    QPushButton* confirm;
    QPushButton* cancel;
    QGridLayout* layout;
    void closeEvent(QCloseEvent *bar);
public:
    NickChange();
public slots:
    void cancelPressed();
    void confirmPressed();
signals:
    void dismissed();
    void changeNick(QString);
};

#endif

