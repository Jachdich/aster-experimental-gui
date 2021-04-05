#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H
#include <vector>
#include <string>
#include <QWidget>
#include <QString>

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
class SettingsMenu;

class MainWindow : public QWidget {
    NewServerView* nsv;
    SettingsMenu* settings;
    QLineEdit* input;
    QVBoxLayout* layout;
    QHBoxLayout* serverButtonLayout;
    QPushButton* addServerButton;
    QPushButton* settingsButton;
    QStackedLayout* serverContentLayout;
    std::vector<ServerButton*> serverButtons;
    std::vector<ServerModel*> servers;
    size_t selectedServer = 0;
    bool safeToSave = true;

    void updateMeta();

public:
    QString uname;
    QString passwd;
    QString pfp_b64;
    MainWindow();
    void addMessage(Message *msg);
    void insertMessage(uint32_t pos, Message *msg);
    void handleServerClick(ServerButton* button);
    void save();

public slots:
	void deleteServerButton(ServerButton*);
    void handleButton();
    void openNewServerView();
    void closeNewServerView();
    void openSettings();
    void closeSettings();
    void addNewServer(QString ip, uint16_t port, uint64_t uuid);
    void onServerInitialised(ServerModel* server, bool active);
};
#endif
