#ifndef __CHANNELWIDGET_H
#define __CHANNELWIDGET_H
#include <QListWidgetItem>

class ChannelWidget : public QListWidgetItem {
Q_PROPERTY(bool unread READ isUnread WRITE setUnread)
public:
	ChannelWidget(QString a, QListWidget* b) : QListWidgetItem(a, b) {}
	
};

#endif
