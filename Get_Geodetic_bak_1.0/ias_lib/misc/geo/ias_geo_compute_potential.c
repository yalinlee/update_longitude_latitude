/*****************************************************************************
NAME: ias_geo_compute_potential

PURPOSE:
Calculates the gravitational potential of the Earth.  This
is represented by the second order partial derivatives of 
position (X,Y,Z) or acceleration of X, Y, Z.  

p2x - second derivative of X (acceleration)
p2y - second derivative of Y (acceleration)
p2z - second derivative of Z (acceleration)

******************************************************************************/
#include <math.h>
#include "ias_geo.h"

#define J2 1.08262668e-03
#define J3 -2.53265649e-06
#define J4 -1.61962159e-06
#define J5 -2.27296083e-07
#define J6 5.40681239e-07
#define J7 -3.52359908e-07
#define C21 2.414000e-10
#define S21 -1.543100e-09
#define C22 -1.574460e-06
#define S22 9.038038e-07

double ias_geo_compute_earth_second_partial_x
(
    double semi_major_axis, /* I: Earth's semi-major axis */
    double gravity_constant,/* I: Earth's gravitational constant */
    double x,    /* I: Position in X */
    double y,    /* I: Postion in Y */
    double z     /* I: Postion in Z */
)
{
    double temp,f;
    double p21,p22,p1,p2,p3,p4,p5,p6;
    double re,re2,re3,re4,re5,re6;
    double z2,z3,z4,z5,z6;
    double r,r2,r3,r4,r5,r6,r7,r8,r9;
    double c,ct,d;
    double gm;

    re = semi_major_axis;
    gm = gravity_constant;

    re2 = re * re;
    re3 = re2 * re;
    re4 = re3 * re;
    re5 = re4 * re;
    re6 = re5 * re;

    r = sqrt( x * x + y * y + z * z );
    r2  = r * r;
    r3  = r2 * r;
    r4  = r3 * r;
    r5  = r4 * r;
    r6  = r5 * r;
    r7  = r6 * r;
    r8  = r7 * r;
    r9  = r8 * r;

    z2 = z * z;
    z3 = z2 * z;
    z4 = z3 * z;
    z5 = z4 * z;
    z6 = z5 * z;

    c = -gm * x / r3;
    p1 = 1.0;

    temp = re2 / r2;
    p2 = temp * ( ((5.0 * z2) / r2) - 1.0 );  
    p2 = -3.0 * p2 * J2 / 2.0;

    temp = re3 / r3;
    p3 = temp * ( ((-7.0 * z3) / r3) + ((3.0 * z) / r) );
    p3 =  5.0 * p3 * J3 / 2.0;

    temp = re4 / r4;
    p4 = temp * ( ((63.0 * z4) / r4) + ((-42.0 * z2) / r2) + 3.0 );
    p4 = -5.0 * p4 * J4 / 8.0;

    temp = re5 / r5;
    p5 = temp * ( ((231.0 * z5) / r5) + ((-210.0 * z3) / r3) +
                 ((35.0 * z) / r) );
    p5 = -3.0 * p5 * J5 / 8.0;

    temp = re6 / r6;
    p6 = temp * ( ((-3003.0 * z6) / r6) + ((3465.0 * z4) / r4) +
                 ((-945.0 * z2) / r2) + 35.0 );
    p6 = p6 * J6 / 16.0;

    f = c * ( p1 + p2 + p3 + p4 + p5 + p6 );

    ct = -3.0 * gm  * re2 / r4;
    d = sqrt( x * x + y * y );
  
    p21 = C21 * ( z / r ) - 5.0 * C21 * ( ( x * x * z ) / r3 ) - 
        5.0 * S21 * ( ( x * y * z ) / r3);
    p21 = ct * p21;
 
    p22 = 2.0 * C22 * ( x / r ) - 5.0 * C22 * ( x * ( x * x - y * y ) / r3 ) + 
        2.0 * S22 * ( y / r ) - 10.0 * S22 * ( ( x * x * y ) / r3 );
    p22 = ct * p22;

    f = f + p21 + p22;

    return f;
}

