#include "messagecontainer.h"

#include <QVBoxLayout>
#include <QWidget>
#include <QScrollBar>
#include "message.h"

MessageContainer::MessageContainer() {
    layout = new QVBoxLayout();
    widget = new QWidget();

    QScrollBar* scrollbar = verticalScrollBar();
    connect(scrollbar, &QScrollBar::rangeChanged, this, &MessageContainer::sliderRangeChanged);

    layout->addStretch(1);
    layout->setSpacing(0);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWidgetResizable(true);
    widget->setLayout(layout);
    this->setWidget(widget);
}

void MessageContainer::addMessage(Message* msg) {
    messages.push_back(msg);
    layout->addWidget(msg);
}

void MessageContainer::insertMessage(uint32_t idx, Message* msg) {
    auto itPos = messages.begin() + idx;
    messages.insert(itPos, msg);
    layout->insertWidget(idx + 1, msg);
}

void MessageContainer::sliderRangeChanged(int min, int max) {
    (void)min;
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(max);
}
