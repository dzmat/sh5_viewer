//#include <winnt.h>
#include <fileapi.h>
#include <wtypes.h>
#include <winbase.h>
#include <windows.h>


#include <QString>
#include <QStringList>


#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"






typedef std::list<std::string> MStringList;


typedef void TObject;
class TForm1{
    void __fastcall btn_to_boatClick(TObject *Sender);
};
TForm1 *Form1;
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
typedef void* HANDLE;
HANDLE file_handle;

//----------------------------------------------------------------------


#define MY_EOF -1
const int MY_BUF_SZ=1024;
char buf[MY_BUF_SZ];
unsigned long  buf_count,buf_pos;
QTextEdit* logptr;

void mylog(QString msg)
{
    logptr->append(msg);
}
void mylogs(std::string msg)
{
    QString s(msg.c_str());
    mylog(s);
}

inline int read_char(char& ch)
{
    if (buf_pos>=buf_count || buf_count==0){
        //pump from disk
        bool res=ReadFile(file_handle,buf,MY_BUF_SZ,&buf_count,0);
        if (!res || (res &&  buf_count == 0)){
            if(!res){
                DWORD err=GetLastError();
                wchar_t msg[2048];
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,err,0,msg,2048,NULL);
                mylog(QString::fromWCharArray(msg,2048));
            }
            ch=0;
            return 0;//EOF
        }
        buf_pos=0;
    }
    ch=buf[buf_pos];
    buf_pos++;
    return 1;
}

int my_read_line(std::string& str)
{
    char buf[MY_BUF_SZ+4]; // +4 for preventing accidental write
    //beyond array border while skipping '\n'
    int i=0;
    bool eof_flag=false;
    while (i<MY_BUF_SZ){
        if (!read_char(buf[i]) ){
            eof_flag=true;
            break;
        }
        if (buf[i]=='\r'){
            read_char(buf[i+1]);//skip '\n'
            break;
        }
        i++;
    }
    if (i==0 && eof_flag){//haven't read anything, already at eof
        return MY_EOF;
    }
    str=std::string(buf,i);
    return str.length();
}
TStringList unread_buf;


void init_read()
{
    buf_pos=buf_count=0;//init for read_char
    unread_buf.clear();
}

void unread_entity_list1(TStringList& list)
{
    unread_buf=list;
}

int read_entity_list1(TStringList& list)
{
    if (unread_buf.size()){
        list=unread_buf;
        unread_buf.clear();
        return 1;
    }
    std::string s;
    bool flag=false;
    list.clear();
    //seek section begin
    while (my_read_line(s)!=MY_EOF){
        if (s.c_str()[0]=='['){//found section beginning
            flag=true;
            list.push_back(s);
            break;
        }
    }
    if (!flag)return MY_EOF;
    //read body
    int cnt=0;
    flag=false;
    while (my_read_line(s)!=MY_EOF){
        if (s.length()==0){//end of body - empty string
            break;
        }
        list.push_back(s);
        cnt++;
    }
    return cnt;
}

//int read_entity_list_ptr(TStringList** list);
//int read_entity_list_ptr(TStringList** list)
//{
//    if(unread_buf!=NULL){
//        *list=unread_buf;
//        unread_buf=NULL;
//        return 1;
//    }
//    TStringList* ls=new TStringList;
//    *list=ls;
//    return read_entity_list1(ls);
//}





bool check_header(std::string& instr,const std::string& type)
{
    int ilen=instr.length();
    int tlen=type.length();
    if (ilen<tlen+2)return false;
    if (instr[1]!='[' || instr[ilen]!=']')return false;
    std::string t=instr.substr(2,ilen-2);
    if (t.substr(1,tlen)!=type)return false;
    t=t.substr(tlen+1,t.length()-tlen);
    ilen=t.length();
    for(int i=1;i<=ilen;i++)
        if (!isdigit(t[i]))
            return false;
    return true;
}

bool check_for_group(const TStringList& ls)
{
    std::string s=ls[0];
    return check_header(s,"Group ");
}
bool check_for_my_unit(const TStringList& ls)
{
    std::string s=ls[0];
    if (!check_header(s,"Unit "))return false;
    std::string t=ls.value("Commander");
    if (t!=std::string("1"))return false;
    return true;
}
double read_double(const TStringList& ls, const std::string& key)
{
    std::string t=ls.value(key);
    if (t.length()==0)return 0.0;
    return stod(t);
}

void coord_load(TGameCoord& c,const TStringList& ls)
{
    c.x=read_double(ls,"Long");
    c.y=read_double(ls,"Lat");
}






void TWay::load()
{
    TStringList ls;
    while(1){
        int res=read_entity_list1(ls);
        if (res==MY_EOF){return;}
        if (ls[0].find("Waypoint ")!=std::string::npos){
            TGameCoord t;
            coord_load(t,ls);
            data.push_back(t);
        }else{
            unread_entity_list1(ls);
            break;//end of waypoints list
        }
    }
}

