/******************************************************************************
NAME: ias_math_compute_t_confidence.c

PURPOSE: Contains routines to compute the Student's T-distribution confidence
    interval.  The main routine is ias_math_compute_t_confidence.

******************************************************************************/
#include <math.h>
#include "ias_math.h"

static double gammln(double);

/******************************************************************************
NAME: t_pdf

PURPOSE: Computes a probability density function value.

******************************************************************************/
static double t_pdf
(
    double t,      /* I: value at which to evaluate t dist PDF */
    int dof        /* I: t distribution degrees of freedom */
)
{
    double density;    /* probability density function value */
    double pi;         /* calculated value of pi */

    pi = ias_math_get_pi();
    density = exp(gammln((dof + 1)/2.0) - gammln(dof/2.0) 
            - log(pi * (double)dof)/2.0
            - log(1.0e0 + t * t/(double)dof) * (double)(dof + 1)/2.0e0);

    return density;
}

/******************************************************************************
NAME: gammln

PURPOSE: Helper routine for calculating the probability density function.

******************************************************************************/
static double gammln
(
    double xx            /* I: value to evaluate */
)
{
    int j;               /* Loop variable */
    double x,y,tmp,ser;
    static const double cof[6] = {76.18009172947146,-86.50532032941677,
     24.01409824083091, -1.231739572450155, 0.1208650973866179e-2, 
     -0.5395239384953e-5};

    y = x = xx;
    tmp = x + 5.5;
    tmp -= (x + 0.5) * log(tmp);
    ser = 1.000000000190015;
    for (j = 0 ; j <= 5 ; j++)
        ser += cof[j] / ++y;
    return(-tmp + log(2.5066282746310005 * ser / x));
}

/******************************************************************************
NAME: ias_math_compute_t_confidence

PURPOSE: Computes a confidence interval for Student's T distribution.

Returns: T confidence value

******************************************************************************/
double ias_math_compute_t_confidence
(
    double threshold,       /* I: Probability threshold for interval */
    int dof                 /* I: T distribution degrees of freedom */
)
{
    double step;            /* integration step size */
    double sum;             /* integration sum */
    double target;          /* target of integration sum */
    double index;           /* running variable */
    double t1, t2;          /* temporary pdf values */
    double delta;           /* integration increment for a single step */
    double a, b, c;         /* quadratic formula variables */

    /* Initialize the pdf integrating variables. */
    step = 0.001;
    sum = 0.0;
    target = threshold/2.0;
    index = 0.0;

    /* Set up the integration loop. */
    t1 = t_pdf(index, dof);
    t2 = t_pdf(index + step, dof);
    delta = step * (t1 + t2)/2.0;

    /* Loop and integrate until we reach the target. */
    while (sum + delta < target)
    {
        sum += delta;
        index += step;
        t1 = t2;
        t2 = t_pdf(index + step, dof);
        delta = step * (t1 + t2)/2.0;
    }

    /* Now find the size of the last step needed to hit the target. */
    t1 = t_pdf(index, dof);
    t2 = t_pdf(index + step, dof);

    /* Use the quadratic formula to solve for the step size assuming
     * the function is linear between t1 and t2. */
    a = (t2 - t1)/step/2.0;
    b = t1;
    c = sum - target;
    if (fabs(a) > 0.0)
        step = (-b + sqrt(b * b - 4.0 * a * c))/a/2.0;
    else if (fabs(b) > 0.0)
        step = -c/b;
    else
        step = 0.0;
    index += step;

    /* Send back the final running variable value. */
    return index;
}

