/*************************************************************************
 NAME:                       ias_bpf.h

 PURPOSE: Header file defining BPF File I/O constants and BPF data structure
          members.  The header includes a forward declaration to the
          actual BPF data structure (hiding the structure from the user),
          as well as prototypes for functions outside applications can use
          to access the various BPF data members
**************************************************************************/
#ifndef IAS_BPF_H
#define IAS_BPF_H

#include "ias_l0r_constants.h"
#include "ias_satellite_attributes.h"

/***********************************************************************
  The defines located in this header file are BPF specific and SHALL NOT
  be used elsewhere.  
***********************************************************************/
#define IAS_BPF_MS_MAX_NDET 494
#define IAS_BPF_PAN_MAX_NDET 988
#define IAS_BPF_THERMAL_MAX_NDET 640
#define IAS_BPF_PAN_ODD  1
#define IAS_BPF_PAN_EVEN 0
#define IAS_BPF_VNIR_NBANDS 5
#define IAS_BPF_SWIR_NBANDS 3
#define IAS_BPF_PAN_NBANDS 1
#define IAS_BPF_THERMAL_MAX_NSCAS  3
#define IAS_BPF_OLI_NBANDS \
    (IAS_BPF_VNIR_NBANDS + IAS_BPF_SWIR_NBANDS + IAS_BPF_PAN_NBANDS)
#define IAS_BPF_TIRS_NBANDS 2
#define IAS_BPF_NBANDS (IAS_BPF_OLI_NBANDS + IAS_BPF_TIRS_NBANDS)

/******************** BUG TRACKER #1950 ********************************/

/* Maximum number of possible values in each detector-specific model parameter.
   It should be the maximum of the band type-specific values #defined below */
#define IAS_BPF_MAX_MODEL_PARAM_VALUES 4 

/* Number of detector-specific model parameter values.  Right now, it's 4 for
   all OLI bands (i.e. pre-acquisition response, post-acquisition response,
   slope a1, and intercept C1), and 2 for all TIRS bands (i.e.
   pre-acquisition response, post-acquisition response)  */
#define IAS_BPF_VNIR_VALUES 4
#define IAS_BPF_SWIR_VALUES 4
#define IAS_BPF_PAN_VALUES  4
#define IAS_BPF_THERMAL_VALUES 2

/* Number of values in the A0 coefficient */
#define IAS_BPF_A0_VALUES  1 

typedef enum ias_bpf_detector_model_parms
{
    IAS_BPF_DETECTOR_PRE_AVG,
    IAS_BPF_DETECTOR_POST_AVG,
    IAS_BPF_DETECTOR_A1,
    IAS_BPF_DETECTOR_C1
}IAS_BPF_DETECTOR_MODEL_PARMS;

/******************** END BUG TRACKER #1950 ****************************/

/* FILE_ATTRIBUTES parameter lengths (includes NULL terminator).  Since
   these parameters are the same as the corresponding parameters in
   the CPF, the sizes should be the same */
#define IAS_BPF_FILE_NAME_LENGTH    201   /* BPF file name length */
#define IAS_BPF_SPACECRAFT_NAME_LENGTH 21 /* Same as hard-coded length for
                                             CPF spacecraft_name length */
#define IAS_BPF_SENSOR_NAME_LENGTH  31    /* Same as hard-coded length for
                                             CPF sensor_name field */
#define IAS_BPF_DESCRIPTION_LENGTH 4001   /* Same as hard-coded length for
                                             cpf_description field */


/***********************************************************************
  The structures that make up the overall BPF structure
***********************************************************************/

/* dates are expected to be of the form: YYYY-MM-DDTHH:MM:SS.SSSSS */
/*                                  e.g. 2010-01-01T00:00:00.00000 */

struct IAS_BPF_FILE_ATTRIBUTES
{
    char effective_date_begin[IAS_L0R_DATE_LENGTH];
    char effective_date_end[IAS_L0R_DATE_LENGTH];
    char baseline_date[IAS_L0R_DATE_LENGTH];
    char file_name[IAS_BPF_FILE_NAME_LENGTH];
    char file_source[IAS_BPF_FILE_NAME_LENGTH];
    char spacecraft_name[IAS_BPF_SPACECRAFT_NAME_LENGTH];
    char sensor_name[IAS_BPF_SENSOR_NAME_LENGTH];
    char description[IAS_BPF_DESCRIPTION_LENGTH];
    int  version;
};


