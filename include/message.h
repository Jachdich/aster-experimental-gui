#ifndef __MESSAGE_H
#define __MESSAGE_H

#include "metadata.h"
#include <QWidget>
#include <QString>
#include <QLabel>
class QGridLayout;
class QPixmap;
/*
class TestLabel : public QLabel {
protected:
    void paintEvent(QPaintEvent*) override;
public:
    TestLabel(const QString &s, QWidget *parent) : QLabel(s, parent) {};
};*/

class Message : public QWidget {
Q_OBJECT
//Q_PROPERTY(bool hover)
    QLabel *content;
    QLabel *pfp;
    QLabel *uname;
    QLabel *timestamp;
    QString content_str;
    bool small = false;
    int64_t utc;
    void setTime(bool full);
protected:
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void paintEvent(QPaintEvent*) override;

signals:
    void imageChanged(QPixmap pix);
protected slots:
    void changeImage(QPixmap pix);
//    bool eventFilter(QObject *object, QEvent *event) override;
public:
    QGridLayout *layout;
    const Metadata &meta;
    Message(QWidget *parent, const Metadata &meta, QString cont, QPixmap *pfpPixmap, int64_t timestamp);
    void updateContent(QString newcont);
    const QString getFullText() const;
    void setSmall(bool small);
    void setBeforeSmall(bool beforeSmall);
};

#endif
