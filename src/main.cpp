//TODO save audiso preferences-2

#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "serverbutton.h"
#include "mainwindow.h"
#include "base64.h"
#include "portaudio.h"
#include <experimental/filesystem>

#include <regex>

namespace fs = std::experimental::filesystem;

std::string prefpath;
std::string respath = "resources";
PaDeviceIndex sel_in_device  = 0;
PaDeviceIndex sel_out_device = 0;

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
std::string pathsep = "/";
#endif

#if defined(__MACH__) || defined(__APPLE__)
std::string pathsep = "/";
#endif


#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define WINDOS
#endif

#ifdef WINDOS
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
std::string pathsep = "\\";
#endif

void fatalmsg(std::string textmsg) {
    QMessageBox msg;
    msg.setText(QString::fromStdString(textmsg));
    msg.exec();
    exit(1);
}

void setup_prefpath() {
#ifdef __linux__
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    if (std::filesystem::exists("preferences.json")) {
        prefpath = ".";
    } else {
        prefpath = std::string(homedir) + "/.config/aster";
    }
    respath = "";
    for (std::string searchdir : {"./resources", "/usr/share/aster", "/usr/local/share/aster"}) {
        if (std::filesystem::is_directory(std::filesystem::path(searchdir))) {
            respath = searchdir;
            break;
        }
    }
    if (respath == "") {
        fatalmsg("Could't find the resources directory! Checked current dir, /usr/share/aster, /usr/local/share/aster");
    }
#endif

#ifdef WINDOS
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, szPath))) {
        char c_szPath[MAX_PATH];
        wcstombs(c_szPath, szPath, wcslen(szPath) + 1);
        prefpath = std::string(c_szPath) + "\\aster";
    } else {
        fatalmsg("Could not get the appdata folder location, for some reason. I really have no idea what causes this, maybe like google \"SHGetFolderPath CSIDL_APPDATA fails\"?");
    }
#endif

#if defined(__MACH__) || defined(__APPLE__)
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    prefpath = std::string(homedir) + "/Library/Preferences/aster";
#endif
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

std::string formatStyleSheets(std::string ssheet) {
    std::unordered_map<std::string, std::string> vars;
    std::istringstream iss(ssheet);
    std::string withoutDefs;
    for (std::string line; std::getline(iss, line); ) {
        if (line[0] == '@') {
            const char *ln = line.c_str();
            std::string varnm;
            std::string varval;
            while (*ln != ' ' && *ln != '=' && (unsigned)(ln - line.c_str()) < line.length()) {
                varnm += *ln++;
            }

            while (*ln == ' ' && (unsigned)(ln - line.c_str()) < line.length()) ln++;
            if (*ln != '=') {
                fatalmsg("Error in stylesheets: expected '=' in var decleration '" + varnm + "'");
            }
            //TODO technocally buffer overflow?
            ln++;
            while (*ln == ' ' && (unsigned)(ln - line.c_str()) < line.length()) ln++;

            while (*ln != ';' && (unsigned)(ln - line.c_str()) < line.length()) {
                varval += *ln++;
            }
            vars[varnm] = varval;
        } else {
            withoutDefs += line + "\n";
        }
    }
    ssheet = withoutDefs;
    for (auto &item : vars) {
        ssheet = replaceAll(ssheet, item.first, item.second);
    }
    return ssheet;
}

void setup_audio() {
    PaError err = Pa_Initialize();
    if (err) {
        char buffer[2048];
        sprintf(buffer, "Aster couldn't initialise the audio subsystem, for some reason. this should never happen. traceback:\n%s", Pa_GetErrorText(err));
        fatalmsg(std::string(buffer));
    } else {
	printf("PortAudio initialised correctly\n");
    }
}

int main(int argc, char *argv[]) {
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication app(argc, argv);

    setup_prefpath();
    setup_audio();
    if (!fs::is_directory(fs::path(prefpath))) {
        if (fs::exists(fs::path(prefpath))) {
            fatalmsg("there's a file called aster at path \"" + prefpath + "\" and I dont know what it is. Please deal with it.");
        } else {
            fs::create_directory(prefpath);
        }
    }
    
    std::ifstream ifs(respath + pathsep + "stylesheet.qss");
    std::string ss((std::istreambuf_iterator<char>(ifs)),
                   (std::istreambuf_iterator<char>()));
    
    app.setStyleSheet(QString::fromStdString(formatStyleSheets(ss)));
    MainWindow window;

    QObject::connect(&app, &QApplication::focusChanged, &app, [&window](QWidget* old, QWidget* now) {
        if (old == nullptr) {
            //Gained focus
            window.focusInEvent();
        } else if (now == nullptr) {
            //Lost focus
            window.focusOutEvent();
        }
    });

    int32_t ret = app.exec();
    window.save();
    Pa_Terminate();
    exit(ret);
}