struct IAS_BPF_ORBIT_PARAMETERS
{
    int begin_orbit_number;                    /* Current OLI beginning
                                                  orbit number */
};


struct IAS_BPF_VNIR_BIAS_MODEL
{
    double det_params[IAS_MAX_NSCAS][IAS_BPF_MS_MAX_NDET]
                     [IAS_BPF_VNIR_VALUES];
    double a0_coefficient[IAS_MAX_NSCAS];
};

struct IAS_BPF_SWIR_BIAS_MODEL
{
    double det_params[IAS_MAX_NSCAS][IAS_BPF_MS_MAX_NDET]
                     [IAS_BPF_SWIR_VALUES];
    double a0_coefficient[IAS_MAX_NSCAS];
};

struct IAS_BPF_PAN_BIAS_MODEL
{
    double det_params_even[IAS_MAX_NSCAS][IAS_BPF_PAN_MAX_NDET]
                          [IAS_BPF_PAN_VALUES];
    double a0_coefficient_even[IAS_MAX_NSCAS];

    double det_params_odd[IAS_MAX_NSCAS][IAS_BPF_PAN_MAX_NDET]
                         [IAS_BPF_PAN_VALUES];
    double a0_coefficient_odd[IAS_MAX_NSCAS];
};

struct IAS_BPF_THERMAL_BIAS_MODEL
{
    double det_params[IAS_BPF_THERMAL_MAX_NSCAS][IAS_BPF_THERMAL_MAX_NDET]
                     [IAS_BPF_THERMAL_VALUES];
};

struct IAS_BPF_BIAS_MODEL
{
    int band_number;                      /* Current band number */
    IAS_SPECTRAL_TYPE spectral_type;      /* OLI / TIRS spectral type
                                             (VNIR/SWIR/PAN/THERMAL) */
    struct IAS_BPF_PAN_BIAS_MODEL *pan;   /* Pointer to PAN band bias model
                                             data block */
    struct IAS_BPF_VNIR_BIAS_MODEL *vnir; /* Pointer to VNIR band bias model
                                             data block */
    struct IAS_BPF_SWIR_BIAS_MODEL *swir; /* Pointer to SWIR band bias model
                                             data block */
    struct IAS_BPF_THERMAL_BIAS_MODEL *thermal;
                                          /* Pointer to TIRS band bias model
                                             data block */
};



/* A forward declaration of the IAS_BPF data structure */
typedef struct IAS_BPF IAS_BPF;


                 /* Getter function prototypes */
const struct IAS_BPF_ORBIT_PARAMETERS *ias_bpf_get_orbit_parameters
(
    IAS_BPF *bpf
);


const struct IAS_BPF_FILE_ATTRIBUTES *ias_bpf_get_file_attributes
(
    IAS_BPF *bpf
);


const struct IAS_BPF_BIAS_MODEL *ias_bpf_get_bias_model
(
    IAS_BPF *bpf                          /* I/O: BPF data structure */
);


/* Setter function prototypes.  These are to be called when a
   single parameter or set of parameters in an in-memory BPF data block
   (whether "empty" or populated through reading the BPF file) needs to
   be set/updated*/
void ias_bpf_set_file_attributes_file_name
(
    IAS_BPF *bpf,                  /* I/O: BPF data structure to update */
    const char *bpf_file_name      /* I: Name of BPF file to update */
);

void ias_bpf_set_file_attributes_status
(
    IAS_BPF *bpf,                  /* I/O: BPF data structure to update */
    const char *bpf_status         /* I: BPF file status to update */
);


void ias_bpf_set_file_attributes_file_source
(
    IAS_BPF *bpf,                  /* I/O: BPF data structure to update */
    const char *bpf_file_source    /* I: BPF source file name to update */
);


void ias_bpf_set_file_attributes_effective_date_begin
(
    IAS_BPF *bpf,                  /* I/O: BPF data structure to update */
    const char *effective_date_begin   /* I: Beginning effective date */
);


void ias_bpf_set_file_attributes_effective_date_end
(
    IAS_BPF *bpf,                   /* I/O: BPF data structure to update */
    const char *effective_date_end  /* I: Ending effective date */
);


void ias_bpf_set_file_attributes_spacecraft_name
(
    IAS_BPF *bpf,                   /* I/O: BPF data structure to update */
    const char *bpf_spacecraft_name /* I: Spacecraft Name */
);


void ias_bpf_set_file_attributes_sensor_name
(
    IAS_BPF *bpf,                   /* I/O: BPF data structure to update */
    const char *bpf_sensor_name     /* I: Sensor name */
);


