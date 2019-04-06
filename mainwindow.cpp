// #include <winnt.h>

#include <QString>
#include <QStringList>


#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"


// ----------------------------------------------------------------------


QTextEdit *logptr;

void mylog(QString msg)
{
    logptr->append(msg);
}

void mylogs(std::string msg)
{
    QString s(msg.c_str());
    mylog(s);
}

// int read_entity_list_ptr(TStringList** list);
// int read_entity_list_ptr(TStringList** list)
// {
//    if(unread_buf!=NULL){
//        *list=unread_buf;
//        unread_buf=NULL;
//        return 1;
//    }
//    TStringList* ls=new TStringList;
//    *list=ls;
//    return read_entity_list1(ls);
// }



void MainWindow::load_world(const QString& fname)
{
    world->load_file(fname);
    mylog("U-Boat params:");
    world->my_boat.dump(ui->textEdit_Memo1);
    ui->Image1->viewpoint.pos = world->my_boat.coord;
    ui->Image1->viewpoint.m = ui->e_coef->text().toDouble();
    update_m();
}

// void __fastcall TForm1::btn_go_rightClick(TObject *Sender)
// {
//    viewpoint.x+=(50*viewpoint.m);
//    redraw();
// }
////---------------------------------------------------------------------------

// void __fastcall TForm1::gtn_go_leftClick(TObject *Sender)
// {
//    viewpoint.x-=(50*viewpoint.m);
//    redraw();
// }
////---------------------------------------------------------------------------

// void __fastcall TForm1::btn_go_downClick(TObject *Sender)
// {
//    viewpoint.y-=(50*viewpoint.m);
//    redraw();
// }
////---------------------------------------------------------------------------

// void __fastcall TForm1::btn_go_upClick(TObject *Sender)
// {
//    viewpoint.y+=(50*viewpoint.m);
//    redraw();
// }
////---------------------------------------------------------------------------



// void __fastcall TForm1::Image1MouseDown(TObject *Sender,
//      TMouseButton Button, TShiftState Shift, int X, int Y)
// {
//    if (Button==mbLeft){
//        drag_start(X,Y);
//    }
// }

void summary_add_unit(TStringList *sum, const TUnit &u)
{
    int cnt = std::stoi(sum->value(u.s_class));
    sum->setValue(u.s_class, std::to_string(cnt + 1));
}

// ---------------------------------------------------------------------------
// void __fastcall MainWindow::Image1MouseUp(TObject *Sender, TMouseButton Button,
//      TShiftState Shift, int X, int Y)
// {
//    if (Button==mbLeft){
//        drag_end(X,Y);
//        redraw();
//    }else if (Button==mbRight){//look up ship info
//        TStringList *summary=new TStringList;
//        my_coord b1=my_coord(X-5,Y+5).i2g();
//        my_coord b2=my_coord(X+5,Y-5).i2g();
//        Memo1->Lines->Clear();
//        int cnt=0;
//        for (int i=0;i<groups_count;i++){

//            TGroup* tg=groups[i];
//            for(int j=0;j<tg->size;j++){
//                const TUnit& unit=tg->units[j];
//                my_coord c=my_coord(unit.coord);
//                if( c.x>b1.x && c.x<b2.x &&
//                    c.y>b1.y && c.y<b2.y)
//                {
//                    summary_add_unit(summary,unit);
//                    unit.dump(Memo1->Lines);
//                    Memo1->Lines->Add("-------------");
//                    cnt++;
//                }
//            }
//        }
//        Memo1->Lines->AddStrings(summary);
//        Memo1->Lines->Add("-------------");
//        Memo1->Lines->Add(QString().sprintf("%d ships revealed",cnt));
//        delete summary;
//    }else{//select waypoints to draw
//        selected_way=-1;
//        my_coord b1=my_coord(X-5,Y+5).i2g();
//        my_coord b2=my_coord(X+5,Y-5).i2g();
//        Memo1->Lines->Clear();
//        for (int i=0;i<groups_count;i++){
//            TGroup* tg=groups[i];
//            for(int j=0;j<tg->size;j++){
//                my_coord c=my_coord(tg->units[j].coord);
//                if( c.x>b1.x && c.x<b2.x &&
//                    c.y>b1.y && c.y<b2.y)
//                {
//                    selected_way=i;
//                }
//            }
//        }
//        if (selected_way!=-1){
//            Memo1->Lines->Add(QString().sprintf("%d group way selected",selected_way));
//            Memo1->Lines->Add(
//                QString().sprintf(
//                    "way size = %d",groups[selected_way]->way.size
//                    )
//            );
//            redraw();
//        }else{
//            Memo1->Lines->Add(QString().sprintf("NO group way selected",selected_way));
//            redraw();
//        }
//    }
// }
// ---------------------------------------------------------------------------


