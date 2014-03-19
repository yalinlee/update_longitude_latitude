/******************************************************************************
NAME: ias_sensor_align_ssm_data

PURPOSE: Aligns the high and low 16-bit words of the TIRS Scene Select Mirror
    encoder samples. The input record has 21 samples, the output is trimmed
    to 20 samples.

RETURNS:
    SUCCESS or ERROR
******************************************************************************/
#include <stdio.h>
#include <math.h>
#include "../logging_channel.h" /* define the debug logging channel */
#include "ias_logging.h"
#include "ias_l0r.h"
#include "ias_sensor_model.h"
#include "local_defines.h"

#define LOW     0x0000ffff
#define HIGH    0x00ff0000
#define NSAMP   IAS_L0R_TIRS_TELEMETRY_SSM_ENCODER_POSITION_SAMPLE_COUNT
#define S21     (NSAMP - 1)
#define S20     (NSAMP - 2)

/* Local function prototypes */
static unsigned int shift_high
(
    IAS_L0R_TIRS_SSM_ENCODER *l0r_tirs_ssm  /* I: TIRS anc data from L0R */
);
static unsigned int shift_low
(
    IAS_L0R_TIRS_SSM_ENCODER *l0r_tirs_ssm  /* I: TIRS anc data from L0R */
);

/* Main routine */
void ias_sensor_align_ssm_data
(
    IAS_L0R_TIRS_SSM_ENCODER *l0r_tirs_ssm, /* I/O: SSM telemetry records */
    int count                               /* I: Number SSM records in L0R */
)
{
    unsigned int encoder;       /* Temporary encoder value */
    int field;                  /* Telemetry record encoder samples counter */

    /* Analyze each TIRS ancillary data record */
    for (field = 0; field < count; field++)
    {
        unsigned int *cur_encoder_samps;    /* Pointer to current SSM record's
                                               encoder samples array */
        unsigned int *next_encoder_samps;   /* Pointer to next SSM record's
                                               encoder samples array */

        /* Cache a pointer to the encoder samples arrays for the current and
           the next SSM records */
        cur_encoder_samps = &l0r_tirs_ssm[field].encoder_position[0];
        next_encoder_samps = &l0r_tirs_ssm[field + 1].encoder_position[0];

        /* First, check for the hot case - slow running SSM sampling clock.
           See if the 20th and 21st samples are zero. */
        if ((cur_encoder_samps[S20] == 0) && (cur_encoder_samps[S21] == 0))
        {
            /* If this is not the last ancillary record, move the first sample
               of the next record into this record's 20th slot, otherwise set
               sample 20 equal to sample 19 */
            if (field < (count - 1))
                cur_encoder_samps[S20] = next_encoder_samps[0];
            else
                cur_encoder_samps[S20] = cur_encoder_samps[S20 - 1];
        }

        /* See if the high 16-bits of the 20th sample are zeros */
        if ((cur_encoder_samps[S20] & HIGH) == 0)
        {
            /* If there is data in the 21st slot, move the bits from sample
               21 to sample 20 */
            if ((cur_encoder_samps[S21] & HIGH) > 0)
            {
                cur_encoder_samps[S20] |= (cur_encoder_samps[S21] & HIGH);
                cur_encoder_samps[S21] &= LOW;
            }
            else
            {
                /* Otherwise, move the high 16-bits from the first sample in
                   the next record to sample 20, and move all the high order
                   words in the next record up one sample (setting the 21st
                   sample to zero). If the current record is the last record,
                   copy sample 19 into sample 20. */
                if (field < (count - 1))
                {
                    encoder = shift_high(&l0r_tirs_ssm[field + 1]);
                    cur_encoder_samps[S20] |= encoder;
                }
                else
                    cur_encoder_samps[S20] = cur_encoder_samps[S20 - 1];
            }
        }

        /* See if the low 16-bits of the 20th sample are zeros */
        if ((cur_encoder_samps[S20] & LOW) == 0)
        {
            /* If there is data in the 21st slot, move the bits from sample
               21 to sample 20 */
            if ((cur_encoder_samps[S21] & LOW) > 0)
            {
                cur_encoder_samps[S20] |= (cur_encoder_samps[S21] & LOW);
                cur_encoder_samps[S21] &= HIGH;
            }
            else
            {
                /* Otherwise, move the low 16-bits from the first sample in the
                   next record to sample 20, and move all the low order words
                   in the next record up one sample. If the current record is
                   the last record, copy sample 19 into sample 20. */
                if (field < (count - 1))
                {
                    encoder = shift_low(&l0r_tirs_ssm[field + 1]);
                    cur_encoder_samps[S20] |= encoder;
                }
                else
                    cur_encoder_samps[S20] = cur_encoder_samps[S20 - 1];
            }
        }

        /* Now, check for the hot case - fast running SSM sampling clock.
           Repair high/low misalignment, discarding extra samples. */
        /* If the 21st slot is not zero, adjust the next record, if needed */
        if (cur_encoder_samps[S21] != 0)
        {
            /* See if we have just the low word */
            if ((cur_encoder_samps[S21] & HIGH) == 0)
            {
                /* If this is not the last record, move all the high order
                   words in the next record up one sample */
                if (field < (count - 1))
                    encoder = shift_high(&l0r_tirs_ssm[field + 1]);
            }

            /* See if we have just the high word */
            if ((cur_encoder_samps[S21] & LOW) == 0)
            {
                /* If this is not the last record, move all the low order words
                   in the next record up one sample */
                if (field < (count - 1))
                    encoder = shift_low(&l0r_tirs_ssm[field + 1]);
            }
        }
    }

    /* Trim the SSM encoder samples down to 20 samples */
    for (field = 0; field < count; field++)
        l0r_tirs_ssm[field].encoder_position[S21] = 0;
}

