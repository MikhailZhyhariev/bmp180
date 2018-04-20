/* Name: bmp180.h
 * Author: Mikhail Zhyhariev
 * License: MIT
 */

#ifndef BMP180
#define BMP180

#include <stdint.h>

// Calibration registers address
#define AC1_HIGH        0xAA
#define AC1_LOW         0xAB
#define AC2_HIGH        0xAC
#define AC2_LOW         0xAD
#define AC3_HIGH        0xAE
#define AC3_LOW         0xAF
#define AC4_HIGH        0xB0
#define AC4_LOW         0xB1
#define AC5_HIGH        0xB2
#define AC5_LOW         0xB3
#define AC6_HIGH        0xB4
#define AC6_LOW         0xB5
#define B1_HIGH         0xB6
#define B1_LOW          0xB7
#define B2_HIGH         0xB8
#define B2_LOW          0xB9
#define MB_HIGH         0xBA
#define MB_LOW          0xBB
#define MC_HIGH         0xBC
#define MC_LOW          0xBD
#define MD_HIGH         0xBE
#define MD_LOW          0xBF

// Address "oversampling setting" register
#define OSS_REG         0xF4

/**
 *  Oversampling setting
 *  ___________________________________________________
 *  |                       |     |  Conversion time  |
 *  |         Mode          | OSS | pressure max [ms] |
 *  |_______________________|_____|___________________|
 *  |    ultra low mode     |  0  |        4.5        |
 *  |       standart        |  1  |        7.5        |
 *  |    high resolution    |  2  |       13.5        |
 *  | ultra high resolution |  3  |       25.5        |
 *  |_______________________|_____|___________________|
 */
#define OSS             0x01

// Temperature control register value for OSS
#define OSS_TEMP_VALUE  0x2E
// Temperature max. conversion time [us]
#define TEMP_CONV_TIME  4500

// Pressure control register value for OSS
#define OSS_PRES_VALUE  0x34
// Pressure max. conversion time [us]
#if OSS == 0x00
    #define PRES_CONV_TIME  4500
#elif OSS == 0x01
    #define PRES_CONV_TIME  7500
#elif OSS == 0x02
    #define PRES_CONV_TIME  13500
#elif OSS == 0x03
    #define PRES_CONV_TIME  25550
#endif

// Data (temperature or pressure) registers address
#define DATA_HIGH       0xF6
#define DATA_LOW        0xF7
#define DATA_XLOW       0xF8

// BMP180 module address for reading and writing operation
#define BMP180_READ     0xEF
#define BMP180_WRITE    0xEE

// Pressure on a sea level [Pa]
#define SEA_LEVEL_PRESSURE 101325

// Signed custom variables types
typedef short s16;
typedef int32_t s32;

// Unsigned custom variables types
typedef unsigned char u8;
typedef unsigned short u16;
typedef uint32_t u32;

// A structure that contains calibration coefficients
typedef struct {
    // Variables that read from EEPROM BMP180
    s16 AC1;
    s16 AC2;
    s16 AC3;
    u16 AC4;
    u16 AC5;
    u16 AC6;
    s16 B1;
    s16 B2;
    s16 MB;
    s16 MC;
    s16 MD;

    // Coefficients that calculates
    s32 B3;
    u32 B4;
    s32 B5;
    s32 B6;
    u32 B7;
} bmp180_calibration;

/**
 * Initialise and set the settings BMP180
 */
void BMP180_Init(void);

/**
 * Move pointer to register
 * @param reg register address [hex]
 */
void _BMP180_moveToReg(u8 reg);

/**
 * Writing value to the register
 * @param reg   - register address [hex]
 * @param value - value to write
 */
void BMP180_writeToReg(u8 reg, u8 value);

/**
 * Getting the one-byte value from a register
 * @param  reg - register address [hex]
 * @return     register value
 */
u8 BPM180_readRegValue(u8 reg);

/**
 * Getting the n-byte value from a register
 * @param  reg   - register address [hex]
 * @param  bytes - number of bytes of the register
 * @return       register value
 */
s32 BMP180_readNthByteFromReg(u8 reg, u8 bytes);

/**
 * Reading calibration data from EEPROM BMP180
 * @return  the structure that contains all calibration coefficient
 */
bmp180_calibration BMP180_getCalibrationData(void);

/**
 * Reading two-byte uncompensating temperature value from BMP180
 * @return  uncompensating temperature value
 */
s32 BMP180_readUncompTemp(void);

/**
 * Reading three-byte uncompensating pressure value from BMP180
 * @return  uncompensating pressure value
 */
s32 BMP180_readUncompPressure(void);

/**
 * Calculating true temperature value [degrees Celsius]
 * @return  true temperature value [degrees Celsius]
 */
s32 BMP180_getTemp(void);

/**
 * Calculating true pressure value [Pa]
 * @return  true pressure value [Pa]
 */
s32 BMP180_getPressure(void);

/**
 * Calculating height over a sea level [m]
 * @return  height [m]
 */
double BMP180_getHeight(void);


/**
 * UART FUNCTION
 */

// Sign identifiers
#define MINUS 33001
#define PLUS  33002

/**
 * Checks sign of the data. If data is negative, inverts it and transmitting sign identifier
 * @param data
 * @return      positive data
 */
s32 BMP180_checkSign(s32 data);

/**
 * Transmitting a n-byte data
 * @param data
 * @param bytes - number of bytes of the register
 */
void BMP180_nthBytesTransmit(s32 data, u8 bytes);

#endif
