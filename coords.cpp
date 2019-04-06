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

const my_coord my_coord::operator*(const double rhs) const
{
    return my_coord(x * rhs, y * rhs);
}

double my_coord::operator*(const my_coord &rhs) const
{
    return x * rhs.x + y * rhs.y;
}

const my_coord my_coord::operator/(const double rhs) const
{
    return my_coord(x / rhs, y / rhs);
}

void test()
{
    my_coord a(0, 1);
    my_coord b(a);
    my_coord c;
    c = a + b;
    b = ((a + b) / dd);
}
