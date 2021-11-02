#include "../include/settingsmenu.h"
#include "../include/metadata.h"
#include "../include/main.h"
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
#include <QComboBox>
#include <QTabWidget>

#include "../include/base64.h"
#include "../include/portaudio.h"
#include "../include/mainwindow.h"
#include <string>
#include <vector>

SettingsMenu::SettingsMenu(ClientMeta *meta, MainWindow *mw) {
    this->meta = meta;
    this->mw = mw;
    account = new QWidget(this);
    voice = new QWidget(this);
    
    acct_layout = new QGridLayout(account);
    
    lUname = new QLabel("Username", account);
    lPfp = new QLabel("Profile picture", account);
    lPasswd = new QLabel("Password", account);

    uname = new QLineEdit(account);
    passwd = new QLineEdit(account);
    pfp = new QPushButton(account);

    save = new QPushButton("Save", account);
    cancel = new QPushButton("Cancel", account);

    voice_layout = new QGridLayout(account);

    inlabel = new QLabel("Input device", voice);
    outlabel = new QLabel("Output device", voice);
    inbox = new QComboBox(voice);
    outbox = new QComboBox(voice);

    initialiseAudioOptions();
    
    setDefaults();

    acct_layout->addWidget(lUname, 0, 0);
    acct_layout->addWidget(lPfp, 1, 0);
    acct_layout->addWidget(lPasswd, 2, 0);
    acct_layout->addWidget(uname, 0, 1);
    acct_layout->addWidget(passwd, 2, 1);
    acct_layout->addWidget(pfp, 1, 1);
    acct_layout->addWidget(save, 3, 1);
    acct_layout->addWidget(cancel, 3, 0);

    voice_layout->addWidget(inlabel, 0, 0);
    voice_layout->addWidget(outlabel, 0, 1);
    voice_layout->addWidget(inbox, 1, 0);
    voice_layout->addWidget(outbox, 1, 1);

    account->setLayout(acct_layout);
    voice->setLayout(voice_layout);
    
    pfp_b64 = meta->pfp_b64;

    connect(save,   &QPushButton::clicked, this, &SettingsMenu::saveButton);
    connect(cancel, &QPushButton::clicked, this, &SettingsMenu::backButton);
    connect(pfp,    &QPushButton::clicked, this, &SettingsMenu::pfpButton);
    connect(inbox,  QOverload<int>::of(&QComboBox::activated), this, &SettingsMenu::inChanged);
    connect(outbox, QOverload<int>::of(&QComboBox::activated), this, &SettingsMenu::outChanged);

    addTab(account, "Account");
    addTab(voice, "Voice");
}

void SettingsMenu::inChanged(int index) {
    sel_in_device = inbox->itemData(index).toInt();
    mw->save();
}

void SettingsMenu::outChanged(int index) {
    sel_out_device = outbox->itemData(index).toInt();
    mw->save();
}

void SettingsMenu::initialiseAudioOptions() {
    PaDeviceIndex indefault = Pa_GetDefaultInputDevice();
    PaDeviceIndex outdefault = Pa_GetDefaultOutputDevice();
    
    PaDeviceIndex index = Pa_GetDeviceCount();

    //do the currently set sel_in_device and out_sel_device make sense?
    //check if they are contained in the device list
    bool in_contains = false;
    bool out_contains = false;
    
    if (index < 0) {
        printf("Some error: %s\n", Pa_GetErrorText(index));
        //uhh error handling idk
    }
    
    for (PaDeviceIndex i = 0; i < index; i++) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info->maxOutputChannels > 0) {
            out_contains = out_contains || (i == sel_out_device);
            printf("%d: %-48s %d %d\n", i, info->name, info->maxInputChannels, info->maxOutputChannels);
            outbox->addItem(QString(info->name), i);
        }
    }

    printf("\n");
    for (PaDeviceIndex i = 0; i < index; i++) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info->maxInputChannels > 0) {
            in_contains = in_contains || (i == sel_in_device);
            printf("%d: %-48s %d %d\n", i, info->name, info->maxInputChannels, info->maxOutputChannels);
            inbox->addItem(QString(info->name), i);
        }
    }

    if (!in_contains) sel_in_device = indefault;
    if (!out_contains) sel_out_device = outdefault;

    outbox->setCurrentIndex(outbox->findData(sel_out_device));
    inbox->setCurrentIndex(inbox->findData(sel_in_device));
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
    mw->save();
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
