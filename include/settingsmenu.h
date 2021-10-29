#ifndef __SETTINGS_H
#define __SETTINGS_H

#include <QTabWidget>
#include <QString>

class QLineEdit;
class QLabel;
class QToolButton;
class QPushButton;
class QGridLayout;
class ClientMeta;
class QComboBox;
class QTabWidget;

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class SettingsMenu : public QTabWidget {
    Q_OBJECT

    QWidget *account;
    QWidget *voice;
    
    QGridLayout* acct_layout;
    QLabel* lUname;
    QLabel* lPfp;
    QLabel* lPasswd;

    QLineEdit* uname;
    QLineEdit* passwd;
    QPushButton* pfp;

    QPushButton* save;
    QPushButton* cancel;
    ClientMeta* meta;

    QGridLayout* voice_layout;
    QLabel *inlabel;
    QLabel *outlabel;
    QComboBox *inbox;
    QComboBox *outbox;
    
    QString pfp_b64;

    void setDefaults();
private:
    void initialiseAudioOptions();
public:
    SettingsMenu(ClientMeta*);

public slots:
    void saveButton();
    void backButton();
    void pfpButton();
    void inChanged(int index);
    void outChanged(int index);
    
signals:
    void unameChanged(QString);
    void pfpChanged(QString);
    void passwdChanged(QString);
};

#endif
