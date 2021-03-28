#include "newserverview.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>

NewServerView::NewServerView() {
    layout = new QGridLayout();
    ip = new QLineEdit();
    port = new QLineEdit();
    back = new QPushButton("Back");
    connect = new QPushButton("Connect");
    port->setInputMask("99999;");
    port->setText("2345");
    layout->addWidget(ip, 0, 0);
    layout->addWidget(port, 0, 1);
    layout->addWidget(connect, 1, 0);
    layout->addWidget(back, 1, 1);
    setLayout(layout);
}
