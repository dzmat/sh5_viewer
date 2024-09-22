#ifndef SUPPORT_CLASSES_H
#define SUPPORT_CLASSES_H

#include <string>
#include <vector>
#include <qpoint.h>


class TStringList : public std::vector<std::string>
{
public:
    const std::string getValue(const std::string inkey, const std::string &default_value = std::string()) const;

    void setValue(const std::string &inkey, const std::string &val);
};

QPointF calc_line_end_by_heading(const QPointF &beg, const double len, const double heading);



#endif // SUPPORT_CLASSES_H
