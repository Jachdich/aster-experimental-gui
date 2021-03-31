#include "newserverview.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>

NewServerView::NewServerView() {
    layout = new QGridLayout();
    registerSelect = new QRadioButton("Register", this);
    loginSelect    = new QRadioButton("Login with UUID", this);
    registerSelect->setChecked(true);
    
    ip   = new QLineEdit();
    port = new QLineEdit();
    UUID = new QLineEdit();

    lIp   = new QLabel("IP");
    lPort = new QLabel("Port");
    //lUUID = new QLabel("UUID")
    
    back      = new QPushButton("Back");
    b_connect = new QPushButton("Connect");
    
    port->setInputMask("99999;");
    port->setText("2345");
    
    layout->addWidget(lIp,            0, 0);
    layout->addWidget(lPort,          0, 1);
    layout->addWidget(ip,             1, 0);
    layout->addWidget(port,           1, 1);
    layout->addWidget(registerSelect, 2, 0);
    layout->addWidget(loginSelect,    3, 0);
    layout->addWidget(UUID,           3, 1);
    layout->addWidget(b_connect,      4, 0);
    layout->addWidget(back,           4, 1);
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
