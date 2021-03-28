#ifndef __NEWSERVERVIEW_H
#define __NEWSERVERVIEW_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QGridLayout;

class NewServerView : public QWidget {
    QLineEdit* ip;
    QLineEdit* port;
    QPushButton* back;
    QPushButton* connect;
    QGridLayout* layout;
public:
    NewServerView();
};

#endif
