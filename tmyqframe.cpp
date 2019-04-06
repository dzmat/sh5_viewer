#include <QPainter>
#include "tmyqframe.h"

QColor TmyQFrame::get_color_by_type(int type)
{
    if ((type >= 100) && (type <= 199)) return clBlack; // Cargo
    if ((type == 4) || (type == 2) || (type == 1)) return clRed;
    if (type == 0) return clGray; // Trawler
    if (type == 7) return QColor(127, 0, 0); ; // Kent
    if (type == 200) return QColor(0x00FFFF00);
    return QColor(255, 255, 0);
}

void TmyQFrame::draw_arrow(QPainter *c, int x, int y, double len, double heading, QColor col, Qt::PenStyle style, int width)
{
    double const h_len = 10.0;// length of arrow head parts.
    int ex, ey; double a;
    int tx, ty;
    len += 10;
    a = heading * M_PI / 180.0;
    ex = x + len * sin(a); ey = y - len * cos(a);
    QPen pen(col);
    pen.setStyle(style);
    pen.setWidth(width);
    c->setPen(pen);
    c->drawLine(x, y, ex, ey);
    // arrow head first part
    a = (heading + 160.0) * M_PI / 180.0;
    tx = ex + h_len * sin(a); ty = ey - h_len * cos(a);
    c->drawLine(ex, ey, tx, ty);
    // arrow head second part
    a = (heading + 200.0) * M_PI / 180.0;
    tx = ex + h_len * sin(a); ty = ey - h_len * cos(a);
    c->drawLine(ex, ey, tx, ty);
    pen.setColor(QColor(0xFF9600));
}

void TmyQFrame::draw_unit(QPainter *p, const TUnit *u, QColor cl)
{
    my_coord c = g2i(u->coord);

    if (cl == clBlack) cl = get_color_by_type(u->type); ;
    if (u->is_german)
        draw_arrow(p, c.x, c.y, u->speed * 2, u->heading, cl, Qt::PenStyle::DotLine, 1);
    else
        draw_arrow(p, c.x, c.y, u->speed * 2, u->heading, cl, Qt::PenStyle::SolidLine, 2);
}

TmyQFrame::TmyQFrame(QWidget *parent)
    : QFrame(parent)
{
    drag.started = false;
    viewpoint.m = 10000.0;
}

void TmyQFrame::drag_start(int X, int Y)
{
    drag.gc = i2g(my_coord(X, Y));
    drag.mX = X;
    drag.mY = Y;
    drag.started = true;
}

void TmyQFrame::drag_end(int X, int Y)
{
    TGameCoord t = i2g(my_coord(X, Y));
    viewpoint.pos += (drag.gc - t);
    drag.started = false;
}

void coords_unit_init()
{
}
