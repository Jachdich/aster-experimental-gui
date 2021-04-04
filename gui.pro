######################################################################
# Automatically generated by qmake (3.1) Wed Feb 24 00:25:39 2021
######################################################################

TEMPLATE = app
TARGET = gui
INCLUDEPATH += include
LIBS += -lssl -lcrypto
QT += widgets core
# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += include/network.h include/serverbutton.h include/message.h include/metadata.h include/newserverview.h include/mainwindow.h include/messagecontainer.h include/servermodel.h include/settingsmenu.h
SOURCES += src/base64.cpp src/main.cpp src/message.cpp src/metadata.cpp src/serverbutton.cpp src/clientnetwork.cpp src/mainwindow.cpp src/messagecontainer.cpp src/newserverview.cpp src/servermodel.cpp src/settingsmenu.cpp

QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS -= -O2
