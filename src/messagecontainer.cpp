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
#include <string.h>

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

void VcContainer::joinVoice() {
    server->net->sendRequest("/joinvoice " + channel);
//    ctx.restart();
//    asio::executor_work_guard<decltype(ctx.get_executor())> work{ctx.get_executor()};
    vc = new VoiceClient(ctx);
    setup_opus(&vc->enc, NULL);
    //vc->start_recv();

    clientthread = std::thread([this]() { vc->run(server->uuid); });
//    netthread    = std::thread([this]() { printf("Running ctx\n"); ctx.run(); printf("Ctx finished running\n"); });
    netthread    = std::thread([this]() { printf("Running ctx\n"); vc->start_recv(); printf("Ctx finished running\n"); });
    soundthread  = std::thread([this]() {
        PaError err = vc->audio_run(sel_in_device, sel_out_device);
        if (err != paNoError) {
            fprintf(stderr, "An error occurred while using the portaudio stream\n");
            fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
        }
    });
    printf("joinVoice() returning\n");
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
    vc->hide();

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
