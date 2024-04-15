/**
 *******************************************************************************
 * @file:    Eq_ARM.c
 * @author:  Danny Soppit
 * @brief:   This is the main file for applying the equalizer using the ARM
 *           CMSIS DSP library as referenced from:
 *           "https://www.keil.com/pack/doc/CMSIS/DSP/html/index.html"
 *
 * @Note:    This file is quite heavily commented and documented as it is
 *           supposed to show a simple example of applying the filters and
 *	     demonstrates every step. The only ambiguities are the data
 *	     obtaining methods that I have left for the user. Note that
 *	     through the CMSIS DSP documentation, there are manyforms of
 *           implementing the biquads with different efficiencies. Feel free
 *	     to try other methods, as I am confident there are many ways to
 *	     optimize the code for better performance.
 *
 *******************************************************************************
 */

//******************************************************************************
//  Include Files
//******************************************************************************

// STANDARD DEFINITONS
#include <stdio.h>
#include <math.h>

// ARM CMSIS DSP DEFINITONS
#include "arm_math.h"

//******************************************************************************
//  Defines
//******************************************************************************

// The biquad specific definitons below have to match the ones used in Python!
#define NUMBER_OF_BIQUAD_STAGES 3   // Number of stages used for the filter
#define NUMBER_OF_BANDS         6   // Number of equalization bands
#define COEFFICIENT_POSTSHIFT   4   // Postshift used when creating the coeffs
#define SAMPLES_PER_TRANSFER    256 // Example of apply 256 samples at a time
#define SCALE_FACTOR            1   // Placeholder scale factor

//******************************************************************************
//  Constant Variables
//******************************************************************************

// 3 stages * 6 bands * 5 coefficients for each biquad
// Note that this was copied from the Python terminal output
const q31_t BIQUAD_COEFF[NUMBER_OF_BIQUAD_STAGES * NUMBER_OF_BANDS * 5] =
{
    // Bandpass #1: 70.7 Hz to 141.4 Hz:
    349, 699, 349 264555182, -130541587, 
    134217728, -67, -134219283, 265663083, -131823456, 
    134217728, -268434645, 134216917, 267019266, -132913256,


    // Bandpass #2: 141.4 Hz to 282.8 Hz
    2721, 5441, 2721, 260375768, -126963381, 
    134217728, -67, -134219283, 262193941, -129474301, 
    134217728, -268434645, 134216917, 265393561, -131620459,
	
    // Bandpass #3: 282.8 Hz to 565.7 Hz
    20635, 41271, 20635, 251164150, -120080476, 
    134217728, -67, -134219283, 253261157, -124917151, 
    134217728, -268434645, 134216917, 261522959, -129065288,

    // Bandpass #4: 565.7 Hz to 1131.4 Hz
    456703, 913403, 456700, 204512719, -95574292,
    134217728, -67, -134219283, 194736516, -108736401,
    134217728, -268434645, 134216917, 238200252, -119098659,

    // Bandpass #5: 1131.4 Hz to 2262.7 Hz:
    149046, 298092, 149046, 229631975, -107282897, 
    134217728, -67, -134219283, 228128773, -116401482, 
    134217728, -268434645, 134216917, 251380082, -124047183,
	
    // Bandpass #6: 2262.7 Hz to 4525.5 Hz
    149046, 298092, 149046, 229631975, -107282897 
    134217728, -67, -134219283, 228128773, -116401482, 
    134217728, -268434645, 134216917, 251380082, -124047183
};

//******************************************************************************
//  Static Variables
//******************************************************************************

// 4 * (stages) as the 4 state variables typically represent the past two input...
// ...samples (x[n-1] and x[n-2]) and the past two output samples (y[n-1] and y[n-2])
static q63_t biquadStateBand1Q31[4 * NUMBER_OF_BIQUAD_STAGES];
static q63_t biquadStateBand2Q31[4 * NUMBER_OF_BIQUAD_STAGES];
static q63_t biquadStateBand3Q31[4 * NUMBER_OF_BIQUAD_STAGES];
static q31_t biquadStateBand4Q31[4 * NUMBER_OF_BIQUAD_STAGES];
static q31_t biquadStateBand5Q31[4 * NUMBER_OF_BIQUAD_STAGES];
static q31_t biquadStateBand6Q31[4 * NUMBER_OF_BIQUAD_STAGES];

// Input and output buffers:
static q31_t q31Src[SAMPLES_PER_TRANSFER];
static q31_t q31Dest[SAMPLES_PER_TRANSFER];

