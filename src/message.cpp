#include "../include/message.h"
#include <QPixmap>
#include <QLabel>
#include <QGridLayout>
#include <iostream>
#include <QStyle>
#include <QVariant>
#include <QStyleOption>
#include <QPainter>
#include <QTextDocument>
#include <QEvent>
#include <QResizeEvent>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
void wrapLabelByTextSize(QLabel *label, int widthInPixels, QString orig_content) {
    if (widthInPixels < 100) {
        widthInPixels = 100;
    }
    QString text = orig_content;
    QString word = "";
    bool insideWord = false;
    QFontMetrics fontMetrics(label->font());
    for (int i = 0; i < text.length(); i++) {
        if (text[i] == ' ' || text[i] == '\t' || text[i] == '\n')
            insideWord = false;
        else {
            if (!insideWord) {
                insideWord = true;
                word = "";
            }
            word += text[i];
        }
        if (fontMetrics.horizontalAdvance(word) > widthInPixels) {
            text = text.left(i) + "\n" + text.right(text.length() - i);
            //label->setFixedHeight(label->height() + fontMetrics.height());
            insideWord = false;
        }
    }
    label->setText(text);
}

QPixmap loadXKCDImage(QString cont) {
    QPixmap img;
    QStringList sl = cont.split(" ");
    if (sl.size() == 2) {
        QString number = sl[1];
        bool convertOk = false;
        int xkcd = number.toUInt(&convertOk);

        if (convertOk) {
            QString url = "https://xkcd.com";
            httplib::Client cli(url.toStdString());
            auto res = cli.Get(("/" + number + "/info.0.json").toStdString().c_str());
            if (res->status == 200) {
                json msg = json::parse(res->body);
                std::string img_url = msg["img"].get<std::string>();
                httplib::Client icli("https://imgs.xkcd.com");
                auto ires = icli.Get(img_url.substr(21).c_str());
                if (ires->status != 200) {
                    std::cerr << "Got status " << ires->status << " other than 200\n";
                } else {
                    QByteArray data = QByteArray((const char*)ires->body.data(), ires->body.size());
                    img.loadFromData(data);
                }
            } else {
                std::cerr << "not 200 status: " << res->status << "\n";
            }
        } else {
            std::cerr << "not a number\n";
        }
    } else {
        std::cerr << "Not length of 2\n";
    }
    return img;
}

QPixmap loadURLImage(QString cont) {
    QPixmap img;
    QStringList sl = cont.split(" ");
    if (sl.size() == 2) {
        QString number = sl[1];
        bool convertOk = false;
        int xkcd = number.toUInt(&convertOk);

        if (convertOk) {
            QString url = "https://xkcd.com";
            httplib::Client cli(url.toStdString());
            auto res = cli.Get(("/" + number + "/info.0.json").toStdString().c_str());
            if (res->status == 200) {
                json msg = json::parse(res->body);
                std::string img_url = msg["img"].get<std::string>();
                httplib::Client icli("https://imgs.xkcd.com");
                auto ires = icli.Get(img_url.substr(21).c_str());
                if (ires->status != 200) {
                    std::cerr << "Got status " << ires->status << " other than 200\n";
                } else {
                    QByteArray data = QByteArray((const char*)ires->body.data(), ires->body.size());
                    img.loadFromData(data);
                }
            } else {
                std::cerr << "not 200 status: " << res->status << "\n";
            }
        } else {
            std::cerr << "not a number\n";
        }
    } else {
        std::cerr << "Not length of 2\n";
    }
    return img;
}

