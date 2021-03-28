#include "newserverview.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>

NewServerView::NewServerView() {
    layout = new QGridLayout();
    ip = new QLineEdit();
    port = new QLineEdit();
    back = new QPushButton("Back");
    b_connect = new QPushButton("Connect");
    port->setInputMask("99999;");
    port->setText("2345");
    layout->addWidget(ip, 0, 0);
    layout->addWidget(port, 0, 1);
    layout->addWidget(b_connect, 1, 0);
    layout->addWidget(back, 1, 1);
    setLayout(layout);

    connect(back, &QPushButton::clicked, this, &NewServerView::backButton);
    connect(b_connect, &QPushButton::clicked, this, &NewServerView::connectButton);
}

void NewServerView::connectButton() {
    emit connectPressed(ip->text(), port->text().toInt());
}
void NewServerView::backButton() {
    emit backPressed();
}
