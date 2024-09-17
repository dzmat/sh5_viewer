#include "tway.h"
#include "support_classes.h"
#include "read_functions.h"

void TWay::load()
{
    TStringList ls;
    int waypoint_index = 1;
    while (1) {
        int res = read_entity_list1(ls);
        if (res == MY_EOF) return;
        if (ls[0].find("Waypoint ") != std::string::npos) {
            TGameCoord t;
            coord_load(ls, t);
            double speed = read_double(ls, "Speed");
            data.push_back(TWayPoint{waypoint_index++, t, speed, -100});
        }
        else {
            unread_entity_list1(ls);
            break;// end of waypoints list
        }
    }
}

double TWay::min_distance_to(const TGameCoord &dest) const
{
    size_t size = data.size();
    double min_r = 1e99;
    for (size_t i = 0; i < size - 1; i++) {
        double r = line_min_distance(data[i].coord, data[i + 1].coord, dest);
        if (r < min_r) min_r = r;
    }
    return min_r;
}

TWay TWay::restricted_by_days(double beg, double end)
{
    size_t SZ = data.size();
    TWay res;
    res.data.resize(data.size());
    size_t dst = 0;
    for (size_t i = 0, j = 1; j < SZ; ++i, ++j) {
        if (data[j].time_of_arrival >= end) break;
        if ((data[i].time_of_arrival >= beg) && (data[j].time_of_arrival <= end)) {
            res.data[dst] = data[i], res.data[dst + 1] = data[j];
            ++dst;
            continue;
        }
        double tb = fmax(beg, data[i].time_of_arrival);
        double te = fmin(end, data[j].time_of_arrival);
        if (tb >= te) continue;
        TGameCoord cb, ce;
        tb -= data[i].time_of_arrival;
        te -= data[i].time_of_arrival;
        double tlen = data[j].time_of_arrival - data[i].time_of_arrival; // time lenght of whole interval
        TGameCoord dir = data[j].coord - data[i].coord;
        dir = dir / tlen;
        TGameCoord &origin = data[i].coord;
        cb = origin + (dir * tb);
        ce = origin + (dir * te);
        res.data[dst] = data[i], res.data[dst + 1] = data[j];
        res.data[dst].coord = cb;
        res.data[dst + 1].coord = ce;
        ++dst;
    }
    return res;
}
