#ifndef __SMALLPROFILE_H
#define __SMALLPROFILE_H
#include <QWidget>
#include <QString>

class QLabel;
class QPixmap;
class QHBoxLayout;

class SmallProfile : public QWidget {
    QLabel* pic;
    QLabel* uname;
    QHBoxLayout* layout;
public:
    SmallProfile(QString name, QPixmap* pfp);
};

#endif
