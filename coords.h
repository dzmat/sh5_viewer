#ifndef COORDS_H
#define COORDS_H
#include <math.h>
// ---------------------------------------------------------------------------

struct st_viewpoint;
extern st_viewpoint viewpoint;

class my_coord;

class TGameCoord
{
public:
    TGameCoord()  {     x = y = 0.0;   }
    explicit TGameCoord(const double ix,const double iy) : x(ix),y(iy){}
//    explicit TGameCoord(const my_coord &rhs) : x(rhs.x),y(rhs.y){}

    my_coord g2i() const;
    const TGameCoord operator+(const TGameCoord &rhs) const
    {
        return TGameCoord(x + rhs.x, y + rhs.y);
    };
    TGameCoord& operator+=(const TGameCoord &rhs)
    {
        x+=rhs.x;
        y+=rhs.y;
        return *this;
    };
    const TGameCoord operator-(const TGameCoord &rhs) const
    {
        return TGameCoord(x - rhs.x, y - rhs.y);
    };
    const TGameCoord operator*(const double rhs) const;
    double operator*(const TGameCoord &rhs) const;
    const TGameCoord operator/(const double rhs) const;
    double len_sqr() const
    {
        return x * x + y * y;
    };
    double len() const
    {
        return sqrt(len_sqr());
    };

    /*    void load(TStringList* ls){
            x=read_double(ls,"Long");
            y=read_double(ls,"Lat");
        };*/
    double x,y;
};
struct st_viewpoint
{
    TGameCoord pos;
    double m;
    int o_x,o_y;
};

class my_coord
{
public:
    my_coord()
    {
        x = y = 0;
    }

    explicit my_coord(const double ix,const double iy) : x(ix),y(iy){}
    //explicit my_coord(const TGameCoord &rhs) : x(rhs.x),y(rhs.y)    {    }

    TGameCoord i2g() const;
    const my_coord operator+(const my_coord &rhs) const
    {
        return my_coord(x + rhs.x,y + rhs.y);
    };
    const my_coord operator-(const my_coord &rhs) const
    {
        return my_coord(x - rhs.x,y - rhs.y);
    };
    const my_coord operator*(const double rhs) const;
    double operator*(const my_coord &rhs) const;
    const my_coord operator/(const double rhs) const;
    double len_sqr() const
    {
        return x * x + y * y;
    };
    double len() const
    {
        return sqrt(len_sqr());
    };
    double x,y;
};

struct st_drag
{
    TGameCoord gc;// game coordinates
    int mX,mY;// mouse coordinates
    bool started;
};
extern st_drag drag;

void drag_start(int X,int Y);
void drag_end(int X,int Y);
void coords_unit_init();



#endif // COORDS_H
