/*-----------------------------------------------------------------------------

NAME: ias_l0r_structures.h

PURPOSE: Defines the structures used in the L0R Library

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/

#ifndef __IAS_L0R_STRUCTURES_H__
#define __IAS_L0R_STRUCTURES_H__

#include <stdint.h>

#include "ias_const.h"
#include "ias_structures.h"

/******************************************************************************
* FrameCorrection
*    This enum holds the bit flags used to indicate the status of a Frame
*    Header.
*    A Verified value indicates that the frame header appears to be correct.
*    A Warning flag indicates that the data in this frame header is suspect for
*      for error.
*    All other values indicate that this header data has had that correction
*      performed.
* TIRS CRC Status Flags:
*    For TIRS Frame Headers, the IAS_L0R_CRC_STATUS is set whether the Read-Out
*    Integrated Circuit (ROIC) CRC flags got set or the CRC-12 check performed
*    by Ingest failed. The ROIC CRC check indicates a glitch detected by the
*    TIRS FPE ASICs before the overall CRC-12 is even summed, which means the
*    ROIC CRC status won't cause a CRC-12 failure.  So, to distinguish between
*    the two failures, the IAS_L0R_TIRS_CRC_STATUS bit can be checked to see if
*    the overall CRC-12 failed (which can be anywhere in the frame) or if one of
*    the Band/SCA/Row ROIC checks failed.  For example, if performing a
*    redundant row replacement and the Overall CRC status shows a failure,
*    but the CRC-12 status looks good, the ROIC CRC flags can be checked.
*    If one of the rows for a  particular Band & SCA
*    shows a failure in the ROIC status, it may not be a
*    good idea to do the swapping.  The ROIC CRC status flags are kept in the
*    TIRS Frame Header.
******************************************************************************/
typedef enum
{
  IAS_L0R_FRAMENUMBER_CORRECTED = 0x0001,
  IAS_L0R_TIMECODE_CORRECTED = 0x0002,
  IAS_L0R_FILLED = 0x0004,
  IAS_L0R_DUPLICATE = 0x0008,
  IAS_L0R_WARNING = 0x0010,
  IAS_L0R_VERIFIED = 0x0020,          /* When this bit is set, it indicates
                                         the header has been verified. No
                                         corrections or duplication of the
                                         header took place. */
  IAS_L0R_CRC_STATUS = 0x0040,        /* Overall CRC status for frame. If this
                                         bit is set, the CRC passed */
  IAS_L0R_TIRS_CRC12_STATUS = 0x0080, /* status for TIRS CRC-12 check. If this
                                         bit is set, the CRC passed  */
  IAS_L0R_FRAME_OUTSIDE_ANCILLARY = 0x1000 /* image frame not adequately 
                                           covered by ancillary data.  NOTE: 
                                           These frames will be removed during 
                                           Ingest processing so they will never
                                           be present in the final L0R product*/
}IAS_L0R_FRAME_STATUS;

/******************************************************************************
* IAS_L0R_TIME
*   The times present in the Mission Data are in various formats.  This
*   structure allows for a shared common time format.  This structure provides
*   more precision and range than is needed by any of the time formats which
*   get converted to this format.  This format uses J2000 as the epoch.
*   J2000 is described as: The Julian date 2451545.0 TT (Terrestrial Time),
*   or January 1, 2000, noon TT. This is equivalent to January 1, 2000,
*   11:59:27.816 TAI (International Atomic Time) or
*   January 1, 2000, 11:58:55.816 UTC (Coordinated Universal Time).
*
******************************************************************************/
typedef struct IAS_L0R_TIME
{
    int32_t days_from_J2000; /* count of days since J2000 TAI */
    double seconds_of_day; /* seconds in the day count */
}IAS_L0R_TIME;

typedef struct IAS_L0R_CORNERS
{
  IAS_DBL_LAT_LONG upleft;    /* Value of the upper left corner  */
  IAS_DBL_LAT_LONG upright;   /* Value of the upper right corner */
  IAS_DBL_LAT_LONG loleft;    /* Value of the lower left corner  */
  IAS_DBL_LAT_LONG loright;   /* Value of the lower right corner */
} IAS_L0R_CORNERS;

/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

    TODO - Add detailed descriptions for each structure and its members
           when more stable and detailed information is available from the
           Spacecraft vendor.  The purpose of each structure should be
           explained.  The units and possible values for each member of
           the structures should be added.

&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/

/*******************************************************************************
*ATTITUDE data structures
*******************************************************************************/
#define IAS_L0R_NUM_GYROS 4

