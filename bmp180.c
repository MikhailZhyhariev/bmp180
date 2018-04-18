/* Name: bmp180.c
 * Author: Mikhail Zhyhariev
 * License: MIT
 */

#include <util/delay.h>
#include <math.h>
#include "twi/i2c.h"
#include "bmp180.h"

bmp180_calibration bmp180;

void BMP180_Init(void) {
    i2c_init();
}

void _BMP180_moveToReg(unsigned char reg) {
    i2c_start_cond();
    i2c_send_byte(BMP180_WRITE);
    i2c_send_byte(reg);
    i2c_stop_cond();
}

void BMP180_writeToReg(unsigned char reg, unsigned char value) {
    i2c_start_cond();
    i2c_send_byte(BMP180_WRITE);
    i2c_send_byte(reg);
    i2c_send_byte(value);
    i2c_stop_cond();
}

unsigned char BPM180_readRegValue(unsigned char reg) {
    _BMP180_moveToReg(reg);

    i2c_start_cond();
    i2c_send_byte(BMP180_READ);
    char byte = i2c_get_byte(1);
    i2c_stop_cond();

    return byte;
}

long BMP180_readNthByteFromReg(unsigned char reg, unsigned char bytes) {
    long data = 0;
    for (unsigned char i = 0; i < bytes; i++) {
        data |= BPM180_readRegValue(reg + i) << (8 * (bytes - i - 1));
    }
    return data;
}

void BMP180_getCalibrationData(void) {
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
}

long BMP180_readUncompTemp(void) {
    BMP180_writeToReg(OSS_REG, OSS_TEMP_VALUE);
    _delay_us(4500);
    return BMP180_readNthByteFromReg(DATA_HIGH, 2);
}

long BMP180_readUncompPressure(void) {
    BMP180_writeToReg(OSS_REG, OSS_PRES_VALUE + (OSS << 6));
    _delay_us(4500);
    return BMP180_readNthByteFromReg(DATA_HIGH, 3) >> (8 - OSS);
}

long BMP180_getTemp(void) {
    BMP180_getCalibrationData();
    long UT = BMP180_readUncompTemp();

    long X1 = (UT - bmp180.AC6) * bmp180.AC5 / pow(2, 15);
    long X2 = bmp180.MC * pow(2, 11) / (X1 + bmp180.MD);
    bmp180.B5 = X1 + X2;
    return (bmp180.B5 + 8) / pow(2, 4);
}

long BMP180_getPressure(void) {
    BMP180_getTemp();
    long UP = BMP180_readUncompPressure();

    bmp180.B6 = bmp180.B5 - 4000;
    long X1 = (bmp180.B2 * (bmp180.B6 * bmp180.B6 / pow(2, 12))) / pow(2, 11);
    long X2 = bmp180.AC2 * bmp180.B6 / pow(2, 11);
    long X3 = X1 + X2;
    bmp180.B3 = (((bmp180.AC1 * 4 + X3) << OSS) + 2) / 4;
    X1 = bmp180.AC3 * bmp180.B6 / pow(2, 13);
    X2 = (bmp180.B1 * (bmp180.B6 * bmp180.B6 / pow(2, 12))) / pow(2, 16);
    X3 = ((X1 + X2) + 2) / pow(2, 2);
    bmp180.B4 = bmp180.AC4 * (unsigned long)(X3 + 32768) / pow(2, 15);
    bmp180.B7 = ((unsigned long)UP - bmp180.B3) * (50000 >> OSS);
    long pressure;
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

float BMP180_getHeight(void) {
    long pressure = BMP180_getPressure();
    return 44330 * (1 - pow(pressure / SEA_LEVEL_PRESSURE, 1 / 5.255));
}
