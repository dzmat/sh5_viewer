#ifndef TWORLD_H
#define TWORLD_H

#include <memory>
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

    void load_group(TStringList &ls);

    void load_my_unit(const TStringList &ls)
    {
        my_boat.parse(ls);
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
    std::shared_ptr<TWorld> pWorld;
    bool draw_zero_speed_only;
    double days_before;
    double days_after;

    TFilter()
        : min_group_size(0)
        , radius(50)
        , type(0)
        , pWorld(nullptr)
        , draw_zero_speed_only(false)
        , days_before(0.0)
        , days_after(2.0)
    {}
    void installNewWorld(std::shared_ptr<TWorld> &wp)
    {
        pWorld = wp;
        apply_filter();
    }

    void set_draw_zero_speed_only(bool x)   {draw_zero_speed_only = x;  apply_filter();}
    void set_min_group_size(size_t x)       {min_group_size = x;        apply_filter();}
    void set_type(int x)                    {type = x;                  apply_filter();}
    void set_radius(double x)               {radius = x;                apply_filter();}
    void set_days_before(double x)          {days_before = x;           apply_filter();}
    void set_days_after(double x)           {days_after = x;            apply_filter();}

private:
    void apply_filter()
    {
        if (radius < 0.0) radius = 0.0;
        const double radius_in_metres = radius * 1000;
        const auto &myCoords = pWorld->my_boat.coord;

        if (radius_in_metres == 0.0) {
            for (TGroup *tg : pWorld->groups) {
                tg->filter_draw_group = tg->size() >= min_group_size
                                        && (!draw_zero_speed_only || tg->has_zero_speed());
                tg->filter_draw_way = false;
            }
        }
        else {
            for (TGroup *tg : pWorld->groups) {
                tg->filter_draw_group = !tg->units[0].is_german && tg->way.min_distance_to(myCoords) <= radius_in_metres;
                tg->filter_draw_way = tg->filter_draw_group;
            }
        }
        // apply_way_filter();//restore ways for all types
        if (type != 0) {
            for (TGroup *tg : pWorld->groups) {
                if (tg->has_type(type)) {
                    tg->filter_draw_group = true;
                }
                else {
                    tg->filter_draw_group = false;
                    tg->filter_draw_way = false; // don't draw useless ways
                }
            }
        }
    }
};

#endif // TWORLD_H