void ias_bpf_set_file_attributes_baseline_date
(
    IAS_BPF *bpf,                   /* I/O: BPF data structure to update */
    const char *bpf_baseline_date   /* I: Name of BPF file to update */
);


void ias_bpf_set_file_attributes_description
(
    IAS_BPF *bpf,                   /* I/O: BPF data structure to update */
    const char *bpf_description     /* I: Name of BPF file to update */
);


void ias_bpf_set_file_attributes_version
(
    IAS_BPF *bpf,                   /* I/O: BPF data structure to update */
    int version                     /* I: Name of BPF file to update */
);

void ias_bpf_set_orbit_parameters_begin_orbit_number
(
    IAS_BPF *bpf,                   /* I/O: BPF data structure to update */
    int begin_orbit_number          /* I: Name of BPF file to update */
);


int ias_bpf_set_bias_model_band_number
(
    IAS_BPF *bpf,                   /* I/O: BPF data structure to update */
    int band_number                 /* I: Band number */
);


/* NOTE:  This function also allocates the proper model data block (i.e.
   *vnir, *pan, *swir, *thermal) depending on the input band's spectral type */
int ias_bpf_set_bias_model_spectral_type
(
    IAS_BPF *bpf,                   /* I/O: BPF data structure to update */
    int band_number,                /* I: Band number */
    IAS_SPECTRAL_TYPE spectral_type /* I: Spectral type for the current
                                          band */
);


int ias_bpf_set_bias_model_a0_coefficient
(
    IAS_BPF *bpf,                 /* I/O:  BPF data structure to be updated */
    int band_number,              /* I: 1-based band number */
    int sca_index,                /* I: 0-based SCA index */
    int oddeven_flag,             /* I: PAN band state flag */
    double a0_coeff               /* I: A0 model coefficient value */
);

int ias_bpf_set_bias_model_detector_coefficients
(
    IAS_BPF *bpf,                 /* I/O:  BPF data structure to be updated */
    int band_number,              /* I: 1-based band number */
    int sca_index,                /* I: 0-based SCA index */
    int det_index,                /* I: 0-based detector index */
    int oddeven_flag,             /* I: PAN band state flag */
    double *params,               /* I: Array of detector-specific
                                     parameters to set */
    int num_params                /* I: Number of parameters to set */
);


IAS_SENSOR_ID ias_bpf_get_sensor_id
(
    IAS_BPF *bpf                      /* I: BPF data structure */
);

                 
                 /* Miscellaneous function prototypes */

/* NOTE:  This allocation should be used when a BPF structure is to
   be populated manually, rather than through reading a file. */
IAS_BPF *ias_bpf_allocate();


void ias_bpf_free
(
    IAS_BPF *bpf              /* I: BPF data block to free */
);


IAS_BPF *ias_bpf_read
(
    const char *filename      /* I: name of the BPF file to read */
);


int ias_bpf_get_model_parameters
(
    const struct IAS_BPF_BIAS_MODEL *bias_model,
                               /* I: Fully populated band model data
                                  structure */
    IAS_SPECTRAL_TYPE spectral_type,
                               /* I: Expected spectral type for the
                                  specified band */
    int band_number,           /* I: 1-based band number */
    int sca_number,            /* I: 1-based SCA number */
    int num_detectors,         /* I: Total number of detectors in the
                                  current band/SCA */
    int even_odd,              /* I: Even/Odd indicator (used only for
                                  PAN band) */
    double *pre_acquisition_average,
                               /* O: Array of pre-acquisition average
                                  responses in the current band/SCA. */
    double *post_acquisition_average,
                               /* O: Array of post-acquisition average
                                  responses in the current band/SCA. */
    double *a1_coefficient,    /* O: Array of OLI a1_coefficient for
                                  all detectors in the current band/SCA.
                                  Pass in NULL if reading a TIRS BPF.  */
    double *c1_coefficient,    /* O: Array of OLI c1_coefficient for
                                  all detectors in the current band/SCA.
                                  Pass in NULL if reading a TIRS BPF.  */
    double *a0_coefficient     /* O: The OLI VRP model coefficient for the
                                  current band/SCA.  Pass in NULL if reading
                                  a TIRS BPF.  */
);

int ias_bpf_write
(
    IAS_BPF * bpf,  /* I: BPF structure to write to a file */
    const char * filePath /* I: Path to an ODL file to be written */
);


#endif
