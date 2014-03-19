#ifndef LOCAL_DEFINES_H
#define LOCAL_DEFINES_H

/* Maximum detector offset or delay. In general use, the maximum_detector_delay
   field of the sampling characteristics should be used since it is initialized
   to this value. */
#define MAX_DET_DELAY 10.0

/* Scale of PAN to MS in resolution */
#define PAN_SCALE 2.0

/* Lines per image frame */
#define FRAME_LINES_MS 1
#define FRAME_LINES_PAN 2

/* Define a mask for the bits that are used in the SSM encoder position
   for TIRS */
#define TIRS_SSM_BIT_MASK 0x00ffffff

/* Define the point where the SSM encoder is considered to have overflowed */
#define TIRS_SSM_OVERFLOW 0x01000000

/* Define the point where TIRS has entered signed overflow */
#define TIRS_SSM_LIMIT (TIRS_SSM_OVERFLOW/2)

#endif
