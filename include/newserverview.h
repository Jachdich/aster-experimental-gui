#ifndef __NEWSERVERVIEW_H
#define __NEWSERVERVIEW_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QGridLayout;
class QRadioButton;
class QLabel;

class NewServerView : public QWidget {
    Q_OBJECT
    QLineEdit* ip;
    QLineEdit* port;
    QLineEdit* UUID;
    QLabel* lIp;
    QLabel* lPort;
 //   QLabel* lUUID;
    QPushButton* back;
    QPushButton* b_connect;
    QRadioButton* loginSelect;
    QRadioButton* registerSelect;
    QGridLayout* layout;
public:
    NewServerView();

public slots:
    void connectButton();
    void backButton();
    void registerPressed();
    void loginPressed();

signals:
    void backPressed();
    void connectPressed(QString, uint16_t, uint64_t);
};

#endif
