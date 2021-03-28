#ifndef __METADATA_H
#define __METADATA_H

#include <string>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

class QPixmap;

struct Metadata {
    uint64_t uuid;
    std::string uname;
    std::string pfp_b64;
    QPixmap *pfp;
    static Metadata from_json(json value);
    void update(json value);
};

#endif
