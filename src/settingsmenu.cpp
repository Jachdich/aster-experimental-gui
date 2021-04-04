#include "settingsmenu.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QByteArray>

#include "base64.h"
#include <string>
#include <vector>

SettingsMenu::SettingsMenu(json& currentSettings) {
    layout = new QGridLayout(this);
    
    lUname = new QLabel("Username", this);
    lPfp = new QLabel("Profile picture", this);
    lPasswd = new QLabel("Password", this);

    uname = new QLineEdit(this);
    passwd = new QLineEdit(this);
    pfp = new QPushButton(this);

    pfp->setStyleSheet("QPushButton { border: none; }");
    std::vector<uint8_t> buf = base64_decode(currentSettings["pfp"].get<std::string>());
    QByteArray data = QByteArray((const char*)buf.data(), (int)buf.size());
    QPixmap icon;
    icon.loadFromData(data, "PNG");
    icon = icon.scaled(32, 32);
    pfp->setIcon(icon);
    pfp->setIconSize(icon.rect().size());

    uname->setText(QString::fromStdString(currentSettings["uname"].get<std::string>()));
    passwd->setText(QString::fromStdString(currentSettings["passwd"].get<std::string>()));

    save = new QPushButton("Save", this);
    cancel = new QPushButton("Cancel", this);

    layout->addWidget(lUname, 0, 0);
    layout->addWidget(lPfp, 1, 0);
    layout->addWidget(lPasswd, 2, 0);
    layout->addWidget(uname, 0, 1);
    layout->addWidget(passwd, 2, 1);
    layout->addWidget(pfp, 1, 1);
    layout->addWidget(save, 3, 1);
    layout->addWidget(cancel, 3, 0);

    pfp_b64 = QString::fromStdString(currentSettings["pfp"].get<std::string>());
}

void SettingsMenu::saveButton() {
    //TOOD this is a really bad idea!
    emit unameChanged(uname->text());
    emit pfpChanged(pfp_b64);
    emit passwdChanged(passwd->text());
}

void SettingsMenu::backButton() {
    hide();
}

void SettingsMenu::pfpButton() {
    
}
