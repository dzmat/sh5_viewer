#include "support_classes.h"

const std::string TStringList::value(const std::string inkey) const
{
    auto key = inkey+"+";
    for (auto &str : *this) {
        auto pos = str.find(key);
        if (pos == 0)           // found right at the beginning of the string
            return str.substr(key.length());
    }
    return std::string();
}

void TStringList::setValue(const std::string &inkey, const std::string &val)
{
    auto key = inkey+"+";
    for (auto &str : *this) {
        auto pos = str.find(key);
        if (pos == 0)           // found right at the beginning of the string
            str = key + val;
    }
}