/******************************************************************************
NAME: shift_high

PURPOSE: Move all the high order words in the record up one sample (setting
    the 21st sample to zero).

RETURNS:
    The high 16-bits of the first sample
******************************************************************************/
static unsigned int shift_high
(
    IAS_L0R_TIRS_SSM_ENCODER *l0r_tirs_ssm  /* I: TIRS anc data from L0R */
)
{
    unsigned int encoder;           /* High word from first encoder sample */
    unsigned int *encoder_samps;    /* Pointer to SSM record encoder samples */
    int i;                          /* Loop index */

    /* Cache a pointer to the encoder samples array */
    encoder_samps = &l0r_tirs_ssm->encoder_position[0];

    /* Get the high word from the first encoder sample */
    encoder = encoder_samps[0] & HIGH;

    /* Loop through the encoder samples, shifting the high words up */
    for (i = 0; i < S21; i++)
    {
        encoder_samps[i] &= LOW;
        encoder_samps[i] |= (encoder_samps[i + 1] & HIGH);
    }
    encoder_samps[S21] &= LOW;

    return encoder;
}

/******************************************************************************
NAME: shift_low

PURPOSE: Move all the low order words in the record up one sample.

RETURNS:
    The low 16-bits of the first sample
******************************************************************************/
static unsigned int shift_low
(
    IAS_L0R_TIRS_SSM_ENCODER *l0r_tirs_ssm  /* I: TIRS anc data from L0R */
)
{
    unsigned int encoder;           /* Low word from first encoder sample */
    unsigned int *encoder_samps;    /* Pointer to SSM record encoder samples */
    int i;                          /* Loop index */

    /* Cache a pointer to the encoder samples array */
    encoder_samps = &l0r_tirs_ssm->encoder_position[0];

    /* Get the low word from the first encoder sample */
    encoder = encoder_samps[0] & LOW;

    /* Loop through the encoder samples, shifting the low words up */
    for (i = 0; i < S21; i++)
    {
        encoder_samps[i] &= HIGH;
        encoder_samps[i] |= (encoder_samps[i + 1] & LOW);
    }
    encoder_samps[S21] &= HIGH;

    return encoder;
}
