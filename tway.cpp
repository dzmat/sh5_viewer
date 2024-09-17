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