// void __fastcall TForm1::Image1MouseMove(TObject *Sender, TShiftState Shift,
//      int X, int Y)
// {
//    e_tmp2->Text=QString().sprintf("IM %d %d", X,Y);
//    if (drag.started){
//        drag.mX=X;drag.mY=Y;
//    }
//    //update indication of mouse coordinates
//    my_coord mouse_game_coords=my_coord(X,Y).i2g();
//    e_coords->Text=QString().sprintf("Long %.1f  Lat %.1f", mouse_game_coords.x,mouse_game_coords.y);


//    //update distance and direction to mouse pointer from boat
//    my_coord dv=mouse_game_coords-my_boat.coord;
//    double r=dv.len()/1000;//from meters to kilometers
//    double head;
//    if (r>0.1) {
//        head=atan2(dv.x,dv.y)/M_PI*180; //because heading angle counts from Y axe toward X.
//        if (head<0) head+=360;
//    }else head = 0.0;
//    e_dist->Text=QString().sprintf("%.3f",r);
//    e_head->Text=QString().sprintf("%4f",head);
// }
////---------------------------------------------------------------------------

// void __fastcall TForm1::Timer1Timer(TObject *Sender)
// {
//    if (drag.started){
//        drag_end(drag.mX,drag.mY);
//        redraw();
//        drag_start(drag.mX,drag.mY);
//    }
// }
////---------------------------------------------------------------------------

const double WheelStep = 1.2;

// void __fastcall TForm1::FormMouseWheelUp(TObject *Sender,
//      TShiftState Shift, TPoint &MousePos, bool &Handled)
// {
//    // zoom in
//    TPoint p=Image1->ScreenToClient(MousePos);
//    QString t;
//    t.sprintf("UP %d %d | %d %d", p.x,p.y,MousePos.x, MousePos.y);
//    e_tmp->Text=t;
//    int ih=Image1->Height;
//    int iw=Image1->Width;
//    if( p.x>=0 && p.x<iw && p.y>=0&& p.y<=ih){
//        // wheel is in picture boundaries
//        my_coord vc(viewpoint.x,viewpoint.y);
//        my_coord b=my_coord(p.x,p.y).i2g();
//        my_coord d=vc-b;
//        d=d/WheelStep;
//        my_coord newvc=b+d;//main math here
//        viewpoint.m/=WheelStep;
//        viewpoint.x=newvc.x;
//        viewpoint.y=newvc.y;
//        update_m();
//        redraw();
//    }
// }
// ---------------------------------------------------------------------------

// void __fastcall TForm1::FormMouseWheelDown(TObject *Sender,
//      TShiftState Shift, TPoint &MousePos, bool &Handled)
// {
//    TPoint p=Image1->ScreenToClient(MousePos);
//    QString t;
//    t.sprintf("DW %d %d | %d %d", p.x,p.y,MousePos.x, MousePos.y);
//    e_tmp->Text=t;
//    int ih=Image1->Height;
//    int iw=Image1->Width;
//    if( p.x>=0 && p.x<iw && p.y>=0&& p.y<=ih){
//        // wheel is in picture boundaries
//        my_coord vc(viewpoint.x,viewpoint.y);
//        my_coord b=my_coord(p.x,p.y).i2g();
//        my_coord d=vc-b;
//        d=d*WheelStep;
//        my_coord newvc=b+d;//main math here
//        viewpoint.m*=WheelStep;
//        viewpoint.x=newvc.x;
//        viewpoint.y=newvc.y;
//        update_m();
//        redraw();
//    }
// }
////---------------------------------------------------------------------------