typedef struct IAS_L0R_ATTITUDE
{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    double time_tag_sec_orig;
    IAS_QUATERNION inertial_to_body;/* Inertial to body quaternion */
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_ATTITUDE;

typedef struct IAS_L0R_ATTITUDE_FILTER_STATES
{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    int32_t time_tag_sec_orig;
    int32_t time_tag_subseconds_orig; /* tenths of microseconds */
    IAS_VECTOR gyro_combined_bias_rad_sec; /* Gyro combined bias */
    IAS_VECTOR gyro_scale_factor;
    /* scale factor corrections for each gyro */
    float gyro_x_misalignment_along_y_rad; /* radians */
    float gyro_x_misalignment_along_z_rad; /* radians */
    float gyro_y_misalignment_along_x_rad; /* radians */
    float gyro_y_misalignment_along_z_rad; /* radians */
    float gyro_z_misalignment_along_x_rad; /* radians */
    float gyro_z_misalignment_along_y_rad; /* radians */
    IAS_VECTOR kalman_filter_error_rad;  /* radians */
    IAS_VECTOR covariance_diagonal;
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_ATTITUDE_FILTER_STATES;

/*******************************************************************************
*BAND data structures
*******************************************************************************/
typedef struct IAS_L0R_BAND_ATTRIBUTES
{
    int band_number;       /* The band number these attributes apply to */
    int detectors_per_sca; /* Detectors per sca */
    int detectors_per_line;/* Total number of detectors in an image line */
    int number_of_lines;   /* Number of lines in the image */
    int sca_count;         /* Number of SCAs in the image */
}IAS_L0R_BAND_ATTRIBUTES;

/*******************************************************************************
*EPHEMERIS data structures
*******************************************************************************/
typedef struct IAS_L0R_EPHEMERIS
{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    double time_tag_sec_orig;
    IAS_VECTOR ecef_position_meters; /* spacecraft position in ecef
                                         coordinates */
    IAS_VECTOR ecef_velocity_meters_per_sec; /* spacecraft velocity in ecef */
    IAS_VECTOR orbit_determination_position_error_meters;
        /* spacecraft pos residual error */
    IAS_VECTOR orbit_determination_velocity_error_meters_per_sec;
        /* spacecraft velocity residual error */
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_EPHEMERIS;

/*******************************************************************************
*OLI Payload data structures
*******************************************************************************/

#define IAS_L0R_OLI_TELEMETRY_BENCH_TEMPS 7
#define IAS_L0R_OLI_TELEMETRY_FOCUS_MOTORS 3
#define IAS_L0R_OLI_TELEMETRY_GROUP_3_RESERVED 16
#define IAS_L0R_OLI_TELEMETRY_GROUP_3_SPARE 38
#define IAS_L0R_OLI_TELEMETRY_GROUP_4_RESERVED 6
#define IAS_L0R_OLI_TELEMETRY_GROUP_4_SPARE 12
#define IAS_L0R_OLI_TELEMETRY_GROUP_5_RESERVED 3
#define IAS_L0R_OLI_TELEMETRY_GROUP_5_SPARE 4

typedef struct IAS_L0R_OLI_TELEMETRY_GROUP_3{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    int16_t days_orig;
    int32_t milliseconds_orig;
    int16_t microseconds_orig;
    uint16_t sync_word;
    uint16_t id;
    float stim_lamp_output_current_amps;
    float stim_lamp_bulb_a_volts;
    float stim_lamp_bulb_b_volts;
    float stim_lamp_thermistor1_celsius;
    float stim_lamp_thermistor2_celsius;
    float stim_lamp_photodiode1_micro_amps;
    float stim_lamp_photodiode2_micro_amps;
    float focus_motor_lvdt[IAS_L0R_OLI_TELEMETRY_FOCUS_MOTORS];
    float pos_z_minus_y_temp_celsius;
    /* Positive Z/Minus Y Fitting Temperature (baseplate) */
    float bench_temps_celsius[IAS_L0R_OLI_TELEMETRY_BENCH_TEMPS];
    float fpm_7_temp_celsius; /* Focal Plane Module 7 Interface Temperature */
    float calibration_assembly_a_temp_celsius;
    float pos_z_pos_y_temp_celsius;
    /* Positive Z/Positive Y Fitting Temperature (baseplate) */
    float tertiary_mirror_temp_celsius;
    float fp_chassis_temp_celsius;
    float pos_y_temp_celsius;/* Positive Y Fitting Temperature (baseplate) */
    float fp_evap_temp_celsius; /* Focal Plane Assembly HP Evap Temperature */
    float fp_window_temp_celsius; /* Focal Plane Assembly Window Temperature */
    float minus_z_pos_y_temp_celsius;
    /* Minus Z/Minus Y Fitting Temperature (baseplate) */
    float minus_z_minus_y_temp_celsius;
    /* Minus Z/Minus Y Fitting Temperature (baseplate) */
    float minus_y_temp_celsius; /* Minus Y Fitting Temperature (baseplate) */
    float fpm_14_temp_celsius; /* Focal Plane Module 14 Interface Temperature */
    float lvps_temp_celsius;
    uint8_t reserved[IAS_L0R_OLI_TELEMETRY_GROUP_3_RESERVED];
    uint8_t spare[IAS_L0R_OLI_TELEMETRY_GROUP_3_SPARE];
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_OLI_TELEMETRY_GROUP_3;

typedef struct IAS_L0R_OLI_TELEMETRY_GROUP_4{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    int16_t days_orig;
    int32_t milliseconds_orig;
    int16_t microseconds_orig;
    uint16_t sync_word;
    uint16_t id;
    uint8_t mech_command_reject_count;
        /*Mechanism Control Command Rejected Count*/
    uint8_t mech_command_accept_count;
        /*Mechanism Control Command Accept Counter */
    uint8_t shutter_active;
        /* 0=not moving, 1=moving open loop 2=moving closed*/
    uint8_t last_command_opcode; /*Opcode of the last accepted command */
    uint8_t diffuser_active; /*0=not moving,1=moving open loop,2=moving closed*/
    uint8_t shutter_commanded_moves; /* When a shutter mechanism closed-loop
     move is active, this telemetry point reports how many times the shutter
     motor has been commanded to move.  This telemetry point is reset to zero
     every time that a new closed-loop move is started */
    uint8_t focus_motor_flags;
    /*bit 7 indicates the focus relay 2 (Motor Drive Relay) is being pulsed on*/
    /*bit 6 indicates that focus relay 2 is being pulsed off*/
    /*bit 5 indicates whether or not the LVDT will stay on during a move*/
    /*bit 4 Focus motor move status */
    /*bit 3 Direction of focus motor move */
    /*bit 2 Indicates if focus motor 3 will be included in the next move*/
    /*bit 1 Indicates if focus motor 2 will be included in the next move*/
    /*bit 0 Indicates if focus motor 1 will be included in the next move*/
    uint8_t diffuser_commanded_moves; /* When a diffuser mechanism closed-loop
    move is active, this telemetry point reports how many times the diffuser
    motor has been commanded to move.  This telemetry point is reset to zero
    every time that a new closed-loop move is started */
    double focus_motor_pulse_time_step_sec; /*Time between focus motor pulses */
    double focus_motor_pulse_length_sec; /*Length of focus motor pulses*/
    uint16_t focus_motor_pulses; /*The number of steps that will be used for a
        focus motor move*/
    uint8_t focus_mechanism_lvdt_relay_status;
    uint8_t focus_mechanism_motor_relay_status;
    double shutter_motor_pulse_length_sec; /*Length of shutter motor pulses*/
    uint8_t shutter_status_flags;
        /*bit 7-4 shutter_status Reserved for future use */
        /*bit 3 indicates the shutter/diffuser relay is to open inactive side*/
        /*bit 2 indicates the shutter/diffuser relay is to close active side*/
        /*bit 1 shutter motor move status */
        /*bit 0 Shutter motor move direction */
    uint8_t diffuser_status_flags;
        /*bit 7-2 Diffuser Status Register - Reserved for future use */
        /*bit 1 Diffuser motor move status */
        /*bit 0 Diffuser motor direction */
    double shutter_motor_pulse_time_sec; /*Time between shutter motor pulses */
    double diffuser_motor_pulse_time_sec;
    /*Time between diffuser motor pulses */
    double diffuser_motor_pulse_length_sec; /*Length of diffuser motor pulses */
    uint16_t shutter_move_count; /*The number of steps that will be used for a
        shutter motor move*/
    uint16_t shutter_resolver_position;
    uint16_t diffuser_move_count; /*The number of steps that will be used for a
        diffuser motor move*/
    uint16_t diffuser_resolver_position;
    uint16_t diffuser_flags;
        /*bit 15 indicates the status of the shutter/diffuser motor drive*/
        /*bit 14 Shutter/diffuser relay status (active side) */
        /*bit 13 Shutter/diffuser relay status (inactive side) */
        /*bit 12-0 Reserved for future use*/
    uint8_t stl_command_rejected_count;
        /*Stim Lamp Ctrl CSC Command Reject Count*/
    uint8_t stl_command_accepted_count;
        /*Stim Lamp Ctrl CSC Command Accept Count*/
    uint8_t stl_power_flags;
        /*bit 7-2 Reserved for future use */
        /*bit 1 Actual Main Power Status*/
        /*bit 0 Actual Output Current Status*/
    uint8_t stl_last_accepted_command; /* Opcode of Last Accepted Command */
    uint8_t stl_flags;
        /*bit 7-3 Reserved for future use*/
        /*bit 2 Stim Lamp Main Power Status*/
            /*0: On, 1: Off*/
        /*bit 1-0 Stim Lamp Bulb Pair Selection*/
            /*0: Pristine, 1: Backup, 2: Working, 3: None*/
    uint8_t reserved[IAS_L0R_OLI_TELEMETRY_GROUP_4_RESERVED];
    uint8_t spare[IAS_L0R_OLI_TELEMETRY_GROUP_4_SPARE];
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_OLI_TELEMETRY_GROUP_4;

typedef struct IAS_L0R_OLI_TELEMETRY_GROUP_5
{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    int16_t days_orig;
    int32_t milliseconds_orig;
    int16_t microseconds_orig;
    uint16_t sync_word;
    uint16_t id;
    uint8_t fpe_command_reject_count;
    uint8_t fpe_command_accept_count;
    uint8_t safe_mode_consecutive_requests;
    /* Number of consecutive unacknowledged telemetry requests
        allowed before FSW transitions to safe mode.*/
    uint8_t last_command_opcode;
    uint8_t single_bit_edac_errors_detected;
    uint8_t consecutive_unacknowledged_requests;
    uint8_t fpe_message_errors_detected;
    uint8_t multi_bit_edac_errors_detected;
    uint16_t messages_forwarded_to_fpe;
    uint8_t command_sequence_count;
    uint8_t messages_reject_invalid_mode;
    uint8_t fpe_telemetry_valid;
    double dlvps_relay_pos_28vdc_voltage; /*0-14bit (0 - 35vdc) */
    double dlvps_pos_5v_voltage;          /* (0 - 6vdc) */
    double dlvps_pos_15v_voltage;         /* (0 - 15vdc) */
    double dlvps_neg_15v_voltage;         /* (0 -  -15vdc) */
    double dlvps_pos_3_3v_voltage;        /* (0 - 5vdc) */
    double alvps_hv_bias_pos_85v_voltage; /* (0 - ?vdc) */
    double alvps_pos_12v_voltage;         /* (0 - 15vdc) */
    double alvps_pos_7_5v_voltage;        /* (0 - 10vdc) */
    double alvps_neg_2_5v_voltage;        /* (0 -  -4vdc) */
    double alvps_pos_12v_current_amps;         /* (0 - xa) */
    double alvps_pos_7_5v_current_amps;        /* (0 - xa) */
    double alvps_pos_2_5v_current_amps;        /* (0 - xa) */
    double lvps_temperature_sensor_celsius;       /* (-20 -100c) */
    double ctlr_temperature_sensor_celsius;       /* (-20 -100c) */
    double ana_0_temperature_sensor_celsius;      /* (-20 - 100c) */
    double ana_1_temperature_sensor_celsius;      /* (-20 - 100c) */
    double ana_0_ch_0_vpa_bias_voltage;   /* (0 - 6.0vdc) */
    double ana_0_ch_1_vpa_bias_voltage;   /* (0 - 6.0vdc) */
    double ana_0_ch_2_vpa_bias_voltage;   /* (0 - 6.0vdc) */
    double ana_0_ch_3_vpa_bias_voltage;   /* (0 - 6.0vdc) */
    double ana_0_ch_4_vpa_bias_voltage;   /* (0 - 6.0vdc) */
    double ana_0_ch_5_vpa_bias_voltage;   /* (0 - 6.0vdc) */
    double ana_0_ch_6_vpa_bias_voltage;   /* (0 - 6.0vdc) */
    double ana_0_ch_7_vpa_bias_voltage;   /* (0 - 6.0vdc) */
    uint8_t reserved[IAS_L0R_OLI_TELEMETRY_GROUP_5_RESERVED];
    uint8_t spare[IAS_L0R_OLI_TELEMETRY_GROUP_5_SPARE];
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_OLI_TELEMETRY_GROUP_5;

typedef struct IAS_L0R_OLI_TELEMETRY{
    IAS_L0R_OLI_TELEMETRY_GROUP_3 oli_telemetry_header_3;
    IAS_L0R_OLI_TELEMETRY_GROUP_4 oli_telemetry_header_4;
    IAS_L0R_OLI_TELEMETRY_GROUP_5 oli_telemetry_header_5;
}IAS_L0R_OLI_TELEMETRY;

/*******************************************************************************
*TIRS telemetry data structures
*******************************************************************************/
/* IAS_L0R_TIRS_TELEMETRY_SSM_ENCODER_POSITION_SAMPLE_COUNT is the count of
   raw, oversampled TIRS telemetry records from the L0R.
   IAS_L0R_TIRS_SSM_ENCODER_SAMPLES is the count after the oversampling has
   been eliminated by ias_sensor_align_ssm_data. */
#define IAS_L0R_TIRS_TELEMETRY_SSM_ENCODER_POSITION_SAMPLE_COUNT 21
#define IAS_L0R_TIRS_SSM_ENCODER_SAMPLES 20
#define IAS_L0R_TIRS_TELEMETRY_BLOCK_2_RESERVED 2
#define IAS_L0R_TIRS_TELEMETRY_BLOCK_3_RESERVED 1
#define IAS_L0R_TIRS_TELEMETRY_BLOCK_4_RESERVED 8
#define IAS_L0R_TIRS_TELEMETRY_NUM_TIMING_TABLES 3

/* These are for the values of ssm_position_sel */
#define IAS_L0R_TIRS_SSM_SAFE_POSITION 0
#define IAS_L0R_TIRS_SSM_NADIR_POSITION 1
#define IAS_L0R_TIRS_SSM_DEEP_SPACE_POSITION 2
#define IAS_L0R_TIRS_SSM_BLACK_BODY_POSITION 3

/* A structure for the opcode and command count data */
typedef struct IAS_L0R_TIRS_TELEMETRY_COMMAND
{
    uint8_t unaccepted_command_count;
    uint8_t accepted_command_count;
    uint8_t pulse_per_second_count;
    uint8_t tod_command_counter;
    int16_t day;
    int32_t millisecond;
    uint8_t mc_encoder_flags;
    /*  Bit Purpose
        7-4	Unused
        3	POWER
        2	RAMP
        1	IDX
        0	IDX_ACQ */
    uint16_t science_data_frame_capture_count;
    float science_acquisition_frame_rate;
    uint8_t active_timing_table_pattern; /* 0, 1, 2, or 3 */
    uint16_t mode_register;
    uint8_t timing_table_pattern_id[IAS_L0R_TIRS_TELEMETRY_NUM_TIMING_TABLES];
        /* assigned by LDCM scientist */
    uint8_t ssm_position_sel;
    uint8_t ssm_mech_mode; /* 0 - 15 */
    uint32_t ssm_encoder_position_sample
        [IAS_L0R_TIRS_TELEMETRY_SSM_ENCODER_POSITION_SAMPLE_COUNT];

}IAS_L0R_TIRS_TELEMETRY_COMMAND;

#define IAS_L0R_TIRS_TELEMETRY_NUM_BLACK_BODY_TEMPS 4
typedef struct IAS_L0R_TIRS_TELEMETRY_TEMPERATURES
{
    float bbcal_op7_a_celsius;
    float bbcal_op7_b_celsius;
    float bbcal_supp_1_celsius;
    float blackbody_calibrator_celsius
        [IAS_L0R_TIRS_TELEMETRY_NUM_BLACK_BODY_TEMPS];
    float cold_stage_heat_strap_cf_if_celsius;
    float cryo_diode_t3_measured_celsius;
    float cryo_diode_t4_measured_celsius;
    float cryo_shroud_outer_at_tunnel_celsius;
    float cryo_shroud_outer_flange_celsius;
    float fixed_baff_nadir_aft_hot_corner_celsius;
    float fixed_baff_nadir_aft_space_corner_celsius;
    float fixed_baff_nadir_fwd_hot_corner_celsius;
    float fixed_baff_nadir_fwd_space_corner_celsius;
    float fp_a_asic_celsius;
    float fp_b_asic_celsius;
    float fpe1_fpe_a_asic_celsius;
    float fpe2_fpe_b_asic_celsius;
    float fp_f2_fine_sensor_1_celsius;
    float fp_f4_fine_sensor_3_celsius;
    float fp_f6_fine_sensor_1_celsius;
    float fp_f7_fine_sensor_2_celsius;
    float fp_op6_a_celsius;
    float fp_op6_b_celsius;
    float optical_deck_celsius;
    float spare_4_thermistor_celsius;
    float spare_5_thermistor_celsius;
    float ssm_bearing_aft_celsius;
    float ssm_bearing_fwd_celsius;
    float ssm_bearing_housing_d4_aft_hot_side_celsius;
    float ssm_bearing_housing_d5_fwd_hot_side_celsius;
    float ssm_bearing_housing_d6_aft_space_side_celsius;
    float ssm_bearing_housing_d7_fwd_space_side_celsius;
    float ssm_bh_op5_a_celsius;
    float ssm_bh_op5_b_celsius;
    float ssm_encoder_remote_elec_celsius;
    float ssm_enc_read_head_sensor_1_celsius;
    float ssm_motor_housing_celsius;
    float structure_foot_a_neg_z_celsius;
    float structure_foot_c_pos_z_celsius;
    float structure_nadir_aperture_celsius;
    float tcb_board_celsius;
    float telescope_aft_barrel_neg_z_celsius;
    float telescope_aft_barrel_pos_z_celsius;
    float telescope_aft_op3_a_celsius;
    float telescope_aft_op3_b_celsius;
    float telescope_fwd_barrel_neg_z_celsius;
    float telescope_fwd_barrel_pos_z_celsius;
    float telescope_fwd_op4_a_celsius;
    float telescope_fwd_op4_b_celsius;
    float telescope_stage_op2_a_celsius;
    float telescope_stage_op2_b_celsius;

}IAS_L0R_TIRS_TELEMETRY_TEMPERATURES;

#define IAS_L0R_TIRS_TELEMETRY_NUM_VPD_CURRENTS 3
typedef struct IAS_L0R_TIRS_TELEMETRY_CIRCUIT_FOCAL_PLANE
{
    float mon_pos_12v_volts;
    float a_vpd_current_1_amps[IAS_L0R_TIRS_TELEMETRY_NUM_VPD_CURRENTS];
    float detector_substrate_conn_for_sca_a_roic_volts; /* ~5.5v - 8.5v */
    float detector_substrate_conn_for_sca_b_roic_volts; /* ~5.5v - 8.5v */
    float detector_substrate_conn_for_sca_c_roic_volts; /* ~5.5v - 8.5v */
    float digi_supply_mon_pos_5_5_for_sca_c_roic_volts;
    float supply_mon_pos_5_5_for_sca_a_roic_volts;
    float supply_mon_pos_5_5_for_sca_b_roic_volts;
    float supply_mon_pos_5_5_for_sca_c_roic_volts;
    float output_ref_level_mon_5_5_for_sca_c_roic_volts;
    float supply_10v_for_sca_a_current_mon_amps; /* max: 500ma */
    float supply_10v_for_sca_b_current_mon_amps; /* max: 500ma */
    float supply_10v_for_sca_c_current_mon_amps; /* max: 500ma */
    float output_driver_pos_5_5_for_sca_c_roic_volts;
    float output_ref_level_1_6_for_sca_c_roic_volts;
    float channel_ref_suppy_1_6_for_sca_c_roic_volts;
    float vpe_sca_a_video_ref;
    float vpe_sca_b_video_ref;
    float vpe_sca_c_video_ref;
}IAS_L0R_TIRS_TELEMETRY_CIRCUIT_FOCAL_PLANE;

#define IAS_L0R_TIRS_TELEMETRY_NUM_CIRCUIT_BOARDS 2
typedef struct IAS_L0R_TIRS_TELEMETRY_CIRCUIT
{
    IAS_L0R_TIRS_TELEMETRY_CIRCUIT_FOCAL_PLANE focal_plane_boards
        [IAS_L0R_TIRS_TELEMETRY_NUM_CIRCUIT_BOARDS];
        /* Board A is 1st element and Board B 2nd */
    float cosine_motor_drive_for_mce_current_amps;
    float sine_motor_drive_for_mce_current_amps;
    float hsib_3_3_current_mon_amps;
    float cosine_dac_telemetry_for_mce_volts;
    float sine_dac_telemetry_for_mce_volts;
    uint8_t elec_enabled_flags;
    /*  Bit Purpose
        7-4	Unused
        3	FPE B Enable
        2	FPE A Enable
        1	MCE B Enable
        0	MCE A Enable */
}IAS_L0R_TIRS_TELEMETRY_CIRCUIT;

/* Note about the fill flag: Unlike OLI, the TIRS telemetry has only one time
   stamp and is part of the same mission data group.  Therefore it only has the
   one fill flag, versus the 3 in the OLI telemetry */
typedef struct IAS_L0R_TIRS_TELEMETRY
{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI
                     Note: Original Time is in IAS_L0R_TIRS_TELEMETRY_COMMAND */
    IAS_L0R_TIRS_TELEMETRY_COMMAND command;
    IAS_L0R_TIRS_TELEMETRY_TEMPERATURES temperature;
    IAS_L0R_TIRS_TELEMETRY_CIRCUIT circuit;
    uint8_t reserved_block_2[IAS_L0R_TIRS_TELEMETRY_BLOCK_2_RESERVED];
    uint8_t reserved_block_3[IAS_L0R_TIRS_TELEMETRY_BLOCK_3_RESERVED];
    uint8_t reserved_block_4[IAS_L0R_TIRS_TELEMETRY_BLOCK_4_RESERVED];
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_TIRS_TELEMETRY;

/* define the convenience structure for holding just the select TIRS scene
   select mirror encoder information that is needed. */
typedef struct IAS_L0R_TIRS_SSM_ENCODER
{
   IAS_L0R_TIME l0r_time; /* time for the first encoder position. This time is
                             copied from the IAS_L0R_TIRS_TELEMETRY record
                             associated with these samples */ 
    uint8_t elec_enabled_flags;
    /*  Bit Purpose
        7-4	Unused
        3	FPE B Enable
        2	FPE A Enable
        1	MCE B Enable
        0	MCE A Enable */
    uint8_t ssm_mech_mode; /* 0 - 15 */
    uint32_t encoder_position
        [IAS_L0R_TIRS_TELEMETRY_SSM_ENCODER_POSITION_SAMPLE_COUNT];
    /* encoder position every 1.0/SSM_ENCODER_POSITION_SAMPLE_COUNT seconds */
    uint8_t ssm_position_sel; /* Scene select mirror position, showing where
        the SSM is pointing */
} IAS_L0R_TIRS_SSM_ENCODER;



/*******************************************************************************
*Inertial Measurement Unit Data (IMU) data
*  also known as: Inertial Reference Unit Data (IRU) data
*******************************************************************************/

/* The number of the 50 gyro samples which is stored per IMU record. */
#define IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD 50

typedef struct IAS_L0R_IMU_SAMPLE
{
    int16_t sync_event_time_tag;
    uint16_t time_tag;
    uint8_t saturation_and_scaling;
    /*bit 7 indicates Gyro A saturation -- 0=no 1=yes*/
    /*bit 6 indicates Gyro B saturation -- 0=no 1=yes*/
    /*bit 5 indicates Gyro C saturation -- 0=no 1=yes*/
    /*bit 4 indicates Gyro D saturation -- 0=no 1=yes*/
    /*bit 3 indicates Gyro A Scaling Factor -- 0=low 1=high*/
    /*bit 2 indicates Gyro B Scaling Factor -- 0=low 1=high*/
    /*bit 1 indicates Gyro C Scaling Factor -- 0=low 1=high*/
    /*bit 0 indicates Gyro D Scaling Factor -- 0=low 1=high*/
    uint8_t angular_rate_valid;
    /*bit 7 indicates Gyro A rate valid -- 0=no 1=yes*/
    /*bit 6 indicates Gyro B rate valid -- 0=no 1=yes*/
    /*bit 5 indicates Gyro C rate valid -- 0=no 1=yes*/
    /*bit 4 indicates Gyro D rate valid -- 0=no 1=yes*/
    uint16_t integrated_angle_count[IAS_L0R_NUM_GYROS];
}IAS_L0R_IMU_SAMPLE;

typedef struct IAS_L0R_IMU
{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    int32_t time_tag_sec_orig;
    int32_t time_tag_subseconds_orig; /* tenths of microseconds */
    IAS_L0R_IMU_SAMPLE gyro_samples[IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD];
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_IMU;

typedef struct IAS_L0R_IMU_LATENCY
{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    double fine_ad_solution_time;
    float measured_imu_latency;
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_IMU_LATENCY;

/*******************************************************************************
*GPS_POSITION Receiver Ancillary Data
*******************************************************************************/
#define IAS_GPS_NUM_SATELLITES 12
typedef struct IAS_L0R_GPS_SAT
{
    uint8_t id;              /* 0 to 37 */
    uint8_t track_mode;
    /* Flag indicating one of the tracking modes below:
       CODE_SEARCH&CODE_ACQUIRE&AGC_SET&FREQ_ACQUIRE&
       BIT_SYNC_DETECT&MSG_SYNC_DETECT&SAT_TIME_AVAIL&EP
       HEM_ACQUIRE&AVAIL_FOR_POS
       TBD: The full state names and which bit indicates
            which state.  Also can multiple bits be on at once? */
    uint8_t signal_strength; /* 0 to 255 */
    uint8_t channel_status_flags;
}IAS_L0R_GPS_SAT;

typedef struct IAS_L0R_GPS_POSITION
{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    uint8_t month;
    uint8_t day;
    uint16_t year;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint32_t nanoseconds;
    uint8_t function;
    uint8_t sub_function;
    IAS_DBL_LAT_LONG point_arc_secs; /* lat/lon in seconds  */
    double height_uncorrected_meters;
    double height_corrected_meters;
    double velocity_meters_per_sec;
    double heading_degrees;
    float current_dop;     /* dilution of precision */
    uint8_t dop_type;      /* dilution of precision type */
    uint8_t num_visible_satellites;
    uint8_t num_satellites_tracked;
    IAS_L0R_GPS_SAT tracked_satellites[IAS_GPS_NUM_SATELLITES];
    uint8_t receiver_status_flags;
    IAS_VECTOR ecef_position_meters;
    IAS_VECTOR ecef_velocity_meters_per_sec;
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_GPS_POSITION;

typedef struct IAS_L0R_GPS_SAT_RANGE
{
    uint8_t id;
    uint8_t tracking_mode;
    int32_t gps_time_seconds;
    int32_t gps_time_nanoseconds;
    uint32_t raw_code_phase;
    uint32_t integrated_carrier_phase_cycles;
    double integrated_carrier_phase_deg;
    double code_discriminator_output;
}IAS_L0R_GPS_SAT_RANGE;

typedef struct IAS_L0R_GPS_RANGE
{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    int32_t seconds; /* GPS local time */
    int32_t nanoseconds;
    uint8_t function;
    uint8_t sub_function;
    IAS_L0R_GPS_SAT_RANGE tracked_satellites[IAS_GPS_NUM_SATELLITES];
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_GPS_RANGE;

/*******************************************************************************
*Star Tracker Ancillary Data
*******************************************************************************/
#define IAS_STAR_TRACKER_NUM_STARS 6

typedef struct IAS_L0R_STAR
{
    uint8_t valid; /* 0,1 star validity flag */
    uint16_t id; /* star catalog ID*/
    IAS_DBL_XY position_arcsec;/*boresight horizontal and vertical components*/
    uint16_t background_bias;
    float intensity_mi; /* Star magnitude in range 1-7.3Mi
                        Mi = Mv for a class K0v star */
}IAS_L0R_STAR;

typedef struct IAS_L0R_STAR_TRACKER_CENTROID
{
    uint16_t quaternion_index; /* index for the corresponding 
                                  quaternion record */
    IAS_L0R_STAR stars[IAS_STAR_TRACKER_NUM_STARS]; /* array of star info */
    uint16_t effective_focal_length;
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_STAR_TRACKER_CENTROID;


#define IAS_L0R_STAR_TRACKER_QUATERNION_RESERVED 3
#define IAS_L0R_STAR_TRACKER_NUM_VIRTUAL_STATES 6
typedef struct IAS_L0R_STAR_TRACKER_QUATERNION
{
    uint16_t quaternion_index; /* index for current quaternion record */
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    int32_t udl_time_sec_orig;
    int32_t udl_time_subsec_orig;
    int32_t sta_time_tag;
    uint8_t status_flags_1;
    /* -------
       Bit 7 - Message Incomplete
       Bits 6-4 Quaternion Status
       Bits 3-0 Tracker Mode Status*/
    uint8_t status_flags_2;
    /* -------
       Bit 7 - ATM Mode Status
       Bit 6 - ADM Mode Status
       Bit 5 - Awaiting Complete
       Bit 4 - Catalog Complete
       Bits 3-0 Diagnostic Sub-Mode Status*/
    uint8_t last_processed_command;
    uint8_t virtual_tracker_states[IAS_L0R_STAR_TRACKER_NUM_VIRTUAL_STATES];
    /* Index 0 = Virtual Tracker State #0
       Index 1 = Virtual Tracker State #1
       ............
       Index 5 = Virtual Tracker State #5*/
    uint8_t command_flags;
    uint8_t time_message_value;
    uint8_t camera_id;
    uint8_t sw_version;
    double quaternion_seconds;
    double quaternion_element_1;
    double quaternion_element_2;
    double quaternion_element_3;
    double quaternion_element_4;
    double loss_function_value;
    uint16_t atm_frame_count;
    uint8_t total_sa_writes;
    uint8_t total_sa_reads;
    uint8_t sa_15_writes;
    uint8_t sa_15_reads;
    uint8_t sa_26_writes;
    uint8_t sa_29_reads;
    uint8_t status_flags_3;
    /*--------
       Bits 7-3 Unused
       Bit 2 LED Commanded State
       Bit 1 LED Throughput Fail
       Bit 0 Boot ROM Checksum Fail*/
    uint8_t adm_separation_tolerance_arc_secs;
    uint8_t adm_position_tolerance_arc_secs;
    float adm_mag_tolerance;
    uint8_t hot_pixel_count;
    uint8_t hot_pixel_threshold;
    uint8_t track_mode_pixel_threshold;
    uint8_t acquisition_mode_pixel_threshold;
    double tec_setpoint;
    IAS_DBL_XY boresight;
    float ccd_temperature_celsius;
    float lens_cell_temperature_celsius;
    uint8_t reserved[IAS_L0R_STAR_TRACKER_QUATERNION_RESERVED];
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_STAR_TRACKER_QUATERNION;

/*******************************************************************************
*Temperature following temperature data is
*******************************************************************************/
/* Enumerated type used to access individual temperature data by thermistor */
typedef enum IAS_L0R_OLI_TIRS_TEMPERATURE_INDEX
{
    OLI_PRIMARY_MIRROR_FLEXURE,
    OLI_TELESCOPE_POSITIVE_Z_NEGATIVE_Y_STRUT_TUBE,
    OLI_FPE_HEAT_PIPE_EVAPORATIOR,
    OLI_BASEPLATE_POSITIVE_Z,
    OLI_BASEPLATE_NEGATIVE_Z,
    OLI_PRIMARY_MIRROR_BENCH_AT_FLEX,
    OLI_SECONDARY_MIRROR_CENTER,
    OLI_SECONDARY_MIRROR_EDGE,
    OLI_SECONDARY_MIRROR_FLEXURE,
    OLI_SECONDARY_MIRROR_BENCH_AT_FLEX,
    OLI_TERTIARY_MIRROR_CENTER,
    OLI_TERTIARY_MIRROR_EDGE,
    OLI_TERTIARY_MIRROR_FLEXURE,
    OLI_TERTIARY_MIRROR_BENCH_AT_FLEX,
    OLI_QUAT_MIRROR_CENTER,
    OLI_QUAT_MIRROR_EDGE,
    OLI_FPA_1_RADIATOR,
    OLI_QUAT_MIRROR_FLEXURE,
    OLI_FPA_2_HEAT_PIPE_EVAPORATOR,
    OLI_FPA_3_HEAT_PIPE_CONDENSOR,
    OLI_FPA_4_MOLY_BP_PRIMARY,
    OLI_FPA_5_MOLY_BP_REDUNDANT,
    OLI_FPA_6_SINK,
    OLI_FPA_7_COLD_CABLE_RADIATOR,
    OLI_FPA_8_MLI_NEGATIVE_Y_BENCH_TEDLAR,
    OLI_FPA_9_FOOT_AT_SPACECRAFT_INTERFACE,
    OLI_FPA_10_CONDENSOR,
    TIRS_TB1_CH49_BANK4_01,
    TIRS_TB1_CH50_BANK4_02,
    OLI_FPE_RADIATOR,
    TIRS_TB1_CH51_BANK4_03,
    OLI_FPE_HEAT_PTPT_CONDENSOR,
    TIRS_TB1_CH52_BANK4_04,
    OLI_FPE_CHASSIS_PRIMARY,
    OLI_BASEPLATE_POSITIVE_Y,
    OLI_FPE_CHASSIS_REDUNDANT,
    OLI_ISE_CHASSIS_PRIMARY,
    OLI_ISE_CHASSIS_REDUNDANT,
    OLI_ISE_RADIATOR,
    OLI_QUAT_MIRROR_BENCH_AT_FLEX,
    OLI_BENCH_POSITIVE_Y_1,
    OLI_BENCH_POSITIVE_Y_2,
    OLI_BENCH_POSITIVE_Y_3,
    OLI_BENCH_NEGATIVE_Y_1,
    OLI_BENCH_NEGATIVE_Y_2,
    OLI_BENCH_NEGATIVE_X,
    OLI_BENCH_POSITIVE_X_1,
    OLI_BENCH_POSITIVE_X_2,
    OLI_CAL_ASSEMBLY_DIFFUSER_COVER,
    OLI_NEGATIVE_X_FOCUS_MECHANISM,
    OLI_STIMULATION_LAMP_1_DIODE_BOARD,
    OLI_TB1_CH72_BANK5_8,
    OLI_TB1_CH73_BANK5_9,
    OLI_TB1_CH74_BANK5_10,
    OLI_STIMULATION_LAMP_2_DIODE_BOARD,
    OLI_BENCH_NEGATIVE_X_PANEL,
    OLI_DIFFUSER_WHEEL_MOTOR,
    OLI_SHUTTER_WHEEL_MOTOR,
    TIRS_TB1_CH87_BANK6_7,
    TIRS_TB1_CH88_BANK6_8,
    TIRS_TB1_CH89_BANK6_9,
    OLI_BASEPLATE_NEGATIVE_Y,
    TIRS_TB1_CH90_BANK6_10,
    OLI_PRIMARY_MIRROR_CENTER,
    TIRS_TB1_CH91_BANK6_11,
    OLI_PRIMARY_MIRROR_EDGE,
    TIRS_TB1_CH92_BANK6_12,
    IAS_L0R_OLI_TIRS_TEMPERATURE_NUM_INDEXES
}IAS_L0R_OLI_TIRS_TEMPERATURE_INDEX;

typedef struct IAS_L0R_OLI_TIRS_TEMPERATURE
{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    float temperatures_celsius[IAS_L0R_OLI_TIRS_TEMPERATURE_NUM_INDEXES];
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_OLI_TIRS_TEMPERATURE;

typedef enum IAS_L0R_GYRO_TEMPERATURE_INDEX
{
    GYRO_A_FILTERED_RESONATOR,
    GYRO_A_FILTERED_DERIVATIVE_OF_RESONATOR,
    GYRO_A_FILTERED_ELECTRONICS,
    GYRO_A_FILTERED_DERIVATIVE_OF_ELECTRONICS,
    GYRO_A_FILTERED_DIODE,
    GYRO_A_FILTERED_DERIVATIVE_OF_DIODE,
    GYRO_A_FILTERED_CASE,
    GYRO_A_FILTERED_DERIVATIVE_OF_CASE,
    GYRO_B_FILTERED_RESONATOR,
    GYRO_B_FILTERED_DERIVATIVE_OF_RESONATOR,
    GYRO_B_FILTERED_ELECTRONICS,
    GYRO_B_FILTERED_DERIVATIVE_OF_ELECTRONICS,
    GYRO_B_FILTERED_DIODE,
    GYRO_B_FILTERED_DERIVATIVE_OF_DIODE,
    GYRO_B_FILTERED_CASE,
    GYRO_B_FILTERED_DERIVATIVE_OF_CASE,
    GYRO_C_FILTERED_RESONATOR,
    GYRO_C_FILTERED_DERIVATIVE_OF_RESONATOR,
    GYRO_C_FILTERED_ELECTRONICS,
    GYRO_C_FILTERED_DERIVATIVE_OF_ELECTRONICS,
    GYRO_C_FILTERED_DIODE,
    GYRO_C_FILTERED_DERIVATIVE_OF_DIODE,
    GYRO_C_FILTERED_CASE,
    GYRO_C_FILTERED_DERIVATIVE_OF_CASE,
    GYRO_D_FILTERED_RESONATOR,
    GYRO_D_FILTERED_DERIVATIVE_OF_RESONATOR,
    GYRO_D_FILTERED_ELECTRONICS,
    GYRO_D_FILTERED_DERIVATIVE_OF_ELECTRONICS,
    GYRO_D_FILTERED_DIODE,
    GYRO_D_FILTERED_DERIVATIVE_OF_DIODE,
    GYRO_D_FILTERED_CASE,
    GYRO_D_FILTERED_DERIVATIVE_OF_CASE,
    IAS_L0R_GYRO_TEMPERATURE_NUM_INDEXES

}IAS_L0R_GYRO_TEMPERATURE_INDEX;

#define IAS_L0R_GYRO_TEMPERATURE_RESERVED 192
typedef struct IAS_L0R_GYRO_TEMPERATURE
{
    IAS_L0R_TIME l0r_time; /*time from J2000 epoch TAI */
    float temperatures_celsius[IAS_L0R_GYRO_TEMPERATURE_NUM_INDEXES];
    uint8_t reserved[IAS_L0R_GYRO_TEMPERATURE_RESERVED]; /* Reserved bytes */
    uint8_t warning_flag; /* Value is 1 if the time code was outside
                             tolerance of its expected value, else 0 */
}IAS_L0R_GYRO_TEMPERATURE;

/*******************************************************************************
*Band File structs
*******************************************************************************/
typedef enum
{
    IAS_L0R_COMPRESSION_OFF,
    IAS_L0R_COMPRESSION_ON,
    IAS_L0R_COMPRESSION_DEFAULT,
}IAS_L0R_BAND_COMPRESSION;


/*******************************************************************************
*Metadata structures

Time Format:  For fields present in the metadata which indicate a time, shall
              in the following format: YYYY:DDD:HH:MM:SS.SSSSSSS
              where:
              YYYY = four-digit Julian year,
              DDD = day (001-366*),
              HH = hours (00-23),
              MM = minutes (00-59),
              SS = seconds (00-59),
              SSSSSSS = fractional seconds (0-9999999)
              * For cases when active imaging occurs at the end of a leap year.
              Time is in TAI from the spacecraft.

*******************************************************************************/
typedef struct IAS_L0R_FILE
{
    char name[IAS_L0R_FILE_NAME_LENGTH+1]; /* file name */
}IAS_L0R_FILE;

/**
*  This struct contains information about the files that contain metadata
*/
typedef struct IAS_L0R_FILE_METADATA
{
    IAS_L0R_FILE ancillary_file; /* information about the ancillary file */
    IAS_L0R_FILE checksum_file;
    IAS_L0R_FILE band_files[IAS_L0R_MAX_BAND_FILES]; /* information on each band
                                                             file */
    uint16_t interval_files;
    IAS_L0R_FILE metadata_file;
}IAS_L0R_FILE_METADATA;

/**
*This struct contains the interval level metadata
*
*Contents are listed in alphabetical order as found in the LMDD
*/
typedef struct IAS_L0R_INTERVAL_METADATA
{
    char ancillary_start_time[IAS_L0R_DATE_LENGTH+1];
    /* Year, Day of year and spacecraft start time
        associated with the first group ancillary data received */
    char ancillary_stop_time[IAS_L0R_DATE_LENGTH+1];
    /* Year, Day of year and spacecraft start time
        associated with the last group ancillary data received */
    uint32_t attitude_points; /*good spacecraft attitude points */
    uint32_t attitude_points_missing; /*spacecraft attitude points missing*/
    uint32_t attitude_points_rejected; /*spacecraft attitude points rejected*/
    char collection_type[IAS_L0R_COLLECTION_TYPE_LENGTH+1];
    IAS_L0R_CORNERS corners_oli; /* OLI corner points of the interval */
    IAS_L0R_CORNERS corners_tirs; /* TIRS corner points of the interval */
    char cpf_name[IAS_L0R_CPF_NAME_LENGTH+1];  /* CPF used for processing */
    uint32_t crc_errors_oli; /* Number of frame CRC failures for OLI */
    uint32_t crc_errors_tirs; /* Number of frame CRC failures for TIRS */
    char data_type[IAS_L0R_DATA_TYPE_LENGTH+1]; /*data type identifier string */
    char date_acquired[IAS_L0R_DATE_LENGTH+1];
    /* Year, Day of year and GMT that this scene was imaged */
    uint32_t ephemeris_points; /* good ephemeris data points received */
    uint32_t ephemeris_points_missing; /* ephemeris data points found missing */
    uint32_t ephemeris_points_rejected;/* ephemeris data points found to fail */
    uint32_t frames_filled_oli;/*number of OLI frames that were filled*/
    uint32_t frames_filled_tirs;/*number of TIRs frames that were filled*/
    char hostname[IAS_L0R_HOSTNAME_LENGTH+1];
    /* machine where data was processed*/
    int8_t image_quality_oli;
    int8_t image_quality_tirs;
    uint32_t interval_frames_oli; /*number of OLI frames reported
         in this interval file */
    uint32_t interval_frames_tirs; /*number of TIRS frames reported
         in this interval file */
    uint8_t interval_number;/*Sequence of the interval within the ingest file */
    uint8_t interval_version;/*Processed version of the interval, starts at 0*/
    char is_version[IAS_SOFTWARE_VERSION_SIZE+1];
    /* Version number of the software installed on the IS when a metadata file
    * and its associated Level 0R files were generated */
    char landsat_cal_interval_id[IAS_L0R_INTERVAL_ID_LENGTH+1];
    /* Unique Landsat calibration interval identifier*/
    char landsat_interval_id[IAS_L0R_INTERVAL_ID_LENGTH+1];
    char nadir_offnadir[IAS_L0R_NADIR_LENGTH+1];
    /* Nadir or Off-Nadir condition of the interval */
    char quality_algorithm[IAS_L0R_ALGORITHM_NAME_LENGTH+1];
    /*Algorithm (date, name, and version number) used to calculate the
        quality for this interval*/
    float roll_angle; /*amount of spacecraft roll at interval center */
    uint8_t satellite; /* Landsat vehicle number that captured the data */
    char sensor_id[IAS_L0R_SENSOR_ID_LENGTH+1]; /* name of the sensor  */
    char spacecraft_id[IAS_L0R_SPACECRAFT_ID_LENGTH+1];
    /*name of the spacecraft*/
    char start_time_oli[IAS_L0R_DATE_LENGTH+1];
    /* Year, Day of year and GMT spacecraft start time of the first OLI frame
         of the interval */
    char start_time_tirs[IAS_L0R_DATE_LENGTH+1];
    /* Year, Day of year and GMT spacecraft start time of the first TIRS frame
        of the interval */
    char station_id[IAS_L0R_STATION_ID_LENGTH+1]; /* receiving group
        station code */
    char stop_time_oli[IAS_L0R_DATE_LENGTH+1];
    /* Year, Day of year and spacecraft start time of the last OLI frame
        of the interval */
    char stop_time_tirs[IAS_L0R_DATE_LENGTH+1];
    /* Year, Day of year and spacecraft start time of the last TIRS frame
        of the interval */
    uint32_t time_code_errors_oli;/*number of OLI time code
        errors in the interval */
    uint32_t time_code_errors_tirs;/*number of TIRS time code
        errors in the interval */
    uint32_t detector_map_id_tirs; /* Detector map id for TIRS retrieved from
        the CPF. */
    uint8_t wrs_ending_row;
    /* WRS row number for the last reported scene in this interval */
    uint8_t wrs_scenes; /* total number of full & partial scenes
        in the interval  */
    uint8_t wrs_scenes_full;    /* number of full scenes in the interval */
    uint8_t wrs_scenes_partial; /* number of partial scenes in the interval */
    uint8_t wrs_starting_path;  /* starting WRS2 path for scenes */
    uint8_t wrs_starting_row;   /* WRS2 number row for the first scene */
    uint8_t wrs_type;           /* WRS which applies to the interval, WRS-2 */
}IAS_L0R_INTERVAL_METADATA;

/**
*The IAS_L0R_SCENE_METADATA struct contains the metadata about a WRS2 scene
*
*The IAS_L0R_SCENE_METADATA struct contains the metadata about a WRS2 scene
*Contents are listed in alphabetical order as found in the LMDD
*/
typedef struct IAS_L0R_SCENE_METADATA
{
    uint16_t attitude_points;         /*good spacecraft attitude points */
    uint16_t attitude_points_missing; /*spacecraft attitude points missing*/
    uint16_t attitude_points_rejected;/*spacecraft attitude points rejected*/
    IAS_L0R_CORNERS corners_oli;      /* OLI scene corner points */
    IAS_L0R_CORNERS corners_tirs;      /* OLI scene corner points */
    uint32_t crc_errors; /* Number of frame CRC failures both instruments */
    char date_acquired[IAS_L0R_TIME_LENGTH+1];
    /* timecode for the center frame */
    char day_night[IAS_L0R_DAY_NIGHT_LENGTH+1]; /* day/night condition
                                                 of the scene */
    uint16_t ephemeris_points;         /* good ephemeris data points received */
    uint16_t ephemeris_points_missing; /* ephemeris data points found missing */
    uint16_t ephemeris_points_rejected;/* ephemeris data points found to fail */
    char full_partial_scene[IAS_L0R_SCENE_FULL_PARTIAL_LENGTH+1];
    /* full or partial scene*/
    char hostname[IAS_L0R_HOSTNAME_LENGTH+1];
    int8_t image_quality_oli;
    int8_t image_quality_tirs;
    char landsat_scene_id[IAS_L0R_SCENE_ID_LENGTH+1]; /* unique ID used
                                                     for Landsat data */
    uint16_t missing_frames;
    char nadir_offnadir[IAS_L0R_NADIR_LENGTH+1];
    char sensor_present_oli; /* 1 if data for OLI is present, 0 if not */
    char sensor_present_tirs; /* 1 if data for TIRS is present, 0 if not */
    float roll_angle;        /* amount or spacecraft roll at the scene center */
    IAS_DBL_LAT_LONG scene_center; /* scene center point */
    int32_t scene_center_shift;
    /* distance between the calculated center and the nominal center*/
    uint32_t scene_start_frame_oli;  /* first OLI frame number in the scene */
    uint32_t scene_stop_frame_oli;   /* last OLI frame number in the scene */
    uint32_t scene_start_frame_tirs; /* first TIRS frame number in the scene */
    uint32_t scene_stop_frame_tirs;  /* last TIRS frame number in the scene */
    char start_time[IAS_L0R_TIME_LENGTH+1]; /* earliest spacecraft time of
                               any (OLI/TIRS) scene frame within the interval*/
    char stop_time[IAS_L0R_TIME_LENGTH+1]; /* latest spacecraft time of
                               any (OLI/TIRS) scene frame within the interval*/
    char subsetter_version_l0rp[IAS_SOFTWARE_VERSION_SIZE+1];
    /* Version of the subsetter that created this data.  This will only
       be applicable to L0Rp datasets */
    double sun_azimuth;   /* sun azimuth angle at the scene center */
    double sun_elevation; /* sun elevation angle at the scene center */
    uint8_t target_wrs_path; /* nearest WRS path to
                             the frame of sight scene center*/
    uint16_t target_wrs_row; /* nearest WRS row to the
                             frame of sight scene center or special high
                             latitude numbers (88x and 99x)*/
    uint16_t time_code_errors; /* the number of time code errors in the scene */
    uint16_t wrs_path;
    uint8_t wrs_row;
    uint8_t wrs_scene_number;
}IAS_L0R_SCENE_METADATA;

#define IAS_L0R_FRAME_HEADER_RESERVED_LENGTH 4
/*******************************************************************************
*IAS_L0R_OLI_FRAME_HEADER
* This struct contains the data from a frame header with an indicator to show
* the correction status of the frame. Note this is referred to as the Video Line
* Header in the Spacecraft documentation.
*******************************************************************************/
typedef struct IAS_L0R_OLI_FRAME_HEADER
{
    /* the frame(line) number associated with the frame */
    IAS_L0R_TIME l0r_time;
    int16_t day;          /* day of the year */
    int32_t milliseconds; /* milliseconds of the day */
    int16_t microseconds; /* microseconds of millisecond */
    uint8_t blind_data_included_in_frame; /* blind data received
                                                 in this frame */
    uint32_t frame_number;
    uint8_t time_error;    /* Whether or not the time information for this
                             frame is degraded */
    uint8_t reserved[IAS_L0R_FRAME_HEADER_RESERVED_LENGTH];
    /* vendor reserved space  */
    uint16_t frame_status;  /* field used with IAS_L0R_FRAME_STATUS  */
}IAS_L0R_OLI_FRAME_HEADER;

/*******************************************************************************
*IAS_L0R_OLI_IMAGE_HEADER
*******************************************************************************/
#define IAS_L0R_IMAGE_HEADER_RESERVED_LENGTH_1 3
#define IAS_L0R_IMAGE_HEADER_RESERVED_LENGTH_2 20
typedef struct IAS_L0R_OLI_IMAGE_HEADER
{
    IAS_L0R_OLI_FRAME_HEADER frame_header; /* The Frame Header associated
        with the Image Header. To determine if the Image Header was filled
        look at frame_status field for this Frame Header */
    uint32_t length_of_image;           /* number of frames in the image */
    uint32_t image_content_definition;  /* Vendor proprietary */
    uint16_t ms_integration_time;         /* Multi-spectral Integration Time
                                           in microseconds */ 
    uint16_t pan_integration_time;        /* Pan-chromatic Integration Time
                                           in microseconds */
    uint32_t ms_data_word;              /* vendor debugging information */
    uint32_t pan_data_word;             /* vendor debugging information */
    uint8_t extended_integration_flag;  /* 0 = Normal integration time
                                           1 = extended integration time*/
    uint8_t blind_band_record_rate;     /* A value of 1 indicated blind data
                                           is present */
    uint8_t test_pattern_setting;       /* 0 = Video data 1 = Test pattern */
    uint8_t current_detector_select_table; /* Which of the 5 detector select
        tables is active. Nominal value for imaging is 5 (see
        detector_select_table_id_number field) */
    uint8_t reserved_1[IAS_L0R_IMAGE_HEADER_RESERVED_LENGTH_1];
        /* vendor reserved space */
    uint32_t detector_select_table_id_number; /* Identifier for the primary
        (number 5) active detector select table in use. */
    uint8_t image_data_truncation_setting;    /* indicates which image
        bits are transmitted - 0 = upper 12 bits 1 = lower 12 bits*/
    uint8_t reserved_2[IAS_L0R_IMAGE_HEADER_RESERVED_LENGTH_2];
    /* vendor reserved space */
}IAS_L0R_OLI_IMAGE_HEADER;

/*******************************************************************************
*IAS_L0R_TIRS_FRAME_HEADER and related structures 
*******************************************************************************/
#define IAS_L0R_TIRS_NUM_BANDS 3
#define IAS_L0R_TIRS_NUM_SCAS 3
#define IAS_L0R_TIRS_NUM_ROWS 2

typedef struct IAS_L0R_TIRS_D_HEADER
{
    uint16_t sync_byte;
    uint16_t reserved_1;
    uint16_t reserved_2;
}IAS_L0R_TIRS_D_HEADER;

typedef struct IAS_L0R_TIRS_FPE_WORDS
{
    uint16_t start_code;
    uint16_t asic_id;
    uint16_t data_type;
    uint16_t time_stamp;
    uint16_t line_count;
    uint16_t asic_status;
    uint16_t reserved;
}IAS_L0R_TIRS_FPE_WORDS;

typedef struct IAS_L0R_TIRS_FRAME_HEADER
{
    IAS_L0R_TIME l0r_time; /* Frame time corrected by Ingest */
    int16_t day;          /* Original day of the year provided by TIRS */
    int32_t milliseconds; /* Original milliseconds of the day provided by
                              TIRS */
    int16_t microseconds; /* Original microseconds of millisecond provided
                              by TIRS*/
    uint32_t frame_number;
    uint8_t sync_byte;
    uint8_t reserved;
    uint8_t data_set_type;
    double integration_duration; /* Time during FSYNC low in microseconds */
    uint32_t total_frames_requested;
    uint8_t row_offset[IAS_L0R_TIRS_NUM_BANDS]
        [IAS_L0R_TIRS_NUM_SCAS][IAS_L0R_TIRS_NUM_ROWS];
    IAS_L0R_TIRS_D_HEADER d_header[IAS_L0R_TIRS_NUM_BANDS];
    IAS_L0R_TIRS_FPE_WORDS fpe_words[IAS_L0R_TIRS_NUM_BANDS]
        [IAS_L0R_TIRS_NUM_SCAS][IAS_L0R_TIRS_NUM_ROWS];
    uint8_t roic_crc_status_blind; /* The ROIC CRC Status flags */
    uint8_t roic_crc_status_10_8; 
    uint8_t roic_crc_status_12; 
    uint16_t frame_status; /* field used with IAS_L0R_FRAME_STATUS */
}IAS_L0R_TIRS_FRAME_HEADER;

/*******************************************************************************
* L0RIO
*   This struct is passed between functions to maintain file access information
*******************************************************************************/
typedef struct L0RIO L0RIO;

#endif
