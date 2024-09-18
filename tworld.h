#ifndef TWORLD_H
#define TWORLD_H

#include <QTextEdit>

#include "coords.h"
#include "support_classes.h"
#include "tway.h"


class TUnit
{
public:
    TUnit()
    {
        heading = 0.0; speed = 0.0;
        is_german = is_warship = false;
        type = 0xDEADBEEF;
        is_command = false;
    }

    void load(std::string expected_command_unit_name);
    void parse(const TStringList &ls, const std::string expected_command_unit_name = "$no command unit specified$");
    void dump(QTextEdit *d) const;
    double heading, speed;
    int type;
    int next_waypoint;
    bool is_warship, is_german;
    TGameCoord coord;
    std::string s_name, s_class;
    bool is_command;
};

class TGroup
{
public:
    std::vector<TUnit> units;
    TUnit *command_unit; // not owning pointer
    TWay way_full;
    TWay way_LBD;
    bool filter_draw_group;
    bool filter_draw_way;

    TGroup(int sz)
        : units(sz)
        , command_unit(nullptr)
        , filter_draw_group(true)
        , filter_draw_way(false)
    {
    };
    ~TGroup()
    {
    }

    void load_units(const std::string command_unit_name);
    void calculate_arrival_time();
    bool has_zero_speed();
    bool has_type(int type);
    size_t size(){return units.size();}
    void calc_WLBD(double beg, double end)
    {
        assert(command_unit);
        way_LBD = way_full.extract_WLBD(beg, end);
    }
};


class TWorld
{
public:
    TWorld();
    TUnit my_boat;
    std::vector<TGroup *> groups;
    int my_units;
    double scale = 1;
    // ------------------------------------------

    void reset_groups()
    {
        for (TGroup *gp : groups) {
            delete gp;
            gp = NULL;
        }
        groups.clear();
    }

    void load_group(TStringList &ls);

    void load_my_unit(const TStringList &ls)
    {
        my_boat.parse(ls);
        my_boat.is_german = false;// to prevent dash drawing
    }

    int load_entity();
    bool load_file(const QString &fname);
};



#endif // TWORLD_H
