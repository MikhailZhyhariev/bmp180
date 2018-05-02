/* Name: bmp180.c
 * Author: Mikhail Zhyhariev
 * License: MIT
 */

#include <util/delay.h>
#include <math.h>
#include <stdlib.h>
#include "bmp180.h"

// Global structure than contains calibration coefficients
bmp180_calibration bmp180;

/**
 * Initialise and set the settings BMP180
 */
void BMP180_Init(void) {
    // Initialise i2c interface
    I2C_init();
    // Get calibration coefficients
    BMP180_getCalibrationData();
}

/**
 * Move pointer to register
 * @param reg register address [hex]
 */
void _BMP180_moveToReg(u8 reg) {
    I2C_start();
    I2C_send(BMP180_WRITE);
    I2C_send(reg);
    I2C_stop();
}

/**
 * Writing value to the register
 * @param reg   - register address [hex]
 * @param value - value to write
 */
void BMP180_writeToReg(u8 reg, u8 value) {
    I2C_start();
    I2C_send(BMP180_WRITE);
    I2C_send(reg);
    I2C_send(value);
    I2C_stop();
}

/**
 * Getting the one-byte value from a register
 * @param  reg - register address [hex]
 * @return     register value
 */
u8 BPM180_readRegValue(u8 reg) {
    _BMP180_moveToReg(reg);

    I2C_start();
    I2C_send(BMP180_READ);
    u8 byte = I2C_get(1);
    I2C_stop();

    return byte;
}

/**
 * Getting the n-byte value from a register
 * @param  reg   - register address [hex]
 * @param  bytes - number of bytes of the register
 * @return       register value
 */
s32 BMP180_readNthByteFromReg(u8 reg, u8 bytes) {
    // Move to register
    _BMP180_moveToReg(reg);

    u32 value = 0;
    for (u8 i = 0; i < bytes; i++) {
        // Reading the value from register
        u8 temp = BPM180_readRegValue(reg + i);
        // Add value read from the register and shift it
        value += temp * pow(2, 8 * (bytes - i - 1));
    }
    return value;
}

/**
 * Reading calibration data from EEPROM BMP180
 * @return  the structure that contains all calibration coefficient
 */
bmp180_calibration BMP180_getCalibrationData(void) {
    bmp180.AC1 = BMP180_readNthByteFromReg(AC1_HIGH, 2);
    bmp180.AC2 = BMP180_readNthByteFromReg(AC2_HIGH, 2);
    bmp180.AC3 = BMP180_readNthByteFromReg(AC3_HIGH, 2);
    bmp180.AC4 = BMP180_readNthByteFromReg(AC4_HIGH, 2);
    bmp180.AC5 = BMP180_readNthByteFromReg(AC5_HIGH, 2);
    bmp180.AC6 = BMP180_readNthByteFromReg(AC6_HIGH, 2);
    bmp180.B1 = BMP180_readNthByteFromReg(B1_HIGH, 2);
    bmp180.B2 = BMP180_readNthByteFromReg(B2_HIGH, 2);
    bmp180.MB = BMP180_readNthByteFromReg(MB_HIGH, 2);
    bmp180.MC = BMP180_readNthByteFromReg(MC_HIGH, 2);
    bmp180.MD = BMP180_readNthByteFromReg(MD_HIGH, 2);

    return bmp180;
}

/**
 * Reading two-byte uncompensating temperature value from BMP180
 * @return  uncompensating temperature value
 */
s32 BMP180_readUncompTemp(void) {
    BMP180_writeToReg(OSS_REG, OSS_TEMP_VALUE);
    _delay_us(TEMP_CONV_TIME);
    return BMP180_readNthByteFromReg(DATA_HIGH, 2);
}

/**
 * Reading three-byte uncompensating pressure value from BMP180
 * @return  uncompensating pressure value
 */
s32 BMP180_readUncompPressure(void) {
    BMP180_writeToReg(OSS_REG, OSS_PRES_VALUE + (OSS << 6));
    _delay_us(PRES_CONV_TIME);
    return BMP180_readNthByteFromReg(DATA_HIGH, 3) >> (8 - OSS);
}

/**
 * Calculating true temperature value [degrees Celsius]
 * @return  true temperature value [degrees Celsius]
 */
s32 BMP180_getTemp(void) {
    u32 UT = BMP180_readUncompTemp();

    u32 X1 = (UT - bmp180.AC6) * bmp180.AC5 / pow(2, 15);
    u32 X2 = bmp180.MC * pow(2, 11) / (X1 + bmp180.MD);
    bmp180.B5 = X1 + X2;

    // s32* result = (s32 *)malloc(sizeof(s32));
    // *result = (bmp180.B5 + 8) / pow(2, 4) / 10;
    // return result;
    return (bmp180.B5 + 8) / pow(2, 4) / 10;
}

/**
 * Calculating true pressure value [Pa]
 * @return  true pressure value [Pa]
 */
s32 BMP180_getPressure(void) {
    BMP180_getTemp();
    s32 UP = BMP180_readUncompPressure();

    bmp180.B6 = bmp180.B5 - 4000;
    s32 X1 = (bmp180.B2 * (bmp180.B6 * bmp180.B6 / pow(2, 12))) / pow(2, 11);
    s32 X2 = bmp180.AC2 * bmp180.B6 / pow(2, 11);
    s32 X3 = X1 + X2;
    bmp180.B3 = (((bmp180.AC1 * 4 + X3) << OSS) + 2) / 4;
    X1 = bmp180.AC3 * bmp180.B6 / pow(2, 13);
    X2 = (bmp180.B1 * (bmp180.B6 * bmp180.B6 / pow(2, 12))) / pow(2, 16);
    X3 = ((X1 + X2) + 2) / pow(2, 2);
    bmp180.B4 = bmp180.AC4 * (u32)(X3 + 32768) / pow(2, 15);
    bmp180.B7 = ((u32)UP - bmp180.B3) * (50000 >> OSS);
    s32 pressure;
    if (bmp180.B7 < 0x80000000) {
        pressure = (bmp180.B7 * 2) / bmp180.B4;
    } else {
        pressure = (bmp180.B7 / bmp180.B4) * 2;
    }
    X1 = (pressure / pow(2, 8)) * (pressure / pow(2, 8));
    X1 = (X1 * 3038) / pow(2, 16);
    X2 = (-7357 * pressure) / pow(2, 16);
    pressure = pressure + (X1 + X2 + 3791) / pow(2, 4);
    return pressure;
}

/**
 * Calculating height over a sea level [m]
 * @return  height [m]
 */
float BMP180_getHeight(void) {
    u32 pressure = BMP180_getPressure();
    return 44330 * (1 - pow((float)pressure / SEA_LEVEL_PRESSURE, (float)1 / 5.255));
}
