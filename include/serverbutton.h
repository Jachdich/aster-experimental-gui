#ifndef __SERVERBUTTON_H
#define __SERVERBUTTON_H
#include <QPushButton>
#include <QPixmap>
#include <QObject>
#include <QPaintEvent>
#include <string>

class MainWindow;
class ServerButton : public QPushButton {
Q_OBJECT
public:
    std::string name;
    std::string ip;
    uint64_t uuid;
    uint16_t port;
    QPixmap pfp;
    MainWindow *parent;
public:
    ServerButton(std::string name, std::string ip, uint16_t port, uint64_t uuid, std::string pfp_b64, MainWindow* parent);
//    void paintEvent(QPaintEvent *);

public slots:
    void handleClick(bool);
};
#endif