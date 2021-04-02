#include "serverbutton.h"

#include "mainwindow.h"
#include "servermodel.h"
#include "base64.h"

#include <QMenu>
#include <Qt>
#include <QPoint>
#include <QAction>
#include <QGridLayout>
#include <QLineEdit>
#include <QFileDialog>
#include <QByteArray>
#include <QBuffer>
#include <QIODevice>
#include <QBrush>
#include <QImage>
#include <QPainter>
#include <vector>

ServerButton::ServerButton(ServerModel* server, MainWindow* parent) {
    //setText(QString::fromStdString(name));
    this->server = server;
    this->parent = parent;
    setIcon(server->pfp);
    setToolTip(QString::fromStdString(server->name));
    setIconSize(server->pfp.rect().size());
    setCheckable(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QPushButton::customContextMenuRequested, this, &ServerButton::onContextMenu);

    menu = new QMenu(this);

    nick = new QAction("Change nickname", this);
    pfp  = new QAction("Change picture",  this);
    rem  = new QAction("Remove server",   this);
    del  = new QAction("Delete account",  this);
    menu->addAction(nick);
    menu->addAction(pfp);
    menu->addAction(rem);
    menu->addAction(del);

    connect(nick, &QAction::triggered, this, &ServerButton::changeNick);
    connect(pfp,  &QAction::triggered, this, &ServerButton::changePfp);
    connect(del,  &QAction::triggered, this, &ServerButton::deleteAccount);
    connect(rem,  &QAction::triggered, this, &ServerButton::removeServer);
    connect(this, &QAbstractButton::toggled, this, &ServerButton::handleClick);
}

ServerButton::~ServerButton() {
    delete menu;
    delete nick;
    delete pfp;
    delete rem;
    delete del;
    delete server;
}

void ServerButton::deleteAccount() {
	server->sendRequest("/delete " + std::to_string(server->uuid));
	removeServer();
}

void ServerButton::removeServer() {
	emit remove(this);
}

void ServerButton::onContextMenu(const QPoint &point) {
    menu->exec(mapToGlobal(point));
}

void ServerButton::handleClick(bool n) {
    if (n) {
        parent->handleServerClick(this);
    } else {
        blockSignals(true);
        setChecked(true);
        blockSignals(false);
    }
    //TODO make this a signal/slot?
}


void ServerButton::changeNick() {
    NickChange* popup = new NickChange();
    connect(popup, &NickChange::dismissed,  [=]() { popup->hide(); delete popup; });
    connect(popup, &NickChange::changeNick, [=](QString data) {
        popup->hide();
        delete popup; 
        this->server->sendRequest(("/nick " + data.toUtf8()).constData());
    });
}

void ServerButton::changePfp() {
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

    QByteArray bytes;
    QBuffer bytebuffer(&bytes);
    bytebuffer.open(QIODevice::WriteOnly);
    pm.save(&bytebuffer, "PNG");
    std::vector<uint8_t> buffer(bytes.begin(), bytes.end());
    std::string str = base64_encode((const uint8_t*)buffer.data(), (int)buffer.size());
    this->server->sendRequest("/pfp " + str);
}

NickChange::NickChange(/*QWidget* parent*/)/* : QWidget(parent)*/ {
    layout  = new QGridLayout(this);
    edit    = new QLineEdit(this);
    cancel  = new QPushButton("Cancel", this);
    confirm = new QPushButton("Set nick", this);

    layout->addWidget(edit, 0, 0, 1, 0);
    layout->addWidget(cancel, 1, 0);
    layout->addWidget(confirm, 1, 1);
    connect(cancel,  &QPushButton::clicked, this, &NickChange::cancelPressed);
    connect(confirm, &QPushButton::clicked, this, &NickChange::confirmPressed);
    show();
}

#include <QCloseEvent>
void NickChange::closeEvent(QCloseEvent *event) {
    emit dismissed();
    event->accept();
}

void NickChange::cancelPressed() {
    emit dismissed();
}

void NickChange::confirmPressed() {
    emit changeNick(edit->text());
}
