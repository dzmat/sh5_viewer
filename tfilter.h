#ifndef TFILTER_H
#define TFILTER_H
#include "tworld.h"

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
        have_to_recalc_WLBD = true;
        apply_filter();
    }

    void set_draw_zero_speed_only(bool x)   {draw_zero_speed_only = x;  apply_filter();}
    void set_min_group_size(size_t x)       {min_group_size = x;        apply_filter();}
    void set_type(int x)                    {type = x;                  apply_filter();}
    void set_radius(double x)               {radius = x;                apply_filter();}
    void set_days_before(double x)          {days_before = x; have_to_recalc_WLBD = true; apply_filter();}
    void set_days_after(double x)           {days_after = x;  have_to_recalc_WLBD = true; apply_filter();}

private:
    bool have_to_recalc_WLBD;
    void apply_filter()
    {
        if (!pWorld) return;
        if (radius < 0.0) radius = 0.0;
        const double radius_in_metres = radius * 1000;
        const auto &myCoords = pWorld->my_boat.coord;
        if (have_to_recalc_WLBD) {
            for (TGroup *tg : pWorld->groups) tg->calc_WLBD(days_before, days_after);
            have_to_recalc_WLBD = false;
        }

        if (radius_in_metres == 0.0) {
            for (TGroup *tg : pWorld->groups) {
                tg->filter_draw_group = tg->size() >= min_group_size
                                        && (!draw_zero_speed_only || tg->has_zero_speed());
                tg->filter_draw_way = false;
            }
        }
        else {
            for (TGroup *tg : pWorld->groups) {
                tg->filter_draw_group = !tg->units[0].is_german && tg->way_LBD.min_distance_to(myCoords) <= radius_in_metres;
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

#endif // TFILTER_H
