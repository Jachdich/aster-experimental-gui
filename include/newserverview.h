#ifndef __NEWSERVERVIEW_H
#define __NEWSERVERVIEW_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QGridLayout;

class NewServerView : public QWidget {
    Q_OBJECT
    QLineEdit* ip;
    QLineEdit* port;
    QPushButton* back;
    QPushButton* b_connect;
    QGridLayout* layout;
public:
    NewServerView();

public slots:
    void connectButton();
    void backButton();

signals:
    void backPressed();
    void connectPressed(QString, uint16_t);
};

#endif
