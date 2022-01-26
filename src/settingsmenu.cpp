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

class VoiceTab : public QWidget {
public:
    QGridLayout *layout;
    
    QLabel *inlabel;
    QLabel *outlabel;
    QComboBox *inbox;
    QComboBox *outbox;
    
    MainWindow *mw;
    SettingsMenu *menu;
        
    VoiceTab(SettingsMenu *parent, MainWindow *mw) : QWidget(parent) {

        this->mw = mw;
        this->menu = menu;
        
        layout = new QGridLayout(this);
        
        inlabel = new QLabel("Input device", this);
        outlabel = new QLabel("Output device", this);
        inbox = new QComboBox(this);
        outbox = new QComboBox(this);

        layout->addWidget(inlabel, 0, 0);
        layout->addWidget(outlabel, 0, 1);
        layout->addWidget(inbox, 1, 0);
        layout->addWidget(outbox, 1, 1);
        connect(inbox,  QOverload<int>::of(&QComboBox::activated), this, &VoiceTab::inChanged);
        connect(outbox, QOverload<int>::of(&QComboBox::activated), this, &VoiceTab::outChanged);
        setLayout(layout);
        
        initialiseAudioOptions();
    }
    
public slots:
    void inChanged(int index) {
        sel_in_device = inbox->itemData(index).toInt();
        mw->save();
    }
    
    void outChanged(int index) {
        sel_out_device = outbox->itemData(index).toInt();
        mw->save();
    }

public:
    void initialiseAudioOptions() {
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
    
};

class AccountTab : public QWidget {
public:
    QGridLayout *layout;

    QLabel* lUname;
    QLabel* lPfp;
    QLabel* lPasswd;

    QLineEdit* uname;
    QLineEdit* passwd;
    QPushButton* pfp;

    QPushButton* save;
    QPushButton* cancel;
    
    ClientMeta *meta;
    MainWindow *mw;    
    
    QString pfp_b64;
    SettingsMenu *menu;
    
    AccountTab(SettingsMenu *parent, ClientMeta *meta, MainWindow *mw) : QWidget(parent) {

        pfp_b64 = meta->pfp_b64;
        this->mw = mw;
        this->meta = meta;
        this->menu = parent;

        layout = new QGridLayout(this);
        
        lUname = new QLabel("Username", this);
        lPfp = new QLabel("Profile picture", this);
        lPasswd = new QLabel("Password", this);
    
        uname = new QLineEdit(this);
        passwd = new QLineEdit(this);
        pfp = new QPushButton(this);
    
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

        setLayout(layout);

        connect(save,   &QPushButton::clicked, this, &AccountTab::saveButton);
        connect(cancel, &QPushButton::clicked, this, &AccountTab::backButton);
        connect(pfp,    &QPushButton::clicked, this, &AccountTab::pfpButton);

        setDefaults();
    }

    void setDefaults() {
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

public slots:
    void saveButton() {
        //TOOD this is a really bad idea!
        if (uname->text() == "" || passwd->text() == "") {
    		QMessageBox msg;
    		msg.setText("Username or password cannot be empty!");
    		msg.exec();
    		return;
    	}
        emit menu->unameChanged(uname->text());
        emit menu->pfpChanged(pfp_b64);
        emit menu->passwdChanged(passwd->text());
        hide();
        mw->save();
    }
    
    void backButton() {
        setDefaults();
        hide();
    }
    
    void pfpButton() {
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
};

SettingsMenu::SettingsMenu(ClientMeta *meta, MainWindow *mw) {
    account = new AccountTab(this, meta, mw);
    voice = new VoiceTab(this, mw);

    addTab(account, "Account");
    addTab(voice, "Voice");
}