double ias_geo_compute_earth_second_partial_y
(
    double semi_major_axis, /* I: Earth's semi-major axis */
    double gravity_constant,/* I: Earth's gravitational constant */
    double x,    /* I: Postion in X */
    double y,    /* I: Postion in Y */
    double z     /* I: Postion in Z */
)
{
    double temp,f;
    double p21,p22,p1,p2,p3,p4,p5,p6;
    double re,re2,re3,re4,re5,re6;
    double z2,z3,z4,z5,z6;
    double r,r2,r3,r4,r5,r6,r7,r8,r9;
    double c,ct,d;
    double gm;

    re = semi_major_axis;
    gm = gravity_constant;

    re2 = re * re;
    re3 = re2 * re;
    re4 = re3 * re;
    re5 = re4 * re;
    re6 = re5 * re;

    r = sqrt( x * x + y * y + z * z );
    r2  = r * r;
    r3  = r2 * r;
    r4  = r3 * r;
    r5  = r4 * r;
    r6  = r5 * r;
    r7  = r6 * r;
    r8  = r7 * r;
    r9  = r8 * r;

    z2 = z * z;
    z3 = z2 * z;
    z4 = z3 * z;
    z5 = z4 * z;
    z6 = z5 * z;

    c = -gm * y / r3;
    p1 =  1.0;

    temp = re2 / r2;
    p2 = temp * ( ((5.0 * z2) / r2) - 1.0 );
    p2 = -3.0 * p2 * J2 / 2.0;

    temp = re3 / r3;
    p3 = temp * ( ((-7.0 * z3) / r3)  + (3.0 * z / r) );
    p3 =  5.0 * p3 * J3 / 2.0;
 
    temp = re4 / r4;
    p4 = temp * ( ((63.0 * z4) / r4) + ((-42.0 * z2) / r2) + 3.0 );
    p4 = -5.0 * p4 * J4 / 8.0;

    temp = re5 / r5; 
    p5 = temp * ( ((231.0 * z5) / r5) + ((-210.0 * z3) / r3) +
        ((35.0 * z) / r) );
    p5 = -3.0 * p5 * J5 / 8.0;

    temp = re6 / r6;
    p6 = temp * ( ((-3003.0 * z6) / r6) + ((3465.0 * z4) / r4) +
        ((-945.0 * z2) / r2) + 35.0 );
    p6 =  p6 * J6 / 16.0;

    f = c * ( p1 + p2 + p3 + p4 + p5 + p6 );

    ct = -3.0 * gm  * re2 / r4;
    d = sqrt( x * x + y * y );
  
    p21 = -5.0 * C21 * ((x * y * z)/ r3) + S21 * (z / r) - 
        5.0 * S21 * (( y * y * z ) / r3);
    p21 = ct * p21;

    p22 = -2.0 * C22 * ( y / r ) - 5.0 * C22 * ( y * (x * x - y * y) / r3) + 
        2.0 * S22 * (x / r) - 10.0 * S22 * (( y * y * x) / r3);
    p22 = ct * p22;

    f = f + p21 + p22;

    return f;
}

double ias_geo_compute_earth_second_partial_z
(
    double semi_major_axis, /* I: Earth's semi-major axis */
    double gravity_constant,/* I: Earth's gravitational constant */
    double x,    /* I: Postion in X */
    double y,    /* I: Postion in Y */
    double z     /* I: Postion in Z */
)
{ 
    double temp,f;
    double p21,p22,p1,p2,p3,p4,p5,p6;
    double re,re2,re3,re4,re5,re6;
    double z2,z3,z4,z5,z6,z7;
    double r,r2,r3,r4,r5,r6,r7,r8,r9;
    double c,ct,d;
    double gm;

    re = semi_major_axis;
    gm = gravity_constant;

    re2 = re * re;
    re3 = re2 * re;
    re4 = re3 * re;
    re5 = re4 * re;
    re6 = re5 * re;

    r = sqrt( x * x + y * y + z * z );
    r2  = r * r;
    r3  = r2 * r;
    r4  = r3 * r;
    r5  = r4 * r;
    r6  = r5 * r;
    r7  = r6 * r;
    r8  = r7 * r;
    r9  = r8 * r;

    z2 = z * z;
    z3 = z2 * z;
    z4 = z3 * z;
    z5 = z4 * z;
    z6 = z5 * z;
    z7 = z6 * z;

    c = -gm * z / r3;
    p1 =  1.0;

    temp = re2 / r2;
    p2 = temp * ( 3.0 - ((5.0 * z2) / r2) );
    p2 =  3.0 * p2 * J2 / 2.0;

    temp = re3 / r3;
    p3 = temp * ( ((30.0 * z) / r) - ((35.0 * z3) / r3) - ((3.0 * r) / z) );
    p3 =  p3 * J3 / 2.0;

    temp = re4 / r4;
    p4 = temp * ( ((-70.0 * z2) / r2) + ((63.0 * z4) / r4) + 15.0 );
    p4 = -p4 * J4 / 8.0;

    temp = re5 / r5;
    p5 = temp * ( ((-945.0 * z3) / r3) + ((693.0 * z5) / r5) + 
        ((315.0 * z) / r) + ((-15.0 * r ) / z) );
    p5 = -p5 * J5 / 8.0;

    temp = re6 / r6;
    p6 = temp * ( ((4851.0 * z4) / r4) - ((3003.0 * z6) / r6) - 
        ((2205.0 * z2) / r2) + 245.0 ); 
    p6 =  p6 * J6 / 16.0;

    f = c * ( p1 + p2 + p3 + p4 + p5 + p6 );

    ct = -3.0 * gm  * re2 / r4;
    d = sqrt( x * x + y * y );

    p21 = C21 * (x / r) - 5.0 * C21 * ((x * z * z) / r3) + S21 * (y / r) -
        5.0 * S21 * ((y * z * z) / r3);
    p21 = ct * p21;

    p22 = -5.0 * C22 * (z * (x * x - y * y) / r3) - 10.0 * S22 * ((z * y * x) / 
        r3);
    p22 = ct * p22;

    f = f + p21 + p22;

    return f; 
}
