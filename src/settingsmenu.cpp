#include "settingsmenu.h"
#include "metadata.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QByteArray>
#include <QFileDialog>
#include <QBuffer>
#include <QIODevice>
#include <QBrush>
#include <QImage>
#include <QPainter>
#include <QIcon>
#include <QMessageBox>

#include "base64.h"
#include <string>
#include <vector>

SettingsMenu::SettingsMenu(ClientMeta* meta) {
    this->meta = meta;
    layout = new QGridLayout(this);
    
    lUname = new QLabel("Username", this);
    lPfp = new QLabel("Profile picture", this);
    lPasswd = new QLabel("Password", this);

    uname = new QLineEdit(this);
    passwd = new QLineEdit(this);
    pfp = new QPushButton(this);

    save = new QPushButton("Save", this);
    cancel = new QPushButton("Cancel", this);

    setDefaults();

    layout->addWidget(lUname, 0, 0);
    layout->addWidget(lPfp, 1, 0);
    layout->addWidget(lPasswd, 2, 0);
    layout->addWidget(uname, 0, 1);
    layout->addWidget(passwd, 2, 1);
    layout->addWidget(pfp, 1, 1);
    layout->addWidget(save, 3, 1);
    layout->addWidget(cancel, 3, 0);

    pfp_b64 = meta->pfp_b64;

    connect(save,   &QPushButton::clicked, this, &SettingsMenu::saveButton);
    connect(cancel, &QPushButton::clicked, this, &SettingsMenu::backButton);
    connect(pfp,    &QPushButton::clicked, this, &SettingsMenu::pfpButton);
}

void SettingsMenu::setDefaults() {
    pfp->setStyleSheet("QPushButton { border: none; }");
    std::vector<uint8_t> buf = base64_decode(meta->pfp_b64.toUtf8().constData());
    QByteArray data = QByteArray((const char*)buf.data(), (int)buf.size());
    QPixmap icon;
    icon.loadFromData(data, "PNG");
    icon = icon.scaled(32, 32);
    pfp->setIcon(icon);
    pfp->setIconSize(icon.rect().size());
    uname->setText(meta->uname);
    passwd->setText(meta->passwd);
    pfp_b64 = meta->pfp_b64;
}

void SettingsMenu::saveButton() {
    //TOOD this is a really bad idea!
    if (uname->text() == "" || passwd->text() == "") {
		QMessageBox msg;
		msg.setText("Username or password cannot be empty!");
		msg.exec();
		return;
	}
    emit unameChanged(uname->text());
    emit pfpChanged(pfp_b64);
    emit passwdChanged(passwd->text());
    hide();
}

void SettingsMenu::backButton() {
    setDefaults();
    hide();
}

void SettingsMenu::pfpButton() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "", tr("Images (*.png *.jpg *.jpeg)"));

    if (fileName == "") {
        return;
    }
    
    QPixmap image;
    image.load(fileName);
    QPixmap scaled = image.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QImage out_img(32, 32, QImage::Format_ARGB32);
    out_img.fill(Qt::transparent);
  
    QBrush brush(scaled);
    QPainter painter(&out_img);
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 32, 32);
    painter.end();
    QPixmap pm = QPixmap::fromImage(out_img);
    
    pfp->setIcon(pm);
    pfp->setIconSize(pm.rect().size());
    
    QByteArray bytes;
    QBuffer bytebuffer(&bytes);
    bytebuffer.open(QIODevice::WriteOnly);
    pm.save(&bytebuffer, "PNG");
    std::vector<uint8_t> buffer(bytes.begin(), bytes.end());
    std::string str = base64_encode((const uint8_t*)buffer.data(), (int)buffer.size());
    pfp_b64 = QString::fromStdString(str);
}
