#include "newserverview.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QWidget>

NewServerView::NewServerView() {
    layout = new QGridLayout(this);
    registerSelect = new QRadioButton("Register", this);
    loginSelect    = new QRadioButton("Login with UUID", this);
    registerSelect->setChecked(true);

    ip   = new QLineEdit(this);
    port = new QLineEdit(this);
    UUID = new QLineEdit(this);

    lIp   = new QLabel("IP", this);
    lPort = new QLabel("Port", this);
    //lUUID = new QLabel("UUID")

    back      = new QPushButton("Back", this);
    b_connect = new QPushButton("Connect", this);

    port->setInputMask("99999;");
    port->setText("2345");

    layout->addWidget(lIp,            0, 0);
    layout->addWidget(lPort,          0, 1);
    layout->addWidget(ip,             1, 0);
    layout->addWidget(port,           1, 1);
    layout->addWidget(registerSelect, 2, 0);
    layout->addWidget(loginSelect,    3, 0);
    layout->addWidget(UUID,           3, 1);
    layout->addWidget(b_connect,      4, 1);
    layout->addWidget(back,           4, 0);
    setLayout(layout);

    connect(back,      &QPushButton::clicked, this, &NewServerView::backButton);
    connect(b_connect, &QPushButton::clicked, this, &NewServerView::connectButton);
    connect(registerSelect, &QRadioButton::clicked, this, &NewServerView::registerPressed);
    connect(loginSelect,    &QRadioButton::clicked, this, &NewServerView::loginPressed);
    registerPressed();
    //connect()
}

void NewServerView::connectButton() {
    uint64_t uuid = 0;
    if (loginSelect->isChecked()) {
        uuid = UUID->text().toULong();
//	std::cout << "UUID SET: " << uuid << "\n";
    }
    emit connectPressed(ip->text(), port->text().toInt(), uuid);
}
void NewServerView::backButton() {
    emit backPressed();
}

void NewServerView::loginPressed() {
    UUID->setDisabled(false);
}

void NewServerView::registerPressed() {
    UUID->setDisabled(true);
}
