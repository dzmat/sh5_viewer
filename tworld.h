#ifndef TWORLD_H
#define TWORLD_H

#include <memory>
#include <QTextEdit>

#include "coords.h"
#include "support_classes.h"
#include "mylog.h"

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
    void load(const TStringList &ls, const std::string expected_command_unit_name = "$no command unit specified$");
    void dump(QTextEdit *d) const;
    double heading, speed;
    int type;
    int next_waypoint;
    bool is_warship, is_german;
    TGameCoord coord;
    std::string s_name, s_class;
    bool is_command;
};

class TWay
{
public:
    TWay()
    {
    }

    ~TWay()
    {
    }

    typedef struct
    {
        int waypoint_index;
        TGameCoord coord;
    } TWayPoint;
    int next_way_point = 0;
    void load();
    size_t size() const {return data.size();}
    std::vector<TWayPoint> data;
    double min_distance_to(const TGameCoord &dest) const;
};

class TGroup
{
public:
    TGroup(int sz)
        : units(sz)
    {
        filter_draw_group = true;
        filter_draw_way = false;
    };
    ~TGroup()
    {
    }

    void load_units(const std::string command_unit_name);
    std::vector<TUnit> units;
    TWay way;
    bool filter_draw_group;
    bool filter_draw_way;
    bool has_zero_speed();
    bool has_type(int type);
    size_t size(){return units.size();}
};

#define MAX_GROUPS 10000

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

    void load_group(TStringList &ls)
    {
        std::string t = ls.getValue("CommandUnitName");

        mylogger::logs(t);

        auto i = t.find(";Group Size = ");
        if (i == std::string::npos) return;
        std::string command_unit_name = t.substr(0, i);
        mylogger::logs(std::string("extracted command unit name: ") + command_unit_name);
        t = t.substr(i + 14, t.length() - i - 14 + 1);
        int groupsize = stoi(t);

        mylogger::log(QString("groupsize=%1").arg(groupsize));

        TGroup *gr = new TGroup(groupsize);
        gr->load_units(command_unit_name);
        groups.push_back(gr);
    }

    void load_my_unit(const TStringList &ls)
    {
        my_boat.load(ls);
        my_boat.is_german = false;// to prevent dash drawing
    }

    int load_entity();
    bool load_file(const QString &fname);
};

class TFilter
{
public:
    size_t min_group_size;
    double radius;
    int type;
    std::shared_ptr<TWorld> wpWorld;
    bool draw_zero_speed_only;

    TFilter(std::shared_ptr<TWorld> &wp)
        : min_group_size(0)
        , radius(50)
        , type(0)
        , wpWorld(wp)
        , draw_zero_speed_only(false)
    {}

    void set_draw_zero_speed_only(bool x)   {draw_zero_speed_only = x;  apply_group_filter();}
    void set_min_group_size(size_t x)       {min_group_size = x;        apply_group_filter();}
    void set_type(int x)                    {type = x;                  apply_type_filter();}
    void set_radius(double x)               {radius = x;                apply_way_filter();}

private:
    void apply_group_filter()
    {
        for (TGroup *tg : wpWorld->groups) {
            tg->filter_draw_group = true;
            if (tg->size() < min_group_size) {
                tg->filter_draw_group = false;
                continue;// do not draw small groups
            }
            if (draw_zero_speed_only) {
                if (!tg->has_zero_speed()) {
                    tg->filter_draw_group = false;
                    continue;
                }
            }
        }
    }

    void apply_way_filter()
    {
        if (radius < 0.0) radius = 0.0;
        double radius_in_metres = radius * 1000;
        auto &myCoords = wpWorld->my_boat.coord;
        for (TGroup *tg : wpWorld->groups) {
            tg->filter_draw_group = false;
            tg->filter_draw_way = false;
            if (radius_in_metres == 0.0) continue;
            if (tg->units[0].is_german) continue;
            double d = tg->way.min_distance_to(myCoords);
            if (d <= radius_in_metres) {
                tg->filter_draw_group = true;
                tg->filter_draw_way = true;
            }
        }
    }

    void apply_type_filter()
    {
        // apply_way_filter();//restore ways for all types
        for (TGroup *tg : wpWorld->groups) {
            if (tg->has_type(type)) {
                tg->filter_draw_group = true;
            }
            else {
                tg->filter_draw_group = false;
                tg->filter_draw_way = false; // don't draw useless ways
            }
        }
    }
};

#endif // TWORLD_H
