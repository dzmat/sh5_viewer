#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>

#include "tmyqframe.h"
#include "mylog.h"

TmyQFrame::TmyQFrame(QWidget *parent)
    : QFrame(parent)
{
    drag.started = false;
    viewpoint.m = 10000.0;
    selected_way = -1;
    setMouseTracking(true);
    timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, this, &TmyQFrame::onTimer);
    timer->start(50);
}

void TmyQFrame::draw_intercept_circle(QPainter *p)
{
    // draws intercept green dashed circle
    // prepare brush
    QPen pen;
    pen.setColor(clGreen);
    pen.setWidth(1);
    pen.setStyle(Qt::PenStyle::SolidLine);
    QBrush brush;
    brush.setColor(clGreen);
    brush.setStyle(Qt::BrushStyle::BDiagPattern);
    p->setPen(pen);
    p->setBrush(brush);
    // draw
    double eR = interceptRadius * 1000 / (viewpoint.m);
    QPointF center = g2i_QPoint(world->my_boat.coord);
    p->drawEllipse(center, eR, eR);
}

void TmyQFrame::draw_way(QPainter *p, const TWay &w, Qt::PenStyle stt, int width, QColor cl)
{
    if (w.size() < 2) return; // nothing to draw;
    QPen pen;
    pen.setStyle(stt);
    pen.setWidth(width);
    QPointF t1 = g2i_QPoint(w.data[0].coord);
    for (size_t i = 1; i < w.size(); i++) {
        QPointF t2 = g2i_QPoint(w.data[i].coord);
        pen.setColor(cl);
        p->setPen(pen);
        p->drawLine(t1, t2);
        t1 = t2;
        pen.setColor(clBlack);
        p->setPen(pen);
        p->drawText(t2, QString::number(w.data[i].waypoint_index) + "\n" + QString::number(w.data[i].time_of_arrival, 'f', 1));
    }
}

QColor TmyQFrame::get_color_by_type(int type)
{
    if ((type >= 100) && (type <= 199)) return clBlack; // Cargo
    if ((type == 4) || (type == 2) || (type == 1)) return clRed;
    if (type == 0) return clGray; // Trawler
    if (type == 7) return QColor(127, 0, 0); ; // Kent
    if (type == 200) return QColor(0x0000FFFF);
    return QColor(170, 0, 255);
}

QPointF calc_line_end_by_heading(const QPointF &beg, const double len, const double heading)
{
    double a = heading * M_PI / 180.0;
    return QPointF(sin(a), cos(a)) * len + beg;
}

void TmyQFrame::draw_arrow(QPainter *c, const QPointF &begin, double len, double heading, QColor col, Qt::PenStyle style, int width)
{
    double const h_head_part_len = 10.0;    // length of arrow head parts.
    QPointF end = calc_line_end_by_heading(begin, len, heading);
    QPointF tip1 = calc_line_end_by_heading(end, h_head_part_len, heading + 160);
    QPointF tip2 = calc_line_end_by_heading(end, h_head_part_len, heading + 200);
    QPen pen(col);
    pen.setStyle(style);
    pen.setWidth(width);
    c->setPen(pen);
    // draw body
    c->drawLine(begin, end);
    // arrow head first part
    c->drawLine(end, tip1);
    // arrow head second part
    c->drawLine(end, tip2);
}

void TmyQFrame::draw_unit(QPainter *p, const TUnit *unit, QColor color = clBlack)
{
    QPointF c = g2i_QPoint(unit->coord);

    if (color == clBlack) color = get_color_by_type(unit->type);
    double arrow_len = unit->speed * 2 + 10;
    if (unit->is_german)
        draw_arrow(p, c, arrow_len, unit->heading, color, Qt::PenStyle::DashLine, 1);
    else
        draw_arrow(p, c, arrow_len, unit->heading, color, Qt::PenStyle::SolidLine, 2);
}

void TmyQFrame::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    // TODO: rewrite painting code
    int iw = width();
    int ih = height();
    TGameCoord b1 = QPointF_i2g(QPointF(0, ih));
    TGameCoord b2 = QPointF_i2g(QPointF(iw, 0));
    // Image1->Canvas->FillRect(Rect(0,0,iw,ih));
    draw_intercept_circle(&p);
    for (TGroup *group : world->groups) {
        if (group->filter_draw_group) {
            if (group->filter_draw_way)
                draw_way(&p, group->way, Qt::PenStyle::DashLine, 1, cl_Way);
            for (TUnit &unit : group->units) {
                TGameCoord c = unit.coord;
                if (( c.x > b1.x) && ( c.x < b2.x) &&
                    ( c.y > b1.y) && ( c.y < b2.y) )
                    draw_unit(&p, &unit);
            }
        }
    }
    TGameCoord c = world->my_boat.coord;
    if (( c.x > b1.x) && ( c.x < b2.x) &&
        ( c.y > b1.y) && ( c.y < b2.y) )
        draw_unit(&p, &(world->my_boat), clGreen);
    if (selected_way != -1)
        draw_way(&p, world->groups[selected_way]->way, Qt::PenStyle::SolidLine, 2, cl_Way);
}

void TmyQFrame::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton)
        // drag_start(event->x(), event->y());
        drag_start(event->position());
}

