/******************************************************************************
NAME: ias_math_eval_legendre

PURPOSE:
Evaluate legendre polynomials. This function will evaluate up to the first
4 Legrende polynomials and return the sum of these calculated at point x using
the coefficients passed in the array.

RETURN VALUE:   Returns the sum of the Legendre Polynomials at the point x
                with the coefficients passed in the array.

******************************************************************************/
#include "ias_math.h"
#include "ias_logging.h"

double ias_math_eval_legendre 
(
    double x,                             /* I: Point to compute values at */
    const double *coefficients,           /* I: Array of coefficients */
    int num_coefficients                  /* I: Number of coefficients */
)
{

    double sum_value;  /* value to return */
    
    /* check for num coefficients being 4 first and do that as this is the
       case that is called the most */
    if (num_coefficients == 4)
    {
        sum_value = coefficients[0] + (coefficients[1] * x) +
                    (coefficients[2] * (1.5 * x * x - 0.5)) +
                    (coefficients[3] * (x * (2.5 * x * x - 1.5)));
        return sum_value;
    }
    else
    {
        sum_value = coefficients[0];
        if ( num_coefficients > 1)
        {
            sum_value += coefficients[1] * x;
        }
        if (num_coefficients > 2)
        {
            sum_value += coefficients[2] * (1.5 * x * x - 0.5);
        }

        if (num_coefficients > 4)
        {
            IAS_LOG_WARNING("Number of coefficients greater than 4 not "
                            "supported: %d", num_coefficients);
        }

        return sum_value;
    }
}
