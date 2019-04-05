#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QTextEdit>
#include <QMainWindow>
#include <QGraphicsScene>

#include "coords.h"
#include "support_classes.h"

namespace Ui
{
    class MainWindow;
}

/**
*suspicious magic nubers comes here
*/

#define MAX_GROUPS 10000
typedef uint32_t TColor;
const TColor clBlack = 0x00000000;
const TColor clRed = 0x00FF0000;
const TColor clGreen = 0x0000FF00;
const TColor clBlue = 0x000000FF;
const TColor clGray = 0x00A0A0A0;
//typedef std::vector<std::string> TStringList;

class TUnit
{
public:
    TUnit()
    {
        heading = 0.0; speed = 0.0;
        is_german = is_warship = false;
    }

    void load();
    void load(const TStringList &ls);
    void dump(QTextEdit *d) const;
    double heading,speed;
    int type;
    bool is_warship,is_german;
    TGameCoord coord;
    std::string s_name,s_class;
};

class TWay{
    public:
    TWay(){
    };
    ~TWay(){
    }
    void load();
    size_t size(){return data.size();}
    std::vector<TGameCoord> data;
    double min_distance_to(const TGameCoord& dest);
};

class TGroup{
    public:
    TGroup(int sz):
        units(sz)
    {
        filter_draw_group=true;
        filter_draw_way=false;
    };
    ~TGroup(){
    }
    void load();
    int size;
    std::vector<TUnit> units;
    TWay way;
    bool filter_draw_group;
    bool filter_draw_way;
    bool has_zero_speed();
    bool has_type(int type);
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_pbToBoat_clicked();

    void on_spinMinGroup_valueChanged(int arg1);

    void on_chZeroSpeeds_stateChanged(int arg1);

    void on_eradius_of_intercept_textChanged(const QString &arg1);


    void on_tbZoomIn_clicked();

    void on_tbZoomOut_clicked();

    void on_e_flt_type_of_ship_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    TUnit my_boat;
    QGraphicsScene Image1;
    TGroup* groups[MAX_GROUPS];
    int groups_count=0;
    int selected_way=-1;
    int my_units;


    void draw_unit(const TUnit *u,TColor cl = clBlack);
    TColor get_color_by_type(int type);
    void load_file();
    void update_m();
    void draw_intercept_circle();
    void redraw();
    void btn_to_boatClick();
    void load_my_unit(const TStringList& ls);
    void apply_group_filter();
    void apply_way_filter();
    void apply_type_filter(int type);
    int load_entity();
    void reset_groups();
    void load_group(TStringList& ls);



};

#endif // MAINWINDOW_H
