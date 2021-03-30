#ifndef __ERRORPOPUP_H
#define __ERRORPOPUP_H
#include <QWidget>
#include <QString>

class QLabel;
class QPushButton;
class QVBoxLayout;

class ErrorPopup : public QWidget {
Q_OBJECT
    QLabel* label;
    QPushButton* ok;
    QVBoxLayout* layout;
public:
    ErrorPopup(QString message);
    ~ErrorPopup();

public slots:
    void okPressed();

signals:
    void dismissed();
};

#endif
