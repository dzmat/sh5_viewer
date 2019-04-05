#ifndef SUPPORT_CLASSES_H
#define SUPPORT_CLASSES_H

#include <string>
#include <vector>

#include <QTextEdit>
#include "coords.h"

class TStringList : public std::vector<std::string>
{
public:
    const std::string value(const std::string inkey) const;

    void setValue(const std::string &inkey, const std::string &val);
};





#endif // SUPPORT_CLASSES_H
