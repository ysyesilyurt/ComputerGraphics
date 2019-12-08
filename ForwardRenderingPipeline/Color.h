#ifndef __COLOR_H__
#define __COLOR_H__

#include <iostream>

class Color
{
public:
    double r, g, b;

    Color();
    Color(double r, double g, double b);
    Color(const Color &other);
    Color operator+(const Color &);
    Color operator-(const Color &);
    Color operator*(double);
    Color operator/(double);
    Color & round();
    void swap(Color &);
    friend std::ostream& operator<<(std::ostream& os, const Color& c);
};

#endif