void TmyQFrame::mouseMoveEvent(QMouseEvent *event)
{

    const QPointF pos = event->position();

    if (drag.started)
        drag.mouse_position = pos;
    // update indication of mouse coordinates
    TGameCoord mouse_game_coords = QPointF_i2g(pos);

    //        e_coords->Text=QString().sprintf("Long %.1f  Lat %.1f", mouse_game_coords.x,mouse_game_coords.y);

    // update distance and direction to mouse pointer from boat
    TGameCoord dv = mouse_game_coords - world->my_boat.coord;
    double r = dv.len() / 1000;// from meters to kilometers
    double head;
    if (r > 0.001) {
        head = atan2(dv.x, dv.y) / M_PI * 180; // because heading angle counts from Y axe toward X.
        if (head < 0) head += 360;
    }
    else {
        head = 0.0;
    }
    emit changed_polar_coords(r, head);
}

void summary_add_unit(TStringList *sum, const TUnit &u)
{
    std::string s = sum->getValue(u.s_class);
    int cnt = 0;
    if (s.length() != 0)
        cnt = std::stoi(s);
    sum->setValue(u.s_class, std::to_string(cnt + 1));
}

void TmyQFrame::mouseReleaseEvent(QMouseEvent *event)
{
    const QPointF pos = event->position();
    if (event->button() == Qt::MouseButton::LeftButton) {
        drag_end(pos);
        update();
    }
    else if (event->button() == Qt::MouseButton::RightButton) {    // look up ship info

        TStringList *summary = new TStringList;
        TGameCoord b1 = QPointF_i2g(pos + QPointF(-5, 5));
        TGameCoord b2 = QPointF_i2g(pos + QPointF(5, -5));
        mylogger::logptr->clear();
        int cnt = 0;
        for (auto &tg : world->groups) {
            int group_leader_speed = -1;
            bool group_print_speed_flag = false;
            for (const TUnit &unit : tg->units) {
                TGameCoord c = unit.coord;
                if (( c.x > b1.x) && ( c.x < b2.x) &&
                    ( c.y > b1.y) && ( c.y < b2.y) ) {
                    summary_add_unit(summary, unit);
                    unit.dump(mylogger::logptr);
                    mylogger::log("-------------");
                    cnt++;
                    group_print_speed_flag = true;
                }
                if (unit.is_command) group_leader_speed = unit.speed;
            }
            if (group_print_speed_flag)
                mylogger::log(QString("Group speed = %1").arg(group_leader_speed));
        }
        for (auto &s : (*summary))
            mylogger::logs(s);

        mylogger::log("-------------");
        mylogger::log(QString("%1 ships revealed").arg(cnt));
        delete summary;
    }
    else {    // select waypoints to draw
        selected_way = -1;
        TGameCoord b1 = QPointF_i2g(pos + QPointF(-5, 5));
        TGameCoord b2 = QPointF_i2g(pos + QPointF(5, -5));

        mylogger::logptr->clear();
        int grpCount = world->groups.size();
        for (int i = 0; i < grpCount; ++i) {
            TGroup *tg = world->groups[i];
            for (const TUnit &unit : tg->units) {
                TGameCoord c = unit.coord;
                if (( c.x > b1.x) && ( c.x < b2.x) &&
                    ( c.y > b1.y) && ( c.y < b2.y) )
                    selected_way = i;
            }
        }
        if (selected_way != -1) {
            mylogger::log(QString("%1 group way selected").arg(selected_way));
            mylogger::log(QString("way size = %1").arg(world->groups[selected_way]->way.size()));
            update();
        }
        else {
            mylogger::log(QString("NO group way selected"));
            update();
        }
    }
}

void TmyQFrame::resizeEvent(QResizeEvent *)
{
    viewpoint.o_x = width() / 2;
    viewpoint.o_y = height() / 2;
    emit size_changed();
}

void TmyQFrame::drag_start(const QPointF &position)
{
    drag.gc = QPointF_i2g(position);
    drag.mouse_position = position;
    drag.started = true;
}

void TmyQFrame::drag_end(const QPointF &pos)
{
    TGameCoord t = QPointF_i2g(pos);
    viewpoint.pos += (drag.gc - t);
    drag.started = false;
}

void TmyQFrame::wheelEvent(QWheelEvent *event)
{
    const double WheelStep = 1.2;
    QPoint numDegrees = event->angleDelta() / 8;
    int numSteps = numDegrees.y() / 15;
    QPoint p = event->position().toPoint();
    // TGameCoord vc=viewpoint.pos;
    TGameCoord b = QPointF_i2g(QPointF(p.x(), p.y()));
    TGameCoord d = viewpoint.pos - b;
    while (numSteps > 0) {
        viewpoint.m /= WheelStep;
        d = d / WheelStep;
        numSteps--;
    }
    while (numSteps < 0) {
        viewpoint.m *= WheelStep;
        d = d * WheelStep;
        numSteps++;
    }
    TGameCoord newvc = b + d;// main math here
    viewpoint.pos = newvc;
    event->accept();
    update();
    emit scale_changed();
}

void TmyQFrame::onTimer()
{
    if (drag.started) {
        drag_end(drag.mouse_position);
        repaint();
        drag_start(drag.mouse_position);
    }
}