Message::Message(QWidget *parent, const Metadata &nmeta, QString cont, QPixmap *pfpPixmap, int64_t utc) : QWidget(parent), meta(nmeta) {
    layout = new QGridLayout(this);
    uname = new QLabel(" " + QString::fromStdString(meta.uname) + ": ", this);
    content = new QLabel(cont, this);
    timestamp = new QLabel(this);
    content->setWordWrap(true);
    content->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding));
    if (Qt::mightBeRichText(cont)) {
        content->setTextFormat(Qt::RichText);
    } else {
        //content->setTextFormat(Qt::MarkdownText);
    }

    content->installEventFilter(this);

    this->utc = utc;
    setTime(false);

    content_str = cont;
    content->setObjectName("content");
    uname->setObjectName("uname");
    timestamp->setObjectName("timestamp");
    content->setTextInteractionFlags(Qt::TextSelectableByMouse);
    uname->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    pfp = new QLabel(this);
    pfp->setFixedWidth(32);
    pfp->setFixedHeight(32);
    pfp->setObjectName("pfp");
    if (pfpPixmap == nullptr) {
        std::cout << "Pfp was null! Uname: " << meta.uname << "\n";
    } else {
        pfp->setPixmap(pfpPixmap->scaledToWidth(32));
    }

    layout->setSpacing(0);
    layout->addWidget(pfp,      0, 0);
    layout->addWidget(uname,    0, 1);
    layout->addWidget(content,  0, 2);
    layout->addWidget(timestamp,0, 3);
    setLayout(layout);
    layout->setColumnStretch(1, 0);
    layout->setColumnStretch(2, 1);
    layout->setRowStretch(0, 1);

    if (cont.left(4) == QString("xkcd")) {
        connect(this, &Message::imageChanged, this, &Message::changeImage);
        std::thread([cont, this]() {
            emit imageChanged(loadXKCDImage(cont));
        }).detach();
    }
}

void Message::changeImage(QPixmap pix) {
    content->setPixmap(pix);
    content->updateGeometry();
    updateGeometry();
}

void Message::updateContent(QString newcont) {
    content_str = newcont;
    content->setText(newcont);
}

const QString Message::getFullText() const {
    return content->text();
}

void Message::setSmall(bool small) {
    this->small = small;
    if (small) {
        pfp->hide();
        layout->setContentsMargins(42, 0, -1, 0);
    } else {
        pfp->show();
        layout->setContentsMargins(-1, -1, -1, -1);
    }
}

void Message::setBeforeSmall(bool beforeSmall) {
    if (beforeSmall) {
        content->setAlignment(Qt::AlignBottom);
        if (!small) {
            uname->setAlignment(Qt::AlignBottom);
        } else {
            uname->setAlignment(Qt::AlignTop);
        }
        layout->setContentsMargins(
            layout->contentsMargins().left(),
            layout->contentsMargins().top(), -1, 0);
    } else {
        content->setAlignment(Qt::AlignVCenter);
        uname->setAlignment(Qt::AlignVCenter);
    }
}

void Message::enterEvent(QEvent *) {
    setTime(true);
}

void Message::leaveEvent(QEvent *) {
    setTime(false);
}

void Message::setTime(bool full) {
    char buffer[64];
    struct tm *time;
    struct tm ct;
    struct tm mt;
    int64_t current_time = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now().time_since_epoch()).count();
    time = localtime(&current_time);
    ct = *time;
    time = localtime(&utc);
    mt = *time;

    if (full) {
        strftime(buffer, sizeof(buffer), "%H:%M:%S %d/%m/%Y", &mt);
    } else if (mt.tm_yday == ct.tm_yday && mt.tm_year == ct.tm_year) {
        strftime(buffer, sizeof(buffer), "%H:%M:%S", &mt);
    } else {
        strftime(buffer, sizeof(buffer), "%d/%m/%Y", &mt);
    }
    this->timestamp->setText(QString::fromStdString(std::string(buffer)));
}

void Message::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
/*
void TestLabel::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    style()->drawItemText(&p, rect(), Qt::AlignLeft | Qt::TextWrapAnywhere, palette(), true, text());
}
*/
/*8
bool Message::eventFilter(QObject *object, QEvent *event) {
    if (object == content && event->type() == QEvent::Resize) {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
        wrapLabelByTextSize((QLabel*)object, resizeEvent->size().width() - 10, content_str);
    }
    return false;
}*/