void TGroup::load(){

    TStringList ls;
        for (int i=0;i<size;){
            read_entity_list1(ls);
            std::string head=ls[0];
            unread_entity_list1(ls);
            if(head.find("Waypoint")!=std::string::npos){
                way.load();
            }else{
                units[i].load();
                i++;
                //try to load folloving waypoints
                read_entity_list1(ls);
                std::string head=ls[0];
                unread_entity_list1(ls);
                if(head.find("Waypoint")!=std::string::npos){
                    way.load();
                }
            }
        }
}

bool TGroup::has_zero_speed()
{
    for(int i=0;i<size;i++){
        if (units[i].speed<0.1)return true;
    }
    return false;
}


void MainWindow::reset_groups()
{
    for (int i=0;i<groups_count;i++){
        delete groups[i];
        groups[i]=NULL;
    }
    groups_count=0;
    selected_way=-1;
}


void MainWindow::load_group(TStringList& ls)
{
    std::string t=ls.value("CommandUnitName");

    mylogs(t);

    auto i=t.find(";Group Size = ");
    if (i==std::string::npos) return;
    t=t.substr(i+14,t.length()-i-14+1);
    int groupsize=stoi(t);

    QString s("groupsize=%1");
    mylog(s.arg(groupsize));

    TGroup* gr=new TGroup(groupsize);
    gr->load();
    if(groups_count<MAX_GROUPS){
        groups[groups_count]=gr;
        groups_count++;
        mylog(QString("group way size=%1").arg(gr->way.size()));
    }else delete gr; //drop group
}

void MainWindow::load_my_unit(const TStringList& ls)
{
    my_boat.load(ls);
    my_boat.is_german=false;//to prevent dash drawing
}

int MainWindow::load_entity()
{
    TStringList ls;
        int res=read_entity_list1(ls);
        if (res==MY_EOF)return MY_EOF;
        if (check_for_group(ls)){
            load_group(ls);
            return 1;
        }
        if (check_for_my_unit(ls)){
            load_my_unit(ls);
            my_units++;
            return 1;
        }
    return 0;
}



void MainWindow::load_file()
{
    buf_pos=buf_count=0;//init for read_char
    init_read();
    reset_groups();
    my_units=0;
    int res,cnt=0;
    while (1){
        res=load_entity();
        if(res==MY_EOF)break;
        cnt+=res;;
    }
    mylog(QString("loaded entities: %1").arg(cnt));
    mylog(QString("loaded groups: %1").arg(groups_count));
    mylog(QString("loaded units: %1").arg(my_units));
    mylog("U-Boat params:");
    my_boat.dump(ui->textEdit_Memo1);
    viewpoint.pos=my_boat.coord;
    viewpoint.m=ui->e_coef->text().toDouble();
    update_m();
}

void draw_arrow(QPainter* c,int x,int y,double len,double heading,QColor col,Qt::PenStyle style,int width)
{
    double const h_len=10.0;//length of arrow head parts.
    int ex,ey;double a;
    int tx,ty;
    len+=10;
    a=heading*M_PI/180.0;
    ex=x+len*sin(a);ey=y-len*cos(a);
    QPen pen(col);
    pen.setStyle(style);
    pen.setWidth(width);
    c->setPen(pen);
    c->drawLine(x,y,ex,ey);
    //arrow head first part
    a=(heading+160.0)*M_PI/180.0;
    tx=ex+h_len*sin(a);ty=ey-h_len*cos(a);
    c->drawLine(ex,ey,tx,ty);
    //arrow head second part
    a=(heading+200.0)*M_PI/180.0;
    tx=ex+h_len*sin(a);ty=ey-h_len*cos(a);
    c->drawLine(ex,ey,tx,ty);
}



TColor MainWindow::get_color_by_type(int type)
{
    if (type>=100 && type<=199)return clBlack; //Cargo
    if (type==4 || type==2 || type==1)return clRed;
    if (type==0) return clGray;  //Trawler
    if (type==7) return clRed/2; //Kent
    if (type==200)return TColor(0x00FFFF00);
    return clRed+clGreen;
}

void MainWindow::draw_unit(const TUnit* u,TColor cl)
{
//    my_coord c=my_coord(u->coord).g2i();
//    if (cl==clBlack) cl=get_color_by_type(u->type);;
//    if (u->is_german)
//        draw_arrow(Form1->Image1->Canvas,c.x,c.y,u->speed*2,u->heading,cl,psDot,1);
//    else
//        draw_arrow(Form1->Image1->Canvas,c.x,c.y,u->speed*2,u->heading,cl,psSolid,2);

}

