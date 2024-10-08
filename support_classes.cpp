#include "support_classes.h"

const std::string TStringList::getValue(const std::string inkey, const std::string &default_value) const
{
    auto key = inkey + "=";
    for (auto &str : *this) {
        auto pos = str.find(key);
        if (pos == 0)           // found right at the beginning of the string
            return str.substr(key.length());
    }
    return default_value;
}

void TStringList::setValue(const std::string &inkey, const std::string &val)
{
    auto key = inkey + "=";
    for (auto &str : *this) {
        auto pos = str.find(key);
        if (pos == 0) {          // found right at the beginning of the string
            str = key + val;
            return;
        }
    }
    // string by key is not found, add new one
    this->push_back(key + val);
}

QPointF calc_line_end_by_heading(const QPointF &beg, const double len, const double heading)
{
    double a = (180 - heading) * M_PI / 180.0;
    return QPointF(sin(a), cos(a)) * len + beg;
}
