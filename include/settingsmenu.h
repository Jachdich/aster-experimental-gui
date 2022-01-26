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
class MainWindow;
class AccountTab;
class VoiceTab;

#include "nlohmann/json.hpp"

using json = nlohmann::json;
class SettingsMenu : public QTabWidget {
    Q_OBJECT

    AccountTab *account;
    VoiceTab *voice;

    void setDefaults();
private:
    void initialiseAudioOptions();
public:
    SettingsMenu(ClientMeta*, MainWindow*);

signals:
    void unameChanged(QString);
    void pfpChanged(QString);
    void passwdChanged(QString);
};

#endif
