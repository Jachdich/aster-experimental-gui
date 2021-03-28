#include "metadata.h"
#include "base64.h"

#include <QByteArray>
#include <QPixmap>
#include <string>
#include <vector>

Metadata Metadata::from_json(json value) {
    std::string pfp_b64_json = value["pfp"].get<std::string>();
    std::vector<uint8_t> buf = base64_decode(pfp_b64_json);
    QByteArray data = QByteArray((const char*)buf.data(), (int)buf.size());
    QPixmap* pixMap = new QPixmap();
    pixMap->loadFromData(data, "PNG");
    return {value["uuid"].get<uint64_t>(),
            value["name"].get<std::string>(),
            pfp_b64_json,
            pixMap
            };
}

void Metadata::update(json value) {
    uuid    = value["uuid"].get<uint64_t>();
    uname   = value["name"].get<std::string>();
    pfp_b64 = value["pfp"].get<std::string>();
    
    std::vector<uint8_t> buf = base64_decode(pfp_b64);
    QByteArray data = QByteArray((const char*)buf.data(), (int)buf.size());
    pfp->loadFromData(data, "PNG");
}
