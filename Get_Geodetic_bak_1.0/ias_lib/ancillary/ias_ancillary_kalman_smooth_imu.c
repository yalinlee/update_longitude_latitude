/******************************************************************************
NAME: ias_ancillary_kalman_smooth_imu

PURPOSE: Run the Kalman filter on the IMU and the EPA (Euler Parameter) 
         attitude data and smooth output from the Kalman filter.

RETURN VALUE: Type = int
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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ias_const.h"
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_ancillary_private.h"

#define NUM_AXIS 3 /* number of axis (X, Y, Z) */

/* defines to show the size of matricies being used */
#define M_SIZE 3
#define N_SIZE 3

/* defines for filter noise parameters */
#define PNOISE_ATTRATE_SIGMA 1.0e-2 /* attitude rate process noise in
                                       radians per second per second  */
#define PNOISE_DRIFT_SIGMA   1.0e-5 /* IMU drift rate process noise 
                                       in radians per second per second  */
#define ONOISE_EPA_SIGMA     5.0e+0 /* quaternion observation noise in
                                       arcsec */
#define ONOISE_GYRO_SIGMA    2.5e+0 /* IMU rate observation noise 
                                       in arcsec per second */
#define SNOISE_ATT_SIGMA     7.5    /* initial attitude state standard
                                       deviation in arcsec */
#define SNOISE_ATTRATE_SIGMA 31.0   /* initial attitude rate state standard
                                       deviation in arcsec per second */
#define SNOISE_DRIFT_SIGMA   5.0    /* initial drift state standard
                                       deviation in arcsec per second */
/* To help cleanup the code, created this to free the memory for errors and
   at the end. */
#define FREE_MEMORY() \
    free(X); \
    free(Xk1); \
    free(P); \
    free(Pn1); \
    free(imu_buffer); \
    free(att_buff[0]); \
    free(att_buff[1]); \
    free(att_buff[2]); \
    free(state)

