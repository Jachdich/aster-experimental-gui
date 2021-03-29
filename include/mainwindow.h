#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H
#include <vector>
#include <string>
#include <QWidget>

class MessageContainer;
class NewServerView;
class QLineEdit;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QStackedLayout;
class ServerButton;
class ServerModel;
class Message;

class MainWindow : public QWidget {
    NewServerView* nsv;
    QLineEdit* input;
    QVBoxLayout* layout;
    QHBoxLayout* serverButtonLayout;
    QPushButton* addServerButton;
    QStackedLayout* serverContentLayout;
    std::vector<ServerButton*> serverButtons;
    std::vector<ServerModel*> servers;
    size_t selectedServer;

public:
    MainWindow();
    void addMessage(Message *msg);
    void insertMessage(uint32_t pos, Message *msg);
    void handleServerClick(ServerButton* button);
    void save();

public slots:
    void handleButton();
    void openNewServerView();
    void closeNewServerView();
    void addNewServer(QString ip, uint16_t port);
    void onServerInitialised(ServerModel* server);
};
#endif
