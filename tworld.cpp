#include "tworld.h"

#include <fileapi.h>
#include <wtypes.h>
#include <winbase.h>
#include <windows.h>
#include "read_functions.h"


TWorld::TWorld()
{
}

bool TGroup::has_type(int type)
{
    if (type <= 0) return true;
    for (auto &u : units)
        if (u.type == type) return true;
    return false;
}

bool TGroup::has_zero_speed()
{
    for (auto &u : units)
        if (u.speed < 0.1) return true;
    return false;
}

void TGroup::load_units(std::string command_unit_name)
{

    TStringList ls;
    for (size_t i = 0; i < size(); ) {
        read_entity_list1(ls);
        std::string head = ls[0];
        unread_entity_list1(ls);
        if (head.find("Waypoint") != std::string::npos) {
            way.load();
        }
        else {
            units[i].load(command_unit_name);
            i++;
            // try to load folloving waypoints
            read_entity_list1(ls);
            std::string head = ls[0];
            unread_entity_list1(ls);
            if (head.find("Waypoint") != std::string::npos)
                way.load();
        }
    }
    // extract next waypoint info
    for (TUnit &unit : units) {
        if (unit.is_command)
            way.next_way_point = unit.next_waypoint;
        break;
    }
}

void TUnit::load(std::string expected_command_unit_name)
{
    TStringList ls;
    int res = read_entity_list1(ls);
    if (res == MY_EOF) return;
    load(ls, expected_command_unit_name);
}

void TUnit::load(const TStringList &ls, const std::string expected_command_unit_name)
{
    //    try{
    heading = read_double(ls, "Heading");

    speed = read_double(ls, "Speed");
    s_name = ls.getValue("Name");
    s_class = ls.getValue("Class");
    type = stoi(ls.getValue("Type"));
    next_waypoint = stoi(ls.getValue("NextWP", "0"));
    if (s_name == expected_command_unit_name)
        is_command = true;
    coord_load(coord, ls);
    if ((100 <= type) && (type <= 199)) is_warship = false; else is_warship = true;
    if (ls.getValue("Origin") == "German") is_german = true; else is_german = false;
    //    }catch(EConvertError& E){
    //        QMessageBox::warning(NULL,"Exception encountered","exception while loading Unit");
    //    };
}

void TUnit::dump(QTextEdit *d) const
{
    d->append(QString("Type=%1 class=%2 name=%3").arg(type).arg(s_class.c_str(), s_name.c_str()));
    d->append(QString("Heading: %1 Speed: %2").arg(heading, 0, 'f').arg(speed, 0, 'f'));
    d->append(QString("Long: %1 Lat: %2").arg(coord.x, 0, 'f').arg(coord.y, 0, 'f'));
}

int TWorld::load_entity()
{
    TStringList ls;
    int res = read_entity_list1(ls);
    if (res == MY_EOF) return MY_EOF;
    if (check_for_group(ls)) {
        load_group(ls);
        return 1;
    }
    if (check_for_my_unit(ls)) {
        load_my_unit(ls);
        my_units++;
        return 1;
    }
    return 0;
}

bool TWorld::load_file(const QString &fname)
{
    if (!init_read(fname)) return false;
    reset_groups();
    my_units = 0;
    int res, cnt = 0;
    while (1) {
        res = load_entity();
        if (res == MY_EOF) break;
        cnt += res;;
    }
    if (cnt == 0)
        return false;
    mylogger::log(QString("loaded entities: %1").arg(cnt));
    mylogger::log(QString("loaded groups: %1").arg(groups.size()));
    mylogger::log(QString("loaded units: %1").arg(my_units));
    close_read();
    return true;
}
