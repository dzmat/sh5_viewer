#include "coords.h"

const TGameCoord TGameCoord::operator*(const double rhs) const
{
    return TGameCoord(x * rhs, y * rhs);
}

double TGameCoord::operator*(const TGameCoord &rhs) const
{
    return x * rhs.x + y * rhs.y;
}

const TGameCoord TGameCoord::operator/(const double rhs) const
{
    return TGameCoord(x / rhs, y / rhs);
}

const double dd = 2.0;

void test2()
{
    TGameCoord a(0, 1);
    TGameCoord b(a);
    TGameCoord c;
    c = a + b;
    b = ((a + b) / dd);
}

double line_min_distance(const TGameCoord &a, const TGameCoord &b, const TGameCoord &d)
{
    if ( (a - b).len_sqr() < 1e-6) return (d - a).len(); // too short line, dont calculate projection.
    TGameCoord v = (b - a);
    double proj = (v * (d - a)) / v.len_sqr();
    if (proj < 0) return (d - a).len();
    if (proj > 1) return (d - b).len();
    return (a + v * proj - d).len();// nearest point is somewhere in the middle of the line
}
