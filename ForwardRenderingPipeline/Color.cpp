#include "Color.h"
#include <iostream>
#include <iomanip>

using namespace std;

Color::Color() {}

Color::Color(double r, double g, double b)
{
    this->r = r;
    this->g = g;
    this->b = b;
}

Color::Color(const Color &other)
{
    this->r = other.r;
    this->g = other.g;
    this->b = other.b;
}

Color Color::operator+(const Color & rhs) {
    Color c;
    c.r = r + rhs.r;
    c.g = g + rhs.g;
    c.b = b + rhs.b;
    return c;
}

Color Color::operator-(const Color & rhs) {
    Color c;
    c.r = r - rhs.r;
    c.g = g - rhs.g;
    c.b = b - rhs.b;
    return c;
}

Color Color::operator*(double num) {
    Color c;
    c.r = r*num;
    c.g = g*num;
    c.b = b*num;
    return c;
}

Color Color::operator/(double num) {
    Color c;
    c.r = r/num;
    c.g = g/num;
    c.b = b/num;
    return c;
}

Color & Color::round(){
    r = (int)(r + 0.5);
    g = (int)(g + 0.5);
    b = (int)(b + 0.5);
    return *this;
}

void Color::swap(Color & rhs) {
    std::swap(r,rhs.r);
    std::swap(g,rhs.g);
    std::swap(b,rhs.b);
}

ostream& operator<<(ostream& os, const Color& c)
{
    os << fixed << setprecision(0) << "rgb(" << c.r << ", " << c.g << ", " << c.b << ")";
    return os;
}
