#include "../include/messagecontainer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>
#include <QSize>
#include <QScrollBar>
#include "../include/message.h"
#include "../include/servermodel.h"
#include "../include/main.h"
#include <iostream>

VcContainer::VcContainer(QWidget *parent, ServerModel *server) : QWidget(parent) {
    layout  = new QHBoxLayout();
    join    = new QPushButton("join", this);
    leave   = new QPushButton("leave", this);
    join->setIcon(QIcon(QString::fromStdString(respath + pathsep + "joincall.png")));
    leave->setIcon(QIcon(QString::fromStdString(respath + pathsep + "leavecall.png")));
    connect(join,  &QPushButton::clicked, this, &VcContainer::joinVoice);
    connect(leave, &QPushButton::clicked, this, &VcContainer::leaveVoice);
    layout->addWidget(join);
    layout->addWidget(leave);
    setLayout(layout);

    this->server = server;
}

VcContainer::~VcContainer() {
    delete layout;
    delete join;
    delete leave;
    if (vc != NULL) {
	    vc->stop();
	    delete vc;
	}
}

void VcContainer::changeChannel(const std::string &newChannel) {
    channel = newChannel;
}

void setup_opus(OpusEncoder **enc, OpusDecoder **dec) {
    int err;
    if (enc != NULL) {
        *enc = opus_encoder_create(48000, 1, APPLICATION, &err);
        if (err < 0) {
            fprintf(stderr, "failed to create encoder: %s\n", opus_strerror(err));
            exit(1);
        }

        err = opus_encoder_ctl(*enc, OPUS_SET_BITRATE(BITRATE));
        if (err < 0) {
            fprintf(stderr, "failed to set bitrate: %s\n", opus_strerror(err));
            exit(1);
        }
    }

    if (dec != NULL) {
        *dec = opus_decoder_create(SAMPLE_RATE, 1, &err);
        if (err < 0) {
            fprintf(stderr, "failed to create decoder: %s\n", opus_strerror(err));
            exit(1);
        }
    }
}

void VcContainer::joinVoice() {
    server->net->sendRequest("/joinvoice " + channel);
    ctx.restart();
    vc = new VoiceClient(ctx);
    setup_opus(&vc->enc, &vc->dec);
    vc->start_recv();

    clientthread = std::thread([this]() { vc->run(server->uuid); });
    netthread    = std::thread([this]() { ctx.run(); });
    soundthread  = std::thread([this]() { vc->soundio_run(); });
}

void VcContainer::leaveVoice() {
    server->net->sendRequest("/leavevoice " + channel);
    vc->stop();
    soundthread.join();
    netthread.join();
    clientthread.join();
    vc = NULL;
    delete vc;
}

MessageContainer::MessageContainer(QWidget* parent, ServerModel *server) : QWidget(parent) {
    widget    = new QWidget(this);
    msglayout = new QVBoxLayout();
    scroll    = new QScrollArea(this);

    layout    = new QVBoxLayout();
    vc        = new VcContainer(this, server);

    QScrollBar* scrollbar = scroll->verticalScrollBar();
    connect(scrollbar, &QScrollBar::rangeChanged, this, &MessageContainer::sliderRangeChanged);

    msglayout->addStretch(1);
    msglayout->setSpacing(0);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidgetResizable(true);
    //widget->setObjectName("MessageContainer");
    //layout->setObjectName("MessageContainerLayout");
    //msglayout->setObjectName("MessageContainerMessageLayout");

    widget->setLayout(msglayout);
    scroll->setWidget(widget);

    layout->addWidget(vc);
    layout->addWidget(scroll);
    setLayout(layout);
}

MessageContainer::~MessageContainer() {
	clear();
	delete widget;
	delete layout;
	delete msglayout;
	delete vc;
}

void MessageContainer::clear() {
    for (Message* msg : messages) {
        msglayout->removeWidget(msg);
		msg->deleteLater();
	}
	messages.clear();
}

void MessageContainer::addMessage(Message* msg) {
    if (messages.size() > 0) {
        Message *lmsg = messages.back();
        if (msg->meta.uuid == lmsg->meta.uuid) {
            msg->setSmall(true);
            lmsg->setBeforeSmall(true);
        }
    }
    messages.push_back(msg);
    msglayout->addWidget(msg);
}

void MessageContainer::insertMessage(uint32_t idx, Message* msg) {
    if (messages.size() > 0 && idx > 0 && msg->meta.uuid == messages[idx - 1]->meta.uuid) {
        msg->setSmall(true);
        messages[idx - 1]->setBeforeSmall(true);
    }
    auto itPos = messages.begin() + idx;
    messages.insert(itPos, msg);
    msglayout->insertWidget(idx + 1, msg);
}

void MessageContainer::sliderRangeChanged(int min, int max) {
    (void)min;
    QScrollBar *bar = scroll->verticalScrollBar();
    bar->setValue(max);
}

void MessageContainer::setVoice(bool voice) {
    if (voice) {
        vc->show();
    } else {
        vc->hide();
    }
}