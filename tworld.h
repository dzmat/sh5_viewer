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
    }

    void load();
    void load(const TStringList &ls);
    void dump(QTextEdit *d) const;
    double heading, speed;
    int type;
    bool is_warship, is_german;
    TGameCoord coord;
    std::string s_name, s_class;
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

    void load();
    size_t size() const {return data.size();}
    std::vector<TGameCoord> data;
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

    void load();
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
        t = t.substr(i + 14, t.length() - i - 14 + 1);
        int groupsize = stoi(t);

        mylogger::log(QString("groupsize=%1").arg(groupsize));

        TGroup *gr = new TGroup(groupsize);
        gr->load();
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
    std::shared_ptr<TWorld> wpWorld;
    TFilter(std::shared_ptr<TWorld> &wp)
    {
        wpWorld = wp;
    }

    void apply_group_filter(size_t min_group, bool zeroSpeedChecked = false)
    {
        for (TGroup *tg : wpWorld->groups) {
            tg->filter_draw_group = true;
            if (tg->size() < min_group) {
                tg->filter_draw_group = false;
                continue;// do not draw small groups
            }
            if (zeroSpeedChecked) {
                if (!tg->has_zero_speed()) {
                    tg->filter_draw_group = false;
                    continue;
                }
            }
        }
    }

    void apply_way_filter(double radius)
    {
        if (radius < 0.0) radius = 0.0;
        radius *= 1000;
        auto &myCoords = wpWorld->my_boat.coord;
        for (TGroup *tg : wpWorld->groups) {
            tg->filter_draw_group = false;
            tg->filter_draw_way = false;
            if (radius == 0.0) continue;
            if (tg->units[0].is_german) continue;
            double d = tg->way.min_distance_to(myCoords);
            if (d <= radius) {
                tg->filter_draw_group = true;
                tg->filter_draw_way = true;
            }
        }
    }

    void apply_type_filter(int type)
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
