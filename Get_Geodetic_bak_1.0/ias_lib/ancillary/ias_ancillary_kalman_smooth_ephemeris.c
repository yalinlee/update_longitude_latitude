/******************************************************************************
NAME: ias_ancillary_kalman_smooth_ephemeris

PURPOSE: Run the Kalman filter on the ephemeris data, 
         and smooth output from the Kalman filter.

RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion
ERROR    Operation failed

NOTES:
  This file contains the matrix operations needed for Kalman 
  filtering.  For the notes that preceed each function the 
  following definitions are used.

  [X]  -> State matrix
  [S]  -> State transition matrix
  [P]  -> Error covariance matrix  
  [Q]  -> Covariance matrix of state vector noise
  [R]  -> Covariance matrix of state vector noise
  [H]  -> Matrix relating state matrix to measurement vector
  [K]  -> Kalman gain matrix
  [Z]  -> Measurement matrix
  [I]  -> Identity matrix
  [_]  -> Predicted 
  [ ]' -> Transpose of matrix
  [ ]^ -> Inverse of matrix

ALGORITHM REFERENCES:
Introduction to Random Signal Analysis and Kalman Filtering, 
by Robert Grover Brown (p. 195)
******************************************************************************/

#include <stdlib.h>
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_geo.h"
#include "ias_ancillary_private.h"

/* defines to show the size of matrices being used */
#define M_SIZE          6
#define N_SIZE          6

/* define number of steps used to propagate data */
#define NUM_STEPS       10

/* defines for filter noise parameters */
#define PNOISE_POS_SIGMA        5.0
#define PNOISE_VEL_SIGMA        0.5
#define ONOISE_POS_SIGMA        1.0 
#define ONOISE_VEL_SIGMA        0.02
#define SNOISE_POS_SIGMA        25.0
#define SNOISE_VEL_SIGMA        8.0

/* To help cleanup the code, created this to free the memory for errors and
   at the end. */
#define FREE_MEMORY() \
    free(X); \
    free(Xk1); \
    free(P); \
    free(Pn1); \
    free(tmpx); \
    free(tmpy); \
    free(tmpz); \
    free(tmpxv); \
    free(tmpyv); \
    free(tmpzv)

