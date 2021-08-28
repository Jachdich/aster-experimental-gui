#include <QApplication>

#include <vector>
#include <iostream>
#include <fstream>

#include "serverbutton.h"
#include "mainwindow.h"
#include "base64.h"

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

std::string formatStyleSheets(std::string ssheet) {
    std::string dark = "#2a2a2a";
    std::string darktransparent = "#30000000";
    std::string light = "#555555";
    std::string fg = "#cccccc";
    std::string bg = "#333333";
    return replaceAll(replaceAll(replaceAll(replaceAll(replaceAll(ssheet, "{fg}", fg), "{bg}", bg), "{light}", light), "{dark}", dark), "{darktransparent}", darktransparent);
}

int main(int argc, char *argv[]) {
    
    QApplication app(argc, argv);
    std::ifstream ifs("stylesheet.qss");
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
    exit(ret);
}