void draw_way(const TWay& w,Qt::PenStyle stt,int width=2,TColor cl=TColor(0xFF9600))
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

//---------------------------------------------------------------------------

void MainWindow::draw_intercept_circle()
//draws intercept green dashed circle
{
    bool ok;
    int test=ui->eradius_of_intercept->text().toInt(&ok);
    if (!ok)return;//wrong value at input? do nothing.
    TGameCoord dr=TGameCoord(test*1000,test*1000);
    my_coord a=(my_boat.coord-dr).g2i();
    my_coord b=(my_boat.coord+dr).g2i();
    //TODO: rewrite painting code
//    //prepare brush
//    TCanvas* canvas=Form1->Image1->Canvas;
//    canvas->Pen->Color=clGreen;
//    canvas->Pen->Width=1;
//    canvas->Pen->Style=psSolid;
//    TBrushRecall* ptbr=new TBrushRecall(canvas->Brush);
//    canvas->Brush->Color=clGreen;
//    canvas->Brush->Style=bsBDiagonal;
//    //draw
//    canvas->Ellipse(a.x,a.y,b.x,b.y);
//    delete ptbr;
}

void MainWindow::redraw()
{
    //TODO: rewrite painting code
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

void MainWindow::btn_to_boatClick()
{
    viewpoint.m=ui->e_coef->text().toDouble();
    viewpoint.pos=my_boat.coord;
    redraw();
}

//void __fastcall TForm1::btn_go_rightClick(TObject *Sender)
//{
//    viewpoint.x+=(50*viewpoint.m);
//    redraw();
//}
////---------------------------------------------------------------------------

//void __fastcall TForm1::gtn_go_leftClick(TObject *Sender)
//{
//    viewpoint.x-=(50*viewpoint.m);
//    redraw();
//}
////---------------------------------------------------------------------------

//void __fastcall TForm1::btn_go_downClick(TObject *Sender)
//{
//    viewpoint.y-=(50*viewpoint.m);
//    redraw();
//}
////---------------------------------------------------------------------------

//void __fastcall TForm1::btn_go_upClick(TObject *Sender)
//{
//    viewpoint.y+=(50*viewpoint.m);
//    redraw();
//}
////---------------------------------------------------------------------------



//void __fastcall TForm1::Image1MouseDown(TObject *Sender,
//      TMouseButton Button, TShiftState Shift, int X, int Y)
//{
//    if (Button==mbLeft){
//        drag_start(X,Y);
//    }
//}

void summary_add_unit(TStringList* sum,const TUnit& u)
{
    int cnt=std::stoi(sum->value(u.s_class));
    sum->setValue(u.s_class,std::to_string(cnt+1));
}

//---------------------------------------------------------------------------
//void __fastcall MainWindow::Image1MouseUp(TObject *Sender, TMouseButton Button,
//      TShiftState Shift, int X, int Y)
//{
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
//}
//---------------------------------------------------------------------------


//void __fastcall TForm1::Image1MouseMove(TObject *Sender, TShiftState Shift,
//      int X, int Y)
//{
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
//}
////---------------------------------------------------------------------------

//void __fastcall TForm1::Timer1Timer(TObject *Sender)
//{
//    if (drag.started){
//        drag_end(drag.mX,drag.mY);
//        redraw();
//        drag_start(drag.mX,drag.mY);
//    }
//}
////---------------------------------------------------------------------------

const double WheelStep=1.2;

//void __fastcall TForm1::FormMouseWheelUp(TObject *Sender,
//      TShiftState Shift, TPoint &MousePos, bool &Handled)
//{
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
//}
//---------------------------------------------------------------------------

//void __fastcall TForm1::FormMouseWheelDown(TObject *Sender,
//      TShiftState Shift, TPoint &MousePos, bool &Handled)
//{
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
//}
////---------------------------------------------------------------------------



//void __fastcall TForm1::Panel2Resize(TObject *Sender)
//{
//    Image1->Width=Width-2;
//    Image1->Height=Height-2;
//    Image1->Picture->Bitmap->Height=Image1->Height;
//    Image1->Picture->Bitmap->Width=Image1->Width;
//    update_m();
//    redraw();
//}
//---------------------------------------------------------------------------


void MainWindow::update_m()
{
    viewpoint.o_x = Image1.width() / 2;
    viewpoint.o_y = Image1.height() / 2;
    ui->e_sq_size->setText(
        QString().sprintf(
            "%f",
            double(viewpoint.m * (Image1.width()))
            / 1000)
        );

    ui->e_coef->setText(
        QString().sprintf("%f", viewpoint.m)
        );
}

void MainWindow::apply_group_filter()
{
    int min_group=ui->spinMinGroup->value();
    for (int i=0;i<groups_count;i++){
        TGroup* tg=groups[i];
        tg->filter_draw_group=true;
        if (tg->size<min_group){
            tg->filter_draw_group=false;
            continue;//do not draw small groups
        }
        if (ui->chZeroSpeeds->isChecked()){
            if(!tg->has_zero_speed()){
                tg->filter_draw_group=false;
                continue;
            }
        }
    }
}

void MainWindow::apply_way_filter()
{
    double radius=ui->eradius_of_intercept->text().toInt()*1000; //input is in kilometers
    if (radius<0.0)radius=0.0;
    for (int i=0;i<groups_count;i++){
        TGroup* tg=groups[i];
        tg->filter_draw_group=false;
        tg->filter_draw_way=false;
        if (radius==0.0) continue;
        if (tg->units[0].is_german)continue;
        double d=tg->way.min_distance_to(my_boat.coord);
        if (d<=radius){
            tg->filter_draw_group=true;
            tg->filter_draw_way=true;
        }
    }
}

void MainWindow::apply_type_filter(int type)
{
    apply_way_filter();//restore ways for all types
    for (int i=0;i<groups_count;i++){
        TGroup* tg=groups[i];
        if (tg->has_type(type)){
            tg->filter_draw_group=true;
        }else{
            tg->filter_draw_group =false;
            tg->filter_draw_way   =false;//don't draw useless ways
        }
    }
}

//---------------------------------------------------------------------------


double line_min_distance(const TGameCoord& a,const TGameCoord& b,const TGameCoord& d)
{
    if( (a-b).len_sqr()<1e-6)return 1e99;//too short line, dont calculate.
    TGameCoord v=(b-a);
    double proj= (v*(d-a))/v.len_sqr();
    if (proj<0)return (d-a).len();
    if (proj>1)return (d-b).len();
    return (a+v*proj-d).len();//nearest point is somewhere in the line cut
}

double TWay::min_distance_to(const TGameCoord& dest)
{
    size_t size=data.size();
    double min_r=1e99;
    for (size_t i=0;i<size-1;i++){
        double r=line_min_distance(data[i],data[i+1],dest);
        if (r<min_r) min_r=r;
    }
    return min_r;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------


bool TGroup::has_type(int type)
{
    if (type<=0)return true;
    for(int i=0;i<size;i++){
        if (units[i].type==type)return true;
    }
    return false;
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    DecimalSeparator='.';
    coords_unit_init();
    logptr=ui->textEdit_Memo1;// very dangerous exporting, dirty and quick
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this
                                                    , "Open Save Game"
, "C:\\Users\\dinozaur\\Documents\\SH5\\data\\cfg\\SaveGames\\"
                                                    , "Save Game (*.mis)"
                                                    );
    ui->e_coords->setText(fileName);

//    if (!OpenDialog1->Execute())return;
//    file_handle=CreateFile(
//        OpenDialog1->FileName.c_str(),
//        GENERIC_READ,0,NULL,OPEN_EXISTING,
//        FILE_FLAG_SEQUENTIAL_SCAN,NULL);
//    if (file_handle==INVALID_HANDLE_VALUE){return;}
//    load_file();
//    CloseHandle(file_handle);
//    btn_to_boatClick(Sender);

}

void MainWindow::on_pbToBoat_clicked()
{
    //write To Boat code here
//    void __fastcall TForm1::btn_to_boatClick(TObject *Sender)
//    {
//        viewpoint.m=e_coef->Text.ToDouble();
//        viewpoint.x=my_boat.coord.x;
//        viewpoint.y=my_boat.coord.y;
//        redraw();
//    }

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
    if(!ok){
//        apply_type_filter(0);//draw all types
    }else{
//            apply_type_filter(test);
    }
//    redraw();

}

void TUnit::load(){
    TStringList ls;
    int res=read_entity_list1(ls);
    if (res==MY_EOF)return;
    load(ls);
}

void TUnit::load(const TStringList& ls){
//    try{
        heading=read_double(ls,"Heading");
        speed=read_double(ls,"Speed");
        s_name=ls.value("Name");
        s_class=ls.value("Class");
        type=stoi(ls.value("Type"));
        coord_load(coord,ls);
        if (type<=199 && type>=100)is_warship=false; else is_warship=true;
        if (ls.value("Origin")=="German")is_german=true; else is_german=false;
//    }catch(EConvertError& E){
//        QMessageBox::warning(NULL,"Exception encountered","exception while loading Unit");
//    };
}

void TUnit::dump(QTextEdit* d) const{
    QString s;
    s=QString("Type=%1 class=%2 name=%3").arg(type).arg(s_class.c_str()).arg(s_name.c_str());
    d->append(s);
    s.sprintf("Heading: %f Speed: %f",heading,speed);
    d->append(s);
    s.sprintf("Long: %f Lat: %f",coord.x,coord.y);
    d->append(s);
}