int ias_ancillary_kalman_smooth_imu
(
    int quaternion_count,           /* I: Number of attitude measurements */
    int imu_count,                  /* I: Number of IMU points (array size) */
    const double *quaternion_time_data, /* I: Array of quaternion times */
    IAS_VECTOR *quaternion_data,    /* I/O: Array of quaternion data */
    int *valid_quaternion_flag,     /* I/O: Array of quaternion quality flags */
    double *imu_time_data,          /* I/O: Array of IMU seconds since j2000 */
    IAS_VECTOR *imu_data,           /* I/O: Array of input IMU and output 
                                         "smoothed" data */
    int *valid_imu_flag             /* I/O: Array of IMU data quality flags */
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

    double Pn[M_SIZE * M_SIZE]; /* predicted error cov matrix at time k */
    double K[M_SIZE * N_SIZE];  /* Kalman gain matrix */
    double H[M_SIZE * N_SIZE];  /* M_SIZE relating state to measurement */
    double S[M_SIZE * M_SIZE];  /* state transition matrix */
    double A[M_SIZE * M_SIZE];  /* smoothing gain matrix */
    double Q[M_SIZE * M_SIZE];  /* process noise */
    double R[N_SIZE * N_SIZE];  /* measurment noise */
    double Xk[M_SIZE];          /* state matrix at time k */
    FILTER_VECTOR *X;           /* matrix containing states at all k */
    FILTER_VECTOR *Xk1;         /* matrix containing states at all k+1 */
    FILTER_MATRIX *P;           /* filtered error covar matrix  */
    FILTER_MATRIX *Pn1;         /* predicted error covar matrix at k+1*/
    double XN[M_SIZE];          /* estimate of state [X] up to N */
    double XN1[M_SIZE];         /* estimate of state [X] up to N+1 */
    double z[N_SIZE];           /* measured value matrix */
    double pre[N_SIZE];         /* predicted measurement(from predicted state)*/
    static double *imu_buffer;  /* IMU data buffer */

    double *imu_ptr;           /* IMU pointer */
    double *att_buff[3];       /* temporary attitude data buffer */
    int *state;                /* flag stating what set of matrixes to use*/
    int axis, i, j, k;         /* counters */
    int n_size;                /* size of the arrays */
    int mf;                    /* the major frame in the pcd */
    int mf2;                   /* the IMU point in the major frame */
    int status;                /* status of return from a function */
    int quaternion_index;      /* counter for the current quaternion position */
    int imu_quat_ratio = rint(IAS_ANCILLARY_QUAT_TIME / IAS_ANCILLARY_IMU_TIME);
    int buffer_size;           /* maximum of the quaternion_count and imu_count
                                  for allocating buffers */

    /* get the a2r value from the math library */
    double a2r = ias_math_get_arcsec_to_radian_conversion();

#if DEBUG_GENERATE_DATA_FILES == 1
    FILE *fp; /* Output file for debugging */
#endif

    /* Find the largest buffer needed */
    buffer_size = quaternion_count;
    if (buffer_size < imu_count)
        buffer_size = imu_count;

    /* Allocate local arrays and structures */
    X = (FILTER_VECTOR *)malloc(imu_count * sizeof(FILTER_VECTOR));
    if (X == NULL)
    {
        IAS_LOG_ERROR("Allocating state vector array");
        return ERROR;
    }

    Xk1 = (FILTER_VECTOR *)malloc(imu_count * sizeof(FILTER_VECTOR));
    if (Xk1 == NULL)
    {
        IAS_LOG_ERROR("Allocating next state vector array");
        free(X);
        return ERROR;
    }

    P = (FILTER_MATRIX *)malloc(imu_count * sizeof(FILTER_MATRIX));
    if (P == NULL)
    {
        IAS_LOG_ERROR("Allocating filtered covariance array");
        free(X); free(Xk1);
        return ERROR;
    }

    Pn1 = (FILTER_MATRIX *)malloc(imu_count * sizeof(FILTER_MATRIX));
    if (Pn1 == NULL)
    {
        IAS_LOG_ERROR("Allocating predicted covariance array");
        free(X); free(Xk1);
        free(P);
        return ERROR;
    }

    imu_buffer = (double *)malloc(imu_count * sizeof(double));
    if (imu_buffer == NULL)
    {
        IAS_LOG_ERROR("Allocating temporary IMU array");
        free(X); free(Xk1); 
        free(P); free(Pn1);
        return ERROR;
    }

    att_buff[0] = (double *)malloc(buffer_size * sizeof(double));
    if (att_buff[0] == NULL)
    {
        IAS_LOG_ERROR("Allocating first temporary attitude array");
        free(X); free(Xk1); 
        free(P); free(Pn1);
        free(imu_buffer);
        return ERROR;
    }


    att_buff[1] = (double *)malloc(buffer_size * sizeof(double));
    if (att_buff[1] == NULL)
    {
        IAS_LOG_ERROR("Allocating second temporary attitude array");
        free(X); free(Xk1); 
        free(P); free(Pn1);
        free(imu_buffer);
        free(att_buff[0]);
        return ERROR;
    }


    att_buff[2] = (double *)malloc(buffer_size * sizeof(double));
    if (att_buff[2] == NULL)
    {
        IAS_LOG_ERROR("Allocating third temporary attitude array");
        free(X); free(Xk1); 
        free(P); free(Pn1);
        free(imu_buffer);
        free(att_buff[0]); free(att_buff[1]);
        return ERROR;
    }

    state = (int *)malloc(buffer_size * sizeof(int));
    if (state == NULL)
    {
        IAS_LOG_ERROR("Allocating temporary state array");
        free(X); free(Xk1);
        free(P); free(Pn1);
        free(imu_buffer);
        free(att_buff[0]); free(att_buff[1]); free(att_buff[2]);
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

        for(j = 0; j < imu_count; j++)
        {
            P[j].d[i] = 0.0;
            Pn1[j].d[i] = 0.0;
        }
    }

    /* Time synchronize the input EPA and IMU data. Use Lagrange
       interpolation to synchronize the samples. */
    if (imu_time_data[0] > quaternion_time_data[quaternion_count - 1] 
        || imu_time_data[imu_count - 1] < quaternion_time_data[0])
    {
        IAS_LOG_ERROR("IMU (%f,%f) and EPA (%f,%f) times do not coincide",
            imu_time_data[0], imu_time_data[imu_count - 1], 
            quaternion_time_data[0], 
            quaternion_time_data[quaternion_count - 1]);
        FREE_MEMORY();
        return ERROR;
    }

    /* Synchronize the EPA data */
    for (i = 0; i < quaternion_count; i++)
    {
        att_buff[0][i] = quaternion_data[i].x;
        att_buff[1][i] = quaternion_data[i].y;
        att_buff[2][i] = quaternion_data[i].z;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    fp = fopen("kal.epa.dat", "w");
#endif

    for (i = 0; i < quaternion_count; i++)
    {
        mf = (int)floor(((double)i * IAS_ANCILLARY_QUAT_TIME 
            - quaternion_time_data[0]) / IAS_ANCILLARY_QUAT_TIME);

        /* Since the incoming samples can be irregularly spaced, make sure
           we have the correct one by checking the times */
        while ((mf > 0) && (quaternion_time_data[mf] 
                    > (double)i * IAS_ANCILLARY_QUAT_TIME))
        {
            mf--;
        }
        while ((mf < quaternion_count - 1) && (quaternion_time_data[mf + 1]
                    < (double)i * IAS_ANCILLARY_QUAT_TIME))
        {
            mf++;
        }

        if (mf < 0) 
            mf = IAS_LAGRANGE_PTS / 2;

        if (mf > quaternion_count) 
            mf = quaternion_count - 1;

        mf2 = mf - IAS_LAGRANGE_PTS / 2;

        if (mf2 < 0) 
            mf2 = 0;

        if (mf2 > quaternion_count - IAS_LAGRANGE_PTS) 
            mf2 = quaternion_count - IAS_LAGRANGE_PTS;

        quaternion_data[i].x = ias_math_interpolate_lagrange(&att_buff[0][mf2], 
            &quaternion_time_data[mf2], IAS_LAGRANGE_PTS,
            (double)i * IAS_ANCILLARY_QUAT_TIME);
        quaternion_data[i].y = ias_math_interpolate_lagrange(&att_buff[1][mf2], 
            &quaternion_time_data[mf2], IAS_LAGRANGE_PTS,
            (double)i * IAS_ANCILLARY_QUAT_TIME);
        quaternion_data[i].z = ias_math_interpolate_lagrange(&att_buff[2][mf2], 
            &quaternion_time_data[mf2], IAS_LAGRANGE_PTS,
            (double)i * IAS_ANCILLARY_QUAT_TIME);

        state[i] = 1;

        for (k = mf2; k < mf2 + IAS_LAGRANGE_PTS; k++) 
        {
            if (valid_quaternion_flag[k] == 0) 
                state[i] = 0;
        }

#if DEBUG_GENERATE_DATA_FILES == 1
        fprintf(fp, "%d %f %e %e %e\n", i, i * IAS_ANCILLARY_QUAT_TIME, 
            quaternion_data[i].x, quaternion_data[i].y, quaternion_data[i].z);
#endif
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    fclose(fp);
#endif

    for (i = 0; i < quaternion_count; i++)
    {
        valid_quaternion_flag[i] = state[i]; 
    }

    /* Synchronize the IMU data.  Do the Lagrange interpolation in angle rather
       than rate space. */
    att_buff[0][0] = imu_data[i].x * IAS_ANCILLARY_IMU_TIME;
    att_buff[1][0] = imu_data[i].y * IAS_ANCILLARY_IMU_TIME;
    att_buff[2][0] = imu_data[i].z * IAS_ANCILLARY_IMU_TIME;
    for (i = 1; i < imu_count; i++)
    {
        double time_diff = imu_time_data[i] - imu_time_data[i - 1];

        att_buff[0][i] = att_buff[0][i - 1] + imu_data[i].x * time_diff;
        att_buff[1][i] = att_buff[1][i - 1] + imu_data[i].y * time_diff;
        att_buff[2][i] = att_buff[2][i - 1] + imu_data[i].z * time_diff;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    fp = fopen("kal.imu.dat", "w");
#endif

    for (i = 0; i < imu_count; i++)
    {
        mf = (int)floor(((double)i * IAS_ANCILLARY_IMU_TIME 
            - imu_time_data[0]) / IAS_ANCILLARY_IMU_TIME);

        /* Since the incoming samples can be irregularly spaced, make sure
           we have the correct one by checking the times */
        while ((mf > 0) 
               && (imu_time_data[mf] > (double)i * IAS_ANCILLARY_IMU_TIME))
        {
            mf--;
        }
        while ((mf < imu_count - 1)
               && (imu_time_data[mf + 1] < (double)i * IAS_ANCILLARY_IMU_TIME))
        {
            mf++;
        }

        if (mf < 0) 
            mf = IAS_LAGRANGE_PTS / 2;

        if (mf > imu_count) 
            mf = imu_count - 1;

        mf2 = mf - IAS_LAGRANGE_PTS / 2;

        if (mf2 < 0) 
            mf2 = 0;

        if (mf2 > imu_count - IAS_LAGRANGE_PTS) 
            mf2 = imu_count - IAS_LAGRANGE_PTS;

        imu_data[i].x = ias_math_interpolate_lagrange(&att_buff[0][mf2], 
            &imu_time_data[mf2], IAS_LAGRANGE_PTS, 
            (double)i * IAS_ANCILLARY_IMU_TIME);
        imu_data[i].y = ias_math_interpolate_lagrange(&att_buff[1][mf2], 
            &imu_time_data[mf2], IAS_LAGRANGE_PTS, 
            (double)i * IAS_ANCILLARY_IMU_TIME);
        imu_data[i].z = ias_math_interpolate_lagrange(&att_buff[2][mf2], 
            &imu_time_data[mf2], IAS_LAGRANGE_PTS, 
            (double)i * IAS_ANCILLARY_IMU_TIME);

        state[i] = 1;

        for (k = mf2; k < mf2 + IAS_LAGRANGE_PTS; k++)
        {
            if (valid_imu_flag[k] == 0) 
                state[i] = 0;
        }

#if DEBUG_GENERATE_DATA_FILES == 1
        fprintf(fp, "%d %f %e %e %e\n", i, i * IAS_ANCILLARY_IMU_TIME,
                    imu_data[i].x, imu_data[i].y, imu_data[i].z);
#endif
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    fclose(fp);
#endif

    /* Turn the IMU measurements back into rates for the filter */
    for (i = imu_count - 1; i >= 0; i--) 
    {
        if (i > 0)
        {
            imu_data[i].x = (imu_data[i].x - imu_data[i - 1].x)
                    / IAS_ANCILLARY_IMU_TIME;
            imu_data[i].y = (imu_data[i].y - imu_data[i - 1].y)
                    / IAS_ANCILLARY_IMU_TIME;
            imu_data[i].z = (imu_data[i].z - imu_data[i - 1].z)
                    / IAS_ANCILLARY_IMU_TIME;
        }
        else
        {
            imu_data[i].x /= IAS_ANCILLARY_IMU_TIME;
            imu_data[i].y /= IAS_ANCILLARY_IMU_TIME;
            imu_data[i].z /= IAS_ANCILLARY_IMU_TIME;
        }
        valid_imu_flag[i] = state[i];
        imu_time_data[i] = (double)i * IAS_ANCILLARY_IMU_TIME;
    }

    /* Set up state transition matrix. */
    S[0] = 1.0;    
    S[1] = IAS_ANCILLARY_IMU_TIME;    
    S[2] = 0.0;
    S[3] = 0.0;    
    S[4] = 1.0;       
    S[5] = 0.0;
    S[6] = 0.0;    
    S[7] = 0.0;       
    S[8] = 1.0;

    /* The following state noise matrix is for two white noise processes
       driving the network. */
    Q[0] = (IAS_ANCILLARY_IMU_TIME * IAS_ANCILLARY_IMU_TIME 
            * IAS_ANCILLARY_IMU_TIME * IAS_ANCILLARY_IMU_TIME *
            PNOISE_ATTRATE_SIGMA * PNOISE_ATTRATE_SIGMA) / 4.0;
    Q[1] = (IAS_ANCILLARY_IMU_TIME * IAS_ANCILLARY_IMU_TIME 
            * IAS_ANCILLARY_IMU_TIME *
            PNOISE_ATTRATE_SIGMA * PNOISE_ATTRATE_SIGMA) / 2.0;
    Q[2] = 0.0; 

    Q[3] = (IAS_ANCILLARY_IMU_TIME * IAS_ANCILLARY_IMU_TIME 
            * IAS_ANCILLARY_IMU_TIME *
            PNOISE_ATTRATE_SIGMA * PNOISE_ATTRATE_SIGMA) / 2.0;
    Q[4] = (IAS_ANCILLARY_IMU_TIME * IAS_ANCILLARY_IMU_TIME *
            PNOISE_ATTRATE_SIGMA * PNOISE_ATTRATE_SIGMA);
    Q[5] = 0.0; 

    Q[6] = 0.0;
    Q[7] = 0.0;
    Q[8] = (IAS_ANCILLARY_IMU_TIME * IAS_ANCILLARY_IMU_TIME *
            PNOISE_DRIFT_SIGMA * PNOISE_DRIFT_SIGMA);

    for (axis = 0; axis < NUM_AXIS ; axis++)
    {
        /* use this initialization when you have two noise processes
           driving the process */
        Pn[0] = SNOISE_ATT_SIGMA * a2r;
        Pn[1] = 0.0; 
        Pn[2] = 0.0;
        Pn[3] = 0.0;                  
        Pn[4] = SNOISE_ATTRATE_SIGMA * a2r; 
        Pn[5] = 0.0;
        Pn[6] = 0.0;                  
        Pn[7] = 0.0;                       
        Pn[8] = SNOISE_DRIFT_SIGMA * a2r;
        Pn[0] *= Pn[0]; /* square to convert standard deviation to variance */
        Pn[4] *= Pn[4]; /* square to convert standard deviation to variance */
        Pn[8] *= Pn[8]; /* square to convert standard deviation to variance */

        /* Check to see if Euler parameters are OK for first PCD value time 
           synced with IMU. */
        /* Initialize attitude state with the first valid quaternion
           measurement.  Loop throught the L0R report PCD structure,
           starting with the first value, to find the first "valid" value.
           Set initial state to this value */

        if (axis == 0)
        {
            Xk[0] = quaternion_data[0].x;  /* initialize to first EPA point */
            Xk[1] = imu_data[0].x; /* initialize to first IMU point */
        }

        if (axis == 1)
        {
            Xk[0] = quaternion_data[0].y;  /* initialize to first EPA point */
            Xk[1] = imu_data[0].y; /* initialize to first IMU point */
        }

        if (axis == 2)
        {
            Xk[0] = quaternion_data[0].z;  /* initialize to first EPA point */
            Xk[1] = imu_data[0].z; /* initialize to first IMU point */
        }

        Xk[2] = 0.0; /* drift value is initially zero */

        /* set up matrices for case where IMU, and quaternion are available

           the sign on H[5] is dependent on whether the sign on the drift
           has been changed or not
        */

        for (i=0; i < imu_count; i++)
        {
            quaternion_index = i / imu_quat_ratio;

            if (i % imu_quat_ratio == 0  && quaternion_index < quaternion_count)
            {
               n_size = 2;
               state[i] = 1;
    
               H[0] =  1.0;        
               H[1] =  0.0;    
               H[2] =  0.0;
               H[3] =  0.0;        
               H[4] =  1.0;    
               H[5] = -1.0;

               R[0] = ONOISE_EPA_SIGMA * a2r;

               if (valid_quaternion_flag[quaternion_index] == 0) 
                   R[0] *= 100.0;

               R[1] = 0.0;
               R[2] = 0.0;
               R[3] = ONOISE_GYRO_SIGMA * a2r;

               if (valid_imu_flag[i] == 0) 
                   R[3] *= 100.0;
    
               R[0] *= R[0]; /* square to convert std dev to variance */
               R[3] *= R[3]; /* square to convert std dev to variance */
            }
            else
            {
               n_size = 1;
               state[i] = 2;
    
               H[0] =  0.0;   
               H[1] =  1.0;   
               H[2] = -1.0;
    
               R[0] = ONOISE_GYRO_SIGMA * a2r;

               if (valid_imu_flag[i] == 0) 
                   R[0] *= 100.0;

               R[0] *= R[0]; /* square to convert std dev to variance */
            }

            if (axis == 0)
            {
                if (i % imu_quat_ratio == 0 && quaternion_index 
                    < quaternion_count)
                {
                   z[0] = quaternion_data[quaternion_index].x;
                   z[1] = imu_data[i].x;
                }
                else
                   z[0] = imu_data[i].x;
            }

            if (axis == 1)
            {
                if (i % imu_quat_ratio == 0 && quaternion_index 
                    < quaternion_count)
                {
                   z[0] = quaternion_data[quaternion_index].y;
                   z[1] = imu_data[i].y;
                }
                else
                   z[0] = imu_data[i].y;
            }

            if (axis == 2)
            {
                if (i % imu_quat_ratio == 0 && quaternion_index 
                    < quaternion_count)
                {
                   z[0] = quaternion_data[quaternion_index].z;
                   z[1] = imu_data[i].z;
                }
                else
                   z[0] = imu_data[i].z;
            }

            /* filter data */
            status = ias_math_compute_kalman_gain(Pn, H, R, K, M_SIZE, n_size);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Returned from ias_math_compute_kalman_gain");
                FREE_MEMORY();
                return ERROR;
            }
    
            status = ias_math_update_filter_state(
                         Xk, X[i].d, K, z, H, M_SIZE, n_size);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Returned from ias_math_update_filter_state");
                FREE_MEMORY();
                return ERROR;
            }

            status = ias_math_update_filter_error_covar(
                         K, H, Pn, P[i].d, M_SIZE, n_size);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Returned from "
                              "ias_math_update_filter_error_covar");
                FREE_MEMORY();
                return ERROR;
            }

            /* predict error covariance and state matrix for next iteration */
            status = ias_math_compute_predicted_error_covar(
                         S, P[i].d, Pn1[i].d, Q, M_SIZE);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Returned from "
                              "ias_math_compute_predict_error_covar");
                FREE_MEMORY();
                return ERROR;
            }

            status = ias_math_predict_state(S, X[i].d, Xk1[i].d, M_SIZE);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Returned from ias_math_predict_state");
                FREE_MEMORY();
                return ERROR;
            }
    
            status = ias_math_multiply_matrix(
                         H, Xk1[i].d, pre, n_size, M_SIZE, M_SIZE, 1);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Returned from ias_math_multiply_matrix");
                FREE_MEMORY();
                return ERROR;
            }

            for (j = 0; j < M_SIZE; j++)
            {
                Xk[j] = Xk1[i].d[j];
            }

            for (j = 0; j < M_SIZE * M_SIZE ; j++)
            {
                Pn[j] = Pn1[i].d[j];
            }
        }

        /* NOTE:
           if refering to the book "Introduction to Random Signal Analysis
           and Kalman filtering" the predicted covariance matrix at k+1
           is stored in Pn1[i] in the code while the updated covariance
           matrix at k is stored in P[i]

           the same holds for the state matrix, predicted state at k+1 is
           stored in Xk1[i] the updated estimate at k in X[i]
        */
        imu_ptr = imu_buffer;
        for (j = 0; j < M_SIZE; j++)
        {
            XN[j] = Xk1[imu_count - 1].d[j];
        }

        for (i = imu_count - 1, k = 0;
             i >= 0;
             i--, k++) 
        {
            /* smooth data */

            /* if the matrix multiply associated with the H matrix below is
               not used (it shouldn't be needed for the application, it was
               just put in as a check for mw) then following code doesn't
               need to be updated */
            if (state[i] == 1)
               n_size = 2;
            else
               n_size = 1;
    
            status = ias_math_smooth_gain(
                         P[i].d, Pn1[i].d, S, A, M_SIZE);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Returned from ias_math_smooth_gain");
                FREE_MEMORY();
                return ERROR;
            }

            status = ias_math_smooth_state(
                         X[i].d, Xk1[i].d, XN, A, XN1, M_SIZE);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Returned from ias_math_smooth_state");
                FREE_MEMORY();
                return ERROR;
            }

            *imu_ptr = XN1[0];
            imu_ptr++;

            for (j = 0; j < M_SIZE; j++)
            {
                XN[j] = XN1[j];
            }
        }

        /* reverse the order of the smoothed data set.

           the smoothing algorithm starts with the last data sample and
           works forward
        */
        for (i = imu_count - 1, k = 0;
             i >= 0;
             i--, k++)
        {
            if (axis == 0)
                imu_data[k].x = imu_buffer[i];
            else if (axis == 1)
                imu_data[k].y = imu_buffer[i];
            else
                imu_data[k].z = imu_buffer[i];
        }
    }

    /* free local storage */
    FREE_MEMORY();

    return SUCCESS;
}
