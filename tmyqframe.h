#ifndef TMYQFRAME_H
#define TMYQFRAME_H
#include <memory>
#include <QFrame>
#include <QPainter>
#include "tworld.h"

typedef QColor TColor;
const QColor clBlack = QColor(0, 0, 0);
const QColor clRed = QColor(255, 0, 0);
const QColor clGreen = QColor(0, 255, 0);
const QColor clBlue = QColor(0, 0, 255);
const QColor clGray = QColor(160, 160, 160);


struct st_viewpoint
{
    TGameCoord pos;
    double m;
    int o_x, o_y;
};




class TmyQFrame : public QFrame
{
    Q_OBJECT
    std::shared_ptr<TWorld> world;
    st_drag drag;

    my_coord g2i(const TGameCoord &c) const
    {
        return my_coord(
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

    void draw_intercept_circle();

    void draw_way(QPainter *p
                  , const TWay &w
                  , Qt::PenStyle stt
                  , int width = 2
                  , QColor cl = QColor(0xFF9600))
    {
        //    if (w.size<2)return;//nothing to draw;
        //    TCanvas* canvas=Form1->Image1->Canvas;
        //    my_coord t=my_coord(w.data[0]).g2i();
        //    canvas->Pen->Color=cl;
        //    canvas->Pen->Style=stt;
        //    canvas->Pen->Width=width;
        //    canvas->MoveTo(t.x,t.y);
        //    for (int i=1;i<w.size;i++){
        //        t=my_coord(w.data[i]).g2i();
        //        canvas->LineTo(t.x,t.y);
        //        canvas->TextOut(t.x,t.y,IntToStr(i+1));
        //        canvas->MoveTo(t.x,t.y);
        //    }
    }

    void redraw()
    {
        // TODO: rewrite painting code
        //    int iw=Image1->Width;
        //    int ih=Image1->Height;
        //    my_coord b1=my_coord(0,ih).i2g();
        //    my_coord b2=my_coord(iw,0).i2g();
        //    Image1->Canvas->FillRect(Rect(0,0,iw,ih));
        //    draw_intercept_circle();
        //    for (int i=0;i<groups_count;i++){
        //        TGroup* tg=groups[i];
        //        if (tg->filter_draw_group){
        //            if (tg->filter_draw_way){
        //                draw_way(tg->way,psDot,1);
        //            }
        //            for(int j=0;j<tg->size;j++){
        //                my_coord c=my_coord(tg->units[j].coord);
        //                if( c.x>b1.x && c.x<b2.x &&
        //                    c.y>b1.y && c.y<b2.y)
        //                {
        //                    draw_unit(&(tg->units[j]));
        //                }
        //            }
        //        }
        //    }
        //    my_coord c=my_boat.coord;
        //    if( c.x>b1.x && c.x<b2.x &&
        //        c.y>b1.y && c.y<b2.y)
        //    {
        //        draw_unit(&my_boat,clGreen);
        //    }
        //    if (selected_way!=-1){
        //        draw_way(groups[selected_way]->way,psSolid);
        //    }
    }

    QColor get_color_by_type(int type);
    void draw_arrow(QPainter *c, int x, int y, double len, double heading, QColor col, Qt::PenStyle style, int width);
    void draw_unit(QPainter *p, const TUnit *u, QColor cl);

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
    }

public:
    st_viewpoint viewpoint;
    explicit TmyQFrame(QWidget *parent = nullptr);
    void setWorld( std::shared_ptr<TWorld> &w)
    {
        world = w;
        update();
    }

    void toBoat()
    {
        viewpoint.pos = world->my_boat.coord;
    }

    void setScale(double scale){viewpoint.m = scale;}
    double getScale(){return viewpoint.m;}
    void drag_start(int X, int Y);
    void drag_end(int X, int Y);

signals:

public slots:
};

#endif // TMYQFRAME_H
