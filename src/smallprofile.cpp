#include "smallprofile.h"
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>

SmallProfile::SmallProfile(QString name, QPixmap* pfp) {
    layout = new QHBoxLayout(this);
    pic = new QLabel(this);
    pic->setFixedWidth(32);
    pic->setFixedHeight(32);
    if (pfp == nullptr) {
        //std::cout << "Pfp was null! Uname: " << unamestr.toUtf8().constData() << "\n";
    } else {
        pic->setPixmap(pfp->scaledToWidth(32));
    }

    uname = new QLabel(name, this);

    layout->addWidget(pic);
    layout->addWidget(uname);
    setLayout(layout);
}
