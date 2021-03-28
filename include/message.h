#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <QWidget>
#include <QString>

class QLabel;
class QGridLayout;
class QPixmap;

class Message : public QWidget {
    QLabel *content;
    QLabel *pfp;
    QGridLayout *layout;
public:
    Message(QString unamestr, QString cont, QPixmap *pfpPixmap);
};

#endif
