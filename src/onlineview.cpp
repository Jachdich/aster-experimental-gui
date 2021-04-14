#include "onlineview.h"
#include "smallprofile.h"
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>
#include <QListWidget>

/*
OnlineView::OnlineView(QWidget* parent) : QScrollArea(parent) {
    widget = new QWidget(this);
    layout = new QVBoxLayout(widget);
    
    layout->addStretch(1);
    layout->setSpacing(0);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWidgetResizable(true);
    widget->setLayout(layout);

    this->setWidget(widget);
}

OnlineView::~OnlineView() {
	clear();
}

void OnlineView::clear() {
    for (SmallProfile* profile : profiles) {
        layout->removeWidget(profile);
		profile->deleteLater();
	}
	profiles.clear();
}

void OnlineView::addProfile(SmallProfile* profile) {
    profiles.push_back(profile);
    layout->addWidget(profile);
}


void OnlineView::removeProfile(SmallProfile* profile) {
    
}

void OnlineView::insertProfile(uint32_t idx, SmallProfile* msg) {
    auto itPos = profiles.begin() + idx;
    profiles.insert(itPos, msg);
    layout->insertWidget(idx + 1, msg);
}*/

OnlineView::OnlineView(QWidget* parent) : QWidget(parent) {
    list = new QListWidget(this);
    layout = new QVBoxLayout(this);
    layout->addWidget(list);
    setLayout(layout);
}

void OnlineView::addProfile(SmallProfile* profile) {
    profiles.push_back(profile);
    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(QSize(42, 42));
    list->addItem(item);
    list->setItemWidget(item, profile);
}

void OnlineView::clear() {
    uint32_t idx = 0;
    for (SmallProfile* profile : profiles) {
        list->removeItemWidget(list->item(idx));
		delete profile;
		idx++;
	}
	list->clear();
	profiles.clear();
}
