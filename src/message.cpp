#include "message.h"
#include <QPixmap>
#include <QLabel>
#include <QGridLayout>

Message::Message(QString unamestr, QString cont, QPixmap *pfpPixmap) {
    layout = new QGridLayout();
    content = new QLabel(" " + unamestr + ": " + cont);
    
    pfp = new QLabel();
    pfp->setFixedWidth(32);
    pfp->setFixedHeight(32);
    pfp->setPixmap(pfpPixmap->scaledToWidth(32));

    content->setWordWrap(true);
    layout->setSpacing(0);
    layout->addWidget(pfp, 0, 0, 1, 1);
    layout->addWidget(content, 0, 1, 1, 1);
    setLayout(layout);
    layout->setRowStretch(1, 1);
    layout->setColumnStretch(1, 1);
}
