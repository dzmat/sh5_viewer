#ifndef TWAY_H
#define TWAY_H
#include <vector>
#include "coords.h"

#include <QTextEdit>
#include "coords.h"


class TWay
{
public:
    typedef struct TWayPoint
    {
        int waypoint_index;
        TGameCoord coord;
        double speed;
        double time_of_arrival = -100;
    } TWayPoint;

    std::vector<TWayPoint> data;

    TWay()
    {
    }

    ~TWay()
    {
    }

    void load();
    double min_distance_to(const TGameCoord &dest) const;
    TWay extract_WLBD(double beg, double end);
};


#endif // TWAY_H
