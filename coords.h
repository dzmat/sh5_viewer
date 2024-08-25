#ifndef COORDS_H
#define COORDS_H
#include <math.h>
// ---------------------------------------------------------------------------

class TGameCoord
{
public:
    TGameCoord()  {     x = y = 0.0;   }
    explicit TGameCoord(const double ix, const double iy) : x(ix), y(iy){}
    //    explicit TGameCoord(const my_coord &rhs) : x(rhs.x),y(rhs.y){}

    const TGameCoord operator+(const TGameCoord &rhs) const
    {
        return TGameCoord(x + rhs.x, y + rhs.y);
    };
    TGameCoord &operator+=(const TGameCoord &rhs)
    {
        x += rhs.x;
        y += rhs.y;
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
    double x, y;
};


#endif // COORDS_H