// Intermediate buffers for each band:
static q31_t outputB1[SAMPLES_PER_TRANSFER];
static q31_t outputB2[SAMPLES_PER_TRANSFER];
static q31_t outputB3[SAMPLES_PER_TRANSFER];
static q31_t outputB4[SAMPLES_PER_TRANSFER];
static q31_t outputB5[SAMPLES_PER_TRANSFER];
static q31_t outputB6[SAMPLES_PER_TRANSFER];

// Structs for biquad inits:
// It is noticed that Direct Form I is used as for numerical calculations it is
// more robust for data types.
static arm_biquad_cas_df1_32x64_ins_q31 B1;
static arm_biquad_cas_df1_32x64_ins_q31 B2;
static arm_biquad_cas_df1_32x64_ins_q31 B3;
static arm_biquad_casd_df1_inst_q31 B4;
static arm_biquad_casd_df1_inst_q31 B5;
static arm_biquad_casd_df1_inst_q31 B6;

//******************************************************************************
//  Function Prototypes
//******************************************************************************

// Example functions of the init and the audio equalization
static void ARM_Equalizer_init(void);
static void ARM_Equalizer(int16_t* pSrc, int16_t* pDest, uint16_t blocksize);

// Example of user custom functions for obtaining and transfering data
__attribute__((weak)) void user_custom_data_obtaining(uint16_t* databuf);
__attribute__((weak)) void user_custom_data_transfer(uint16_t* databuf);

//******************************************************************************
//  Functions
//******************************************************************************

/**
 *******************************************************************************
 * @brief:     Main function of the file
 * @parameter: N/A
 * @return:    N/A
 *******************************************************************************
 */
int main(void)
{
    // int16 data buffer, note that float can be used but then the equalization
    // function conversions would have to change to use arm_float_to_q15()
    // or arm_float_to_q31()
    int16_t* databuf;

    // Initialize the filters before using them
    ARM_Equalizer_init();

    // Example of using the filter through some type of thread
    // Note that the way to obtain the data has been left out
    while (1)
    {
        // Assume (SAMPLES_PER_TRANSFER) of data is placed into the databuf
        user_custom_data_obtaining(databuf);

        // Filter this buffer
        ARM_Equalizer(databuf, databuf, SAMPLES_PER_TRANSFER);

        // Assume (SAMPLES_PER_TRANSFER) of data is transferred somewhere else
        user_custom_data_transfer(databuf);
    }

    return 0;
}

/**
 *******************************************************************************
 * @brief:  Inits the structure and buffers for the biquad filter
 * @notes:  Note that for improved noise performance, we use high-precision
 *          32x64-bit Biquad filters for the low-frequency bands and standard
 *          32x32-bit Biquad filters for the high-frequency bands. But any
 *          could be used.
 * @param:  N/A
 * @return: N/A
 *******************************************************************************
 */
static void ARM_Equalizer_init(void)
{
    // Bandpass filter 1:
    // &BIQUAD_COEFF[0 * (NUMBER_OF_BIQUAD_STAGES * 5)] passes a pointer to the coefficients
    // for this specific band. We iterate the pointer by 15 for each bandpass filter
    // as each band has 3 stages * 5 coefficients = (NUMBER_OF_BIQUAD_STAGES * 5)
    arm_biquad_cas_df1_32x64_init_q31(&B1, NUMBER_OF_BIQUAD_STAGES,
                                      (q31_t*) &BIQUAD_COEFF[0 * (NUMBER_OF_BIQUAD_STAGES * 5)],
                                      &biquadStateBand1Q31[0], COEFFICIENT_POSTSHIFT);
    // Bandpass filter 2:
    arm_biquad_cas_df1_32x64_init_q31(&B2, NUMBER_OF_BIQUAD_STAGES,
                                      (q31_t*)&BIQUAD_COEFF[1 * (NUMBER_OF_BIQUAD_STAGES * 5)],
                                      &biquadStateBand2Q31[0], COEFFICIENT_POSTSHIFT);

    // Bandpass filter 3:
    arm_biquad_cas_df1_32x64_init_q31(&B3, NUMBER_OF_BIQUAD_STAGES,
                                      (q31_t*) &BIQUAD_COEFF[2 * (NUMBER_OF_BIQUAD_STAGES * 5)],
                                      &biquadStateBand3Q31[0], COEFFICIENT_POSTSHIFT);

    // Bandpass filter 4:
    arm_biquad_cascade_df1_init_q31(&B4, NUMBER_OF_BIQUAD_STAGES,
                                    (q31_t*) &BIQUAD_COEFF[3 * (NUMBER_OF_BIQUAD_STAGES * 5)],
                                    &biquadStateBand4Q31[0], COEFFICIENT_POSTSHIFT);

    // Bandpass filter 5:
    arm_biquad_cascade_df1_init_q31(&B5, NUMBER_OF_BIQUAD_STAGES,
                                    (q31_t*) &BIQUAD_COEFF[4 * (NUMBER_OF_BIQUAD_STAGES * 5)],
                                    &biquadStateBand5Q31[0], COEFFICIENT_POSTSHIFT);

    // Bandpass filter 6:
    arm_biquad_cascade_df1_init_q31(&B6, NUMBER_OF_BIQUAD_STAGES,
                                    (q31_t*) &BIQUAD_COEFF[5 * (NUMBER_OF_BIQUAD_STAGES * 5)],
                                    &biquadStateBand6Q31[0], COEFFICIENT_POSTSHIFT);
}

