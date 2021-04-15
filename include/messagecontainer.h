#ifndef __MESSAGECONTAINER_H
#define __MESSAGECONTAINER_H

#include <vector>
#include <QScrollArea>

class QVBoxLayout;
class QWidget;
class Message;

class MessageContainer : public QScrollArea {
    QVBoxLayout *layout;
    QWidget *widget;
    std::vector<Message*> messages;
public:
    MessageContainer(QWidget* parent);
    ~MessageContainer();

    void addMessage(Message* msg);
    void clear();
    void insertMessage(uint32_t idx, Message* msg);

public slots:
    void sliderRangeChanged(int min, int max);
};

#endif
