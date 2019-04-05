#include "coords.h"



st_viewpoint viewpoint;
st_drag drag;

my_coord TGameCoord::g2i() const
{
    return my_coord(
        (x - viewpoint.pos.x) / viewpoint.m + viewpoint.o_x,
        (viewpoint.pos.y - y) / viewpoint.m + viewpoint.o_y
        );
}
TGameCoord my_coord::i2g() const
{
    return TGameCoord(
        (x - viewpoint.o_x) * viewpoint.m + viewpoint.pos.x,
        (viewpoint.o_y - y) * viewpoint.m + viewpoint.pos.y
        );
}

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

void drag_start(int X, int Y)
{
    drag.gc = my_coord(X, Y).i2g();
    drag.mX = X;
    drag.mY = Y;
    drag.started = true;
}

void drag_end(int X, int Y)
{
    TGameCoord t = my_coord(X, Y).i2g();
    viewpoint.pos += (drag.gc - t);
    drag.started = false;
}

void coords_unit_init()
{
    drag.started = false;
    viewpoint.m = 10000.0;
}