int ias_ancillary_kalman_smooth_ephemeris
(
    IAS_CPF *cpf,                   /* I: CPF structure */
    int valid_ephemeris_count,      /* I: number of ephemeris points in arrays*/
    const double *ephemeris_time_data, /* I: array of ephemeris time data */
    const IAS_VECTOR *raw_pos,      /* I: array of ephemeris position data */
    const IAS_VECTOR *raw_vel,      /* I: array of ephemeris velocity data */
    double ephemeris_sampling_rate, /* I: ephemeris sampling interval */
    IAS_VECTOR *smoothed_eph_pos,   /* O: smoothed ephemeris position */
    IAS_VECTOR *smoothed_eph_vel    /* O: smoothed ephemeris velocity */
)
{
    typedef struct filter_vector
    {   
        double d[M_SIZE];
    } FILTER_VECTOR;
    
    typedef struct filter_matrix
    {   
        double d[M_SIZE * M_SIZE];
    } FILTER_MATRIX;

    double dt;                      /* Time between ephemeris samples */
    double dt2;                     /* For dt * dt */
    double dt3;                     /* For dt * dt * dt */
    double dt4;                     /* For dt * dt * dt * dt */
    int kcount, gps_count, i, j, k; /* Counters */
    int scnt;                       /* Counter for smoothed data */
    int status;                     /* status of return from a function */
    double *ptr_x;                  /* Pointer to smoothed X position */
    double *ptr_y;                  /* Pointer to smoothed Y position */
    double *ptr_z;                  /* Pointer to smoothed Z position */
    double *ptr_xv;                 /* Pointer to smoothed X velocity */
    double *ptr_yv;                 /* Pointer to smoothed Y velocity */
    double *ptr_zv;                 /* Pointer to smoothed Z velocity */
    double *tmpx;                   /* Temporary data buffer */
    double *tmpy;                   /* Temporary data buffer */
    double *tmpz;                   /* Temporary data buffer */
    double *tmpxv;                  /* Temporary data buffer */
    double *tmpyv;                  /* Temporary data buffer */
    double *tmpzv;                  /* Temporary data buffer */
    double acc_x;                   /* Gravity model acceleration in X */
    double acc_y;                   /* Gravity model acceleration in Y */
    double acc_z;                   /* Gravity model acceleration in Z */
    double sigma_x;                 /* Standard dev. in X for process noise */
    double sigma_y;                 /* Standard dev. in Y for process noise */
    double sigma_z;                 /* Standard dev. in Z for process noise */
    double sigma_xv;                /* Standard dev. in XV for process noise */
    double sigma_yv;                /* Standard dev. in YV for process noise */
    double sigma_zv;                /* Standard dev. in ZV for process noise */
    double step_X[M_SIZE];          /* Array used to shorten length of 
                                       prediction calc */
    double step_Xk1[M_SIZE];        /* Array used to shorten length of 
                                       prediction calc */
    double time_step;               /* Time step used for prediction calc */
    double time;                    /* Total accumulation of time in 
                                       prediction calc */
    double frac;                    /* Fraction of step of prediction calc */
    
    const struct IAS_CPF_EARTH_CONSTANTS *earth_constant = NULL; 

    double Pn[M_SIZE*M_SIZE];  /* Predicted error cov matrix at time k */
    double K[M_SIZE*N_SIZE];   /* Kalman gain matrix */
    double H[M_SIZE*N_SIZE];   /* M_SIZE relating state to measurement */
    double S[M_SIZE*M_SIZE];   /* State transition matrix */
    double A[M_SIZE*M_SIZE];   /* Smoothing gain matrix */
    double Q[M_SIZE*M_SIZE];   /* Process noise */
    double R[N_SIZE*N_SIZE];   /* Measurment noise */
    double Xk[M_SIZE];         /* State matrix at time k */
    FILTER_VECTOR *X;          /* Matrix containing states at all k */
    FILTER_VECTOR *Xk1;        /* Matrix containing states at all k+1 */
    FILTER_MATRIX *P;          /* Filtered error covar matrix */
    FILTER_MATRIX *Pn1;        /* Predicted error covar matrix at k+1*/
    double XN[M_SIZE];         /* Estimate of state [X] up to N */
    double XN1[M_SIZE];        /* Estimate of state [X] up to N+1 */
    double z[N_SIZE];          /* Measured value matrix */
    double pre[N_SIZE];        /* Predicted measurement(from predicted state) */

    earth_constant = ias_cpf_get_earth_const(cpf);
    if (earth_constant == NULL)
    {
        IAS_LOG_ERROR("Reading earth constants from the CPF");
        return ERROR;
    }

    /* Allocate local arrays and structures */
    X = (FILTER_VECTOR *)
        malloc(valid_ephemeris_count * sizeof(FILTER_VECTOR));
    if (X == NULL)
    {
        IAS_LOG_ERROR("Allocating state vector array");
        return ERROR;
    }

    Xk1 = (FILTER_VECTOR *)
        malloc(valid_ephemeris_count * sizeof(FILTER_VECTOR));
    if (Xk1 == NULL)
    {
        IAS_LOG_ERROR("Allocating next state vector array");
        free(X);
        return ERROR;
    }

    P = (FILTER_MATRIX *)
        malloc(valid_ephemeris_count * sizeof(FILTER_MATRIX));
    if (P == NULL)
    {
        IAS_LOG_ERROR("Allocating filtered covariance array");
        free(X);
        free(Xk1);
        return ERROR;
    }

    Pn1 = (FILTER_MATRIX *)
        malloc(valid_ephemeris_count * sizeof(FILTER_MATRIX));
    if (Pn1 == NULL)
    {
        IAS_LOG_ERROR("Allocating predicted covariance array");
        free(X);
        free(Xk1);
        free(P);
        return ERROR;
    }

    tmpx = (double *)malloc(valid_ephemeris_count * sizeof(double));
    tmpy = (double *)malloc(valid_ephemeris_count * sizeof(double));
    tmpz = (double *)malloc(valid_ephemeris_count * sizeof(double));
    if (tmpx == NULL || tmpy == NULL || tmpz == NULL)
    {
        IAS_LOG_ERROR("Allocating temporary position arrays");
        free(X);
        free(Xk1);
        free(P);
        free(Pn1);
        free(tmpx);
        free(tmpy);
        free(tmpz);
        return ERROR;
    }

    tmpxv = (double *)malloc(valid_ephemeris_count * sizeof(double));
    tmpyv = (double *)malloc(valid_ephemeris_count * sizeof(double));
    tmpzv = (double *)malloc(valid_ephemeris_count * sizeof(double));
    if (tmpxv == NULL || tmpyv == NULL || tmpzv == NULL)
    {
        IAS_LOG_ERROR("Allocating temporary velocity arrays");
        FREE_MEMORY();
        return ERROR;
    }

    /* zero out the matricies */
    for (i = 0; i < M_SIZE * M_SIZE; i++)
    {
        Pn[i] = 0.0;
        K[i] = 0.0;
        H[i] = 0.0;
        S[i] = 0.0;
        A[i] = 0.0;
        Q[i] = 0.0;
        R[i] = 0.0;

        for (j = 0; j < valid_ephemeris_count; j++)
        {
            P[j].d[i] = 0.0;
            Pn1[j].d[i] = 0.0;
        }
    }

    /* Set up state transition matrix. */
    dt = ephemeris_time_data[1] - ephemeris_time_data[0];
    S[0] = 1.0; S[1]  = 0.0; S[2]  = 0.0; S[3]  = dt;  S[4]  = 0.0; S[5]  = 0.0;
    S[6] = 0.0; S[7]  = 1.0; S[8]  = 0.0; S[9]  = 0.0; S[10] = dt;  S[11] = 0.0;
    S[12]= 0.0; S[13] = 0.0; S[14] = 1.0; S[15] = 0.0; S[16] = 0.0; S[17] = dt;
    S[18]= 0.0; S[19] = 0.0; S[20] = 0.0; S[21] = 1.0; S[22] = 0.0; S[23] = 0.0;
    S[24]= 0.0; S[25] = 0.0; S[26] = 0.0; S[27] = 0.0; S[28] = 1.0; S[29] = 0.0;
    S[30]= 0.0; S[31] = 0.0; S[32] = 0.0; S[33] = 0.0; S[34] = 0.0; S[35] = 1.0;

    for (i = 0; i < 3; i++)
    {
        j = i * M_SIZE;
        Pn[j + i] = SNOISE_POS_SIGMA * SNOISE_POS_SIGMA;
        /* Set Pn1 in case smoother is set to go from
           valid_ephemeris_count-1 to 0 */
        Pn1[0].d[j + i] = Pn[j + i]; 
        R[j + i] = ONOISE_POS_SIGMA * ONOISE_POS_SIGMA;
    }

    for (i = 3; i < M_SIZE; i++)
    {
        j = i * M_SIZE;
        Pn[j + i] = SNOISE_VEL_SIGMA * SNOISE_VEL_SIGMA;
        /* Set Pn1 in case smoother is set to go from
           valid_ephemeris_count-1 to 0 */
        Pn1[0].d[j + i] = Pn[j + i]; 
        R[j + i] = ONOISE_VEL_SIGMA * ONOISE_VEL_SIGMA;
    }

    for (i = 0; i < M_SIZE; i++)
        H[i * M_SIZE + i] = 1.0;

    /* Initialize state */
    /* Set Xk1 in case smoother is set to go from
       valid_ephemeris_count - 1 to 0 */
    Xk1[0].d[0] = Xk[0] = raw_pos[0].x;
    Xk1[0].d[1] = Xk[1] = raw_pos[0].y;
    Xk1[0].d[2] = Xk[2] = raw_pos[0].z;
    Xk1[0].d[3] = Xk[3] = raw_vel[0].x;  
    Xk1[0].d[4] = Xk[4] = raw_vel[0].y;  
    Xk1[0].d[5] = Xk[5] = raw_vel[0].z;  

    sigma_x  = PNOISE_POS_SIGMA;
    sigma_y  = PNOISE_POS_SIGMA;
    sigma_z  = PNOISE_POS_SIGMA;
    sigma_xv = PNOISE_VEL_SIGMA;
    sigma_yv = PNOISE_VEL_SIGMA;
    sigma_zv = PNOISE_VEL_SIGMA;

    /* valid_ephemeris_count = valid_ephemeris_count - 1; */
    for (gps_count = 0, kcount = 0;
         gps_count < valid_ephemeris_count;
         kcount++, gps_count++)
    {
    
        /* Set up measurement matrix */
        z[0] = raw_pos[gps_count].x;
        z[1] = raw_pos[gps_count].y;
        z[2] = raw_pos[gps_count].z;
        z[3] = raw_vel[gps_count].x;
        z[4] = raw_vel[gps_count].y;
        z[5] = raw_vel[gps_count].z;
    
        if (gps_count < valid_ephemeris_count - 1)
        {
            dt = ephemeris_time_data[gps_count + 1] -
                 ephemeris_time_data[gps_count];
        }
        else 
            dt = ephemeris_sampling_rate;

        /* do some of the multiplications up front */
        dt2 = dt * dt;
        dt3 = dt2 * dt;
        dt4 = dt3 * dt;

        /* Set up process noise.  */
        Q[0]  = (dt2 * sigma_x * sigma_x) + ((dt4 * sigma_xv * sigma_xv) / 4.0);
        Q[3]  = (dt3 * sigma_xv * sigma_xv) / 2.0;
        Q[7]  = (dt2 * sigma_y * sigma_y) + ((dt4 * sigma_yv * sigma_yv) / 4.0);
        Q[10] = (dt3 * sigma_yv * sigma_yv) / 2.0;
        Q[14] = (dt2 * sigma_z * sigma_z) + ((dt4 * sigma_zv * sigma_zv) / 4.0);
        Q[17] = (dt3 * sigma_zv * sigma_zv) / 2.0;
        Q[18] = (dt3 * sigma_xv * sigma_xv) / 2.0;
        Q[21] = (dt2 * sigma_xv * sigma_xv);
        Q[25] = (dt3 * sigma_yv * sigma_yv) / 2.0;
        Q[28] = (dt2 * sigma_yv * sigma_yv);
        Q[32] = (dt3 * sigma_zv * sigma_zv) / 2.0;
        Q[35] = (dt2 * sigma_zv * sigma_zv);

        /* Filter data.  */
        status = ias_math_compute_kalman_gain(Pn, H, R, K, M_SIZE, N_SIZE);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Returned from ias_math_compute_kalman_gain");
            FREE_MEMORY();
            return ERROR;
        }

        status = ias_math_update_filter_state(Xk, &X[kcount].d[0], K, z, H, 
                                              M_SIZE, N_SIZE);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Returned from ias_math_update_filter_state");
            FREE_MEMORY();
            return ERROR;
        }

        status = ias_math_update_filter_error_covar(K, H, Pn, &P[kcount].d[0],  
                                                    M_SIZE, N_SIZE);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Returned from ias_math_update_filter_error_covar");
            FREE_MEMORY();
            return ERROR;
        }

        S[3]  = dt;
        S[10] = dt;
        S[17] = dt;

        /* Predict error covariance and state matrix for next iteration. */
        status = ias_math_compute_predicted_error_covar(S, &P[kcount].d[0], 
                                               &Pn1[kcount].d[0], Q, M_SIZE);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Returned from "
                          "ias_math_compute_predicted_error_covar");
            FREE_MEMORY();
            return ERROR;
        }

        for (k = 0; k < M_SIZE; k++)
            step_X[k] = X[kcount].d[k];
    
        time_step = 0.0;
        time      = 0.0;
        /* Propagate state in steps if necessary. */
        for (k = 0; k < NUM_STEPS; k++)
        {
            frac = ((double)k + 1) / NUM_STEPS;
            time_step = frac * dt - time;
    
            /* Setup up state matrix */
            S[3]  = time_step;
            S[10] = time_step;
            S[17] = time_step;

            status = ias_math_predict_state(S, step_X, step_Xk1, M_SIZE);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Returned from ias_math_predict_state");
                FREE_MEMORY();
                return ERROR;
            }

            /* Find force (acceleration) due to Earth's potential.  */
            acc_x = ias_geo_compute_earth_second_partial_x(
                earth_constant->semi_major_axis,
                earth_constant->gravity_constant, 
                step_X[0], step_X[1], step_X[2]);
            acc_y = ias_geo_compute_earth_second_partial_y(
                earth_constant->semi_major_axis,
                earth_constant->gravity_constant, 
                step_X[0], step_X[1], step_X[2]);
            acc_z = ias_geo_compute_earth_second_partial_z(
                earth_constant->semi_major_axis,
                earth_constant->gravity_constant, 
                step_X[0], step_X[1], step_X[2]);

            step_X[0] = step_Xk1[0];
            step_X[1] = step_Xk1[1];
            step_X[2] = step_Xk1[2];
            step_X[3] = step_Xk1[3] + acc_x * time_step;
            step_X[4] = step_Xk1[4] + acc_y * time_step;
            step_X[5] = step_Xk1[5] + acc_z * time_step;

            time = time + time_step;
        }

        for (k = 0; k < M_SIZE; k++)
            Xk1[kcount].d[k] = step_X[k];

        status = ias_math_multiply_matrix(H, &Xk1[kcount].d[0], pre, N_SIZE, 
                                          M_SIZE, M_SIZE, 1);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Returned from ias_math_multiply_matrix");
            FREE_MEMORY();
            return ERROR;
        }

        for (j = 0; j < M_SIZE; j++)
            Xk[j] = Xk1[kcount].d[j];

        for (j = 0; j < M_SIZE * M_SIZE; j++)
            Pn[j] = Pn1[kcount].d[j];
    }

    ptr_x  = tmpx;
    ptr_y  = tmpy;
    ptr_z  = tmpz;
    ptr_xv = tmpxv;
    ptr_yv = tmpyv;
    ptr_zv = tmpzv;

    for (j = 0; j < M_SIZE; j++)
        XN[j] = Xk1[kcount - 1].d[j];

    /* Smooth from valid_ephemeris_count to 0 since the first measurement
       was used for initial conditions.  Kalman went from k=0
      (corresponding to second ephemris point) to valid_ephemeris_count-1. */
    dt = ephemeris_sampling_rate;
    for (scnt = kcount - 1; scnt >= 0; scnt--)
    {
        /* Setup up state matrix */
        S[3]  = dt;
        S[10] = dt;
        S[17] = dt;

        /* Smooth data. */
        status = ias_math_smooth_gain(P[scnt].d, Pn1[scnt].d, S, A, M_SIZE);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Returned from ias_math_smooth_gain");
            FREE_MEMORY();
            return ERROR;
        }
    
        status = ias_math_smooth_state(X[scnt].d, Xk1[scnt].d, XN, A, XN1, 
                                       M_SIZE);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Returned from ias_math_smooth_state");
            FREE_MEMORY();
            return ERROR;
        }
    
        *ptr_x = XN1[0];
        ptr_x++;

        *ptr_y = XN1[1];
        ptr_y++;

        *ptr_z = XN1[2];
        ptr_z++;

        *ptr_xv = XN1[3];
        ptr_xv++;

        *ptr_yv = XN1[4];
        ptr_yv++;

        *ptr_zv = XN1[5];
        ptr_zv++;
    
        for (j = 0; j < M_SIZE; j++)
            XN[j] = XN1[j];
    
        if (scnt > 0)
            dt = ephemeris_time_data[scnt] - ephemeris_time_data[scnt - 1];
    }

    /* Reverse the order of the smoothed data set.  The smoothing algorithm
       starts with the last data sample and works forward.  */
    /* Ignoring first (last point smoothed) smoothed point, this is throwing
       off the fit_smooth routine.  */
    for (i = kcount - 1, k = 0; i >= 0; i--, k++)
    {
        smoothed_eph_pos[k].x = tmpx[i];
        smoothed_eph_pos[k].y = tmpy[i];
        smoothed_eph_pos[k].z = tmpz[i];
        smoothed_eph_vel[k].x = tmpxv[i];
        smoothed_eph_vel[k].y = tmpyv[i];
        smoothed_eph_vel[k].z = tmpzv[i];
    }

    FREE_MEMORY();

    return SUCCESS;
}
