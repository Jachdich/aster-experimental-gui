#ifndef __SETTINGS_H
#define __SETTINGS_H

#include <QWidget>
#include <QString>

class QLineEdit;
class QLabel;
class QToolButton;
class QPushButton;
class QGridLayout;
class ClientMeta;

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class SettingsMenu : public QWidget {
    Q_OBJECT
    
    QGridLayout* layout;
    QLabel* lUname;
    QLabel* lPfp;
    QLabel* lPasswd;

    QLineEdit* uname;
    QLineEdit* passwd;
    QPushButton* pfp;

    QPushButton* save;
    QPushButton* cancel;
    ClientMeta* meta;
    
    QString pfp_b64;

    void setDefaults();
    
public:
    SettingsMenu(ClientMeta*);

public slots:
    void saveButton();
    void backButton();
    void pfpButton();
    
signals:
    void unameChanged(QString);
    void pfpChanged(QString);
    void passwdChanged(QString);
};

#endif
