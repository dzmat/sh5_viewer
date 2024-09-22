#ifndef TMYGLWIDGET_H
#define TMYGLWIDGET_H

#include "QOpenGLWidget"

#include <memory>
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
const QColor cl_Way = QColor(0, 150, 255);
const QColor cl_background = QColor(255, 255, 255);


struct st_viewpoint
{
    TGameCoord pos;
    double m;
    int o_x, o_y;
};
struct st_drag
{
    TGameCoord gc;// game coordinates
    QPointF mouse_position;// mouse coordinates
    bool started;
};

class TmyGLWidget : public QOpenGLWidget
{
    Q_OBJECT
    std::shared_ptr<TWorld> world;
    st_drag drag;
    int selected_way = -1;
    QTimer *timer;

    QPointF g2i_QPoint(const TGameCoord &c) const
    {
        return QPointF(
            (c.x - viewpoint.pos.x) / viewpoint.m + viewpoint.o_x,
            (viewpoint.pos.y - c.y) / viewpoint.m + viewpoint.o_y
            );
    }

    TGameCoord QPointF_i2g(const QPointF &c) const
    {
        return TGameCoord(
            (c.x() - viewpoint.o_x) * viewpoint.m + viewpoint.pos.x,
            (viewpoint.o_y - c.y()) * viewpoint.m + viewpoint.pos.y
            );
    }

    void draw_intercept_circle(QPainter *p);
    void draw_way(QPainter *p, const TWay &w, Qt::PenStyle stt, int width, QColor cl);
    QColor get_color_by_type(int type);
    void draw_arrow(QPainter *c, const QPointF &begin, double len, double heading, QColor col, Qt::PenStyle style, int width);
    void draw_unit(QPainter *p, const TUnit *u, QColor cl);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

public:
    double interceptRadius;
    st_viewpoint viewpoint;
    explicit TmyGLWidget(QWidget *parent = nullptr);
    void setWorld( std::shared_ptr<TWorld> &w) {world = w; update();}

    void toBoat() {viewpoint.pos = world->my_boat.coord;}

    void worldChanged(){selected_way = -1;}
    void setScale(double scale){viewpoint.m = scale;}
    void drag_start(const QPointF &position);
    void drag_end(const QPointF &pos);

signals:
    void size_changed();
    void scale_changed();
    void changed_polar_coords(double r, double head);

public slots:
    void onTimer();
    void onResized();
};


#endif // TMYGLWIDGET_H
