#ifndef TWAY_H
#define TWAY_H
#include <vector>
#include "coords.h"

#include <QTextEdit>
#include "coords.h"


class TWay
{
public:
    TWay()
    {
    }

    ~TWay()
    {
    }

    typedef struct TWayPoint
    {
        int waypoint_index;
        TGameCoord coord;
        double time_of_arrival = -100;
    } TWayPoint;
    int next_way_point = 0;
    void load();
    //    size_t size() const {return data.size();}
    size_t size() const {return data.size();}
    std::vector<TWayPoint> data;
    double min_distance_to(const TGameCoord &dest) const;
};


#endif // TWAY_H