/**
 *******************************************************************************
 * @brief:     Apply the IIR filters using the ARM CMSIS DSP library and the SciPy
 *             Generated Q31 Coefficients that have been scaled
 * @parameter: int16_t* pSrc      - Pointer to the source buffer
 *             int16_t* pDest     - Pointer to the destination buffer
 *             uint16_t blocksize - Number of samples to use in the filter
 * @return:    N/A
 *******************************************************************************
 */
static void ARM_Equalizer(int16_t* pSrc, int16_t* pDest, uint16_t blocksize)
{
    // Convert pSrc to q31_t format (q15 works for int16)
    arm_q15_to_q31(pSrc, q31Src, blocksize);

    // Scale the input audio down to leave room for gain by a factor of -1/8 - 2^(-3)
    // 0x7FFFFFFF represents the fractional portion of the scale value (this can be left as is)
    arm_scale_q31(q31Src, 0x7FFFFFFF, -3, q31Src, blocksize);

    // Apply 6 bandpass filters using the two different versions
    arm_biquad_cas_df1_32x64_q31(&B1, q31Src, outputB1, blocksize);
    arm_biquad_cas_df1_32x64_q31(&B2, q31Src, outputB2, blocksize);
    arm_biquad_cas_df1_32x64_q31(&B3, q31Src, outputB3, blocksize);
    arm_biquad_cascade_df1_q31(&B4, q31Src, outputB4, blocksize);
    arm_biquad_cascade_df1_q31(&B5, q31Src, outputB5, blocksize);
    arm_biquad_cascade_df1_q31(&B6, q31Src, outputB6, blocksize);

    // Scale any one of the 6 bands, note this is not implemented...
    // .. but rather just shows an example of how to equalize the audio
	{
		// SCALE HERE IF DESIRED
		arm_scale_q31(outputB1, 0x7FFFFFFF, SCALE_FACTOR, outputB1, blocksize);
	}

    // Add corresponding elements of the 6 source buffers
    arm_add_q31(outputB1, outputB2, q31Dest, blocksize);
    arm_add_q31(q31Dest, outputB3, q31Dest, blocksize);
    arm_add_q31(q31Dest, outputB4, q31Dest, blocksize);
    arm_add_q31(q31Dest, outputB4, q31Dest, blocksize);
    arm_add_q31(q31Dest, outputB6, q31Dest, blocksize);

    // Scale the output back up to the original range in Q31 format by a factor of 8 - 2^(3)
    arm_scale_q31(q31Dest, 0x7FFFFFFF, 3, q31Dest, SAMPLES_PER_TRANSFER);

    // Convert q31 Dest to int16_t format (q15 works for int16)
    arm_q31_to_q15(q31Dest, pDest, SAMPLES_PER_TRANSFER);
}

/**
 *******************************************************************************
 * @brief:     User custom data obtaining implemenation. This can be changed 
 * @parameter: int16_t* databuf - Pointer to the data buffer
 * @return:    N/A
 *******************************************************************************
 */
__attribute__((weak)) void user_custom_data_obtaining(int16_t* databuf)
{
    // Default weak implementation
	UNUSED(databuf);
}

/**
 *******************************************************************************
 * @brief:     User custom data transfer implemenation. This can be changed 
 * @parameter: int16_t* databuf - Pointer to the data buffer
 * @return:    N/A
 *******************************************************************************
 */
__attribute__((weak)) void user_custom_data_transfer(int16_t* databuf)
{
    // Default weak implementation
	UNUSED(databuf);
}

// ************************************End of file******************************
