#include "errorpopup.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ErrorPopup::ErrorPopup(QString message) {
    label = new QLabel(message, this);
    ok = new QPushButton("Ok", this);
    layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(ok);
    connect(ok, &QPushButton::clicked, this, &ErrorPopup::okPressed);
}

ErrorPopup::~ErrorPopup() {
    delete label;
    delete ok;
    delete layout;
}

void ErrorPopup::okPressed() {
    emit dismissed();
}
