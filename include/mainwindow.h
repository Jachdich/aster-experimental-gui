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
    MessageContainer* cont;
    NewServerView* nsv;
    QLineEdit* input;
    QVBoxLayout* layout;
    QHBoxLayout* serverLayout;
    QPushButton* addServerButton;
    QStackedLayout* stackedLayout;
    std::vector<ServerButton*> serverButtons;
    std::vector<ServerModel*> servers;
    size_t selectedServer;

    bool addNewServerSelected = false;
public:

    MainWindow();
    void addMessage(Message *msg);
    void insertMessage(uint32_t pos, Message *msg);
    void handleServerClick(ServerButton* button);

public slots:
    void handleButton();
    void addServer();
};
#endif
