#ifndef __ONLINEVIEW_H
#define __ONLINEVIEW_H
#include <QWidget>
#include <QString>
#include <vector>
#include <QScrollArea>

class QVBoxLayout;
class QWidget;
class SmallProfile;
class QListWidget;
/*
class OnlineView : public QScrollArea {
    QVBoxLayout *layout;
    QWidget *widget;
    std::vector<SmallProfile*> profiles;
public:
    OnlineView(QWidget* parent);
    ~OnlineView();

    void addProfile(SmallProfile* profile);
    void removeProfile(SmallProfile* profile);
    void clear();
    void insertProfile(uint32_t idx, SmallProfile* profile);

public slots:
};*/

class OnlineView : public QWidget {
    QListWidget* list;
    QVBoxLayout *layout;
    std::vector<SmallProfile*> profiles;
public:
    OnlineView(QWidget* parent);
    void addProfile(SmallProfile* profile);
    void clear();
};

#endif
