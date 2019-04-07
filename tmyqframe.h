#ifndef TMYQFRAME_H
#define TMYQFRAME_H
#include <memory>
#include <QFrame>
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QTimer>

#include "tworld.h"

typedef QColor TColor;
const QColor clBlack = QColor(0, 0, 0);
const QColor clRed = QColor(255, 0, 0);
const QColor clGreen = QColor(0, 255, 0);
const QColor clBlue = QColor(0, 0, 255);
const QColor clGray = QColor(160, 160, 160);
const QColor cl_Way= QColor(0, 150, 255);

struct st_viewpoint
{
    TGameCoord pos;
    double m;
    int o_x, o_y;
};
struct st_drag
{
    TGameCoord gc;// game coordinates
    int mX, mY;// mouse coordinates
    bool started;
};

class TmyQFrame : public QFrame
{
    Q_OBJECT
    std::shared_ptr<TWorld> world;
    st_drag drag;
    int selected_way = -1;
    QTimer *timer;

    my_coord g2i(const TGameCoord &c) const
    {
        return my_coord(
            (c.x - viewpoint.pos.x) / viewpoint.m + viewpoint.o_x,
            (viewpoint.pos.y - c.y) / viewpoint.m + viewpoint.o_y
            );
    }

    QPoint g2i_QPoint(const TGameCoord &c) const
    {
        return QPoint(
            (c.x - viewpoint.pos.x) / viewpoint.m + viewpoint.o_x,
            (viewpoint.pos.y - c.y) / viewpoint.m + viewpoint.o_y
            );
    }

    TGameCoord i2g(const my_coord &c) const
    {
        return TGameCoord(
            (c.x - viewpoint.o_x) * viewpoint.m + viewpoint.pos.x,
            (viewpoint.o_y - c.y) * viewpoint.m + viewpoint.pos.y
            );
    }

    void draw_intercept_circle(QPainter *p);
    void draw_way(QPainter *p, const TWay &w, Qt::PenStyle stt, int width, QColor cl);
    QColor get_color_by_type(int type);
    void draw_arrow(QPainter *c, int x, int y, double len, double heading, QColor col, Qt::PenStyle style, int width);
    void draw_unit(QPainter *p, const TUnit *u, QColor cl);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public:
    double interceptRadius;
    st_viewpoint viewpoint;
    explicit TmyQFrame(QWidget *parent = nullptr);
    void setWorld( std::shared_ptr<TWorld> &w) {world = w;update();}

    void toBoat() {viewpoint.pos = world->my_boat.coord;}

    void worldChanged(){selected_way = -1;}
    void setScale(double scale){viewpoint.m = scale;}
    double getScale(){return viewpoint.m;}
    void drag_start(int X, int Y);
    void drag_end(int X, int Y);

signals:
    void size_changed();
    void scale_changed();
    void changed_polar_coords(double r,double head);

public slots:
    void onTimer();
};

#endif // TMYQFRAME_H