// void __fastcall TForm1::Panel2Resize(TObject *Sender)
// {
//    Image1->Width=Width-2;
//    Image1->Height=Height-2;
//    Image1->Picture->Bitmap->Height=Image1->Height;
//    Image1->Picture->Bitmap->Width=Image1->Width;
//    update_m();
//    redraw();
// }
// ---------------------------------------------------------------------------


void MainWindow::update_m()
{
    TmyQFrame* img=ui->Image1;
    img->viewpoint.o_x = img->width() / 2;
    img->viewpoint.o_y = img->height() / 2;
    double sq_size = img->viewpoint.m * img->width() / 1000.0;
    ui->e_sq_size->setText(
        QString("%1").arg(sq_size)
        );
    ui->e_coef->setText(
        QString("%1").arg(img->viewpoint.m)
        );
}

void MainWindow::redraw(){
    ui->Image1->update();
}

// ---------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //    DecimalSeparator='.';
    coords_unit_init();
    world = std::make_shared<TWorld>();
    logptr = ui->textEdit_Memo1;// very dangerous exporting, dirty and quick
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
        this
        , "Open Save Game"
        , "C:\\Users\\dinozaur\\Documents\\SH5\\data\\cfg\\SaveGames\\"
        , "Save Game (*.mis)"
        );
    if (fileName.isEmpty())return;
    ui->e_coords->setText(fileName);
    load_world(fileName);
}

void MainWindow::on_pbToBoat_clicked()
{
    ui->Image1->setScale(ui->e_coef->text().toDouble());
    ui->Image1->toBoat();
    ui->Image1->update();
}

void MainWindow::on_spinMinGroup_valueChanged(int arg1)
{
    //    void __fastcall TForm1::eMinGroupChange(TObject *Sender)
    //    {
    //        int test=eMinGroup->Text.ToIntDef(-1);
    //        if (test==-1)return;//wrong value at input? do nothing.
    //        apply_group_filter();
    //        redraw();
    //    }
}

void MainWindow::on_chZeroSpeeds_stateChanged(int arg1)
{
    //    void __fastcall TForm1::Ch_zerospeedsClick(TObject *Sender)
    //    {
    //        apply_group_filter();
    //        redraw();
    //    }
}

void MainWindow::on_eradius_of_intercept_textChanged(const QString &arg1)
{
    //    void __fastcall TForm1::eradius_of_interceptChange(TObject *Sender)
    //    {
    //        int test=eradius_of_intercept->Text.ToIntDef(-1);
    //        if (test==-1)return;//wrong value at input? do nothing.
    //        apply_way_filter();
    //        redraw();
    //    }
}

void MainWindow::on_tbZoomIn_clicked()
{
    ui->e_tmp2->setText("plus clicked");
    //    void __fastcall TForm1::btn_zoom_inClick(TObject *Sender)
    //    {
    //        viewpoint.m/=2;
    //        update_m();
    //        redraw();
    //    }
}

void MainWindow::on_tbZoomOut_clicked()
{
    ui->e_tmp2->setText("minus clicked");
    //    void __fastcall TForm1::btn_zoom_outClick(TObject *Sender)
    //    {
    //        viewpoint.m*=2;
    //        update_m();
    //        redraw();
    //    }
}

void MainWindow::on_e_flt_type_of_ship_textChanged(const QString &arg1)
{
    bool ok;
    arg1.toInt(&ok);
    if (!ok) {
        //        apply_type_filter(0);//draw all types
    }
    else {
        //            apply_type_filter(test);
    }
    //    redraw();
}
