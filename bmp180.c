/* Name: bmp180.c
 * Author: Mikhail Zhyhariev
 * License: MIT
 */

#include <util/delay.h>
#include <math.h>
#include "twi/i2c.h"
#include "bmp180.h"

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

char BPM180_readRegValue(unsigned char reg) {
    _BMP180_moveToReg(reg);

    i2c_start_cond();
    i2c_send_byte(BMP180_READ);
    char byte = i2c_get_byte(1);
    i2c_stop_cond();

    return byte;
}

int BMP180_readNthByteRegValue(unsigned char reg, unsigned char bytes) {
    int data = 0;
    for (unsigned char i = 0; i < bytes; i++) {
        data |= BPM180_readRegValue(reg + i) << (8 * (bytes - i - 1));
    }
    return data;
}

bmp180_calibration BMP180_getCalibrationData(void) {
    bmp180_calibration bmp180;

    bmp180.AC1 = BMP180_readNthByteRegValue(AC1_HIGH, 2);
    bmp180.AC2 = BMP180_readNthByteRegValue(AC2_HIGH, 2);
    bmp180.AC3 = BMP180_readNthByteRegValue(AC3_HIGH, 2);
    bmp180.AC4 = BMP180_readNthByteRegValue(AC4_HIGH, 2);
    bmp180.AC5 = BMP180_readNthByteRegValue(AC5_HIGH, 2);
    bmp180.AC6 = BMP180_readNthByteRegValue(AC6_HIGH, 2);
    bmp180.B1 = BMP180_readNthByteRegValue(B1_HIGH, 2);
    bmp180.B2 = BMP180_readNthByteRegValue(B2_HIGH, 2);
    bmp180.MB = BMP180_readNthByteRegValue(MB_HIGH, 2);
    bmp180.MC = BMP180_readNthByteRegValue(MC_HIGH, 2);
    bmp180.MD = BMP180_readNthByteRegValue(MD_HIGH, 2);

    return bmp180;
}

int BMP180_readUncompTemp(void) {
    BMP180_writeToReg(OSS_REG, OSS_TEMP_VALUE);
    _delay_us(4500);
    return BMP180_readNthByteRegValue(DATA_HIGH, 2);
}

int BMP180_readUncompPressure(void) {
    BMP180_writeToReg(OSS_REG, OSS_PRES_VALUE + (OSS << 6));
    _delay_us(4500);
    return BMP180_readNthByteRegValue(DATA_HIGH, 3) >> (8 - OSS);
}

bpm180_temp BMP180_getTemp(void) {
    bmp180_calibration bmp180_calibr = BMP180_getCalibrationData();
    bpm180_temp bmp180_t;
    bmp180_t.UT = BMP180_readUncompTemp();
    bmp180_t.X1 = (bmp180_t.UT - bmp180_calibr.AC6) * bmp180_calibr.AC5 / pow(2, 15);
    bmp180_t.X2 = bmp180_calibr.MC * pow(2, 11) / (bmp180_t.X1 + bmp180_calibr.MD);
    bmp180_t.B5 = bmp180_t.X1 + bmp180_t.X2;
    bmp180_t.temp = (bmp180_t.B5 + 8) / pow(2, 4);

    return bmp180_t;
}

bmp180_pressure BMP180_getPressure(void) {
    bmp180_calibration bmp180_calibr = BMP180_getCalibrationData();
    bpm180_temp bmp180_t = BMP180_getTemp();
    bmp180_pressure bmp180_p;

    bmp180_p.UP = BMP180_readUncompPressure();
    bmp180_p.B6 = bmp180_t.B5 - 4000;
    bmp180_p.X1 = bmp180_calibr.B2 * (bmp180_p.B6 * bmp180_p.B6 / pow(2, 12)) / pow(2, 11);
    bmp180_p.X2 = bmp180_calibr.AC2 * bmp180_p.B6 / pow(2, 11);
    bmp180_p.X3 = bmp180_p.X1 + bmp180_p.X2;
    bmp180_p.B3 = (((bmp180_calibr.AC1 * 4 + bmp180_p.X3) << OSS) + 2) / 4;
    bmp180_p.X1 = bmp180_calibr.AC3 * bmp180_p.B6 / pow(2, 13);
    bmp180_p.X2 = bmp180_calibr.B1 * (bmp180_p.B6 * bmp180_p.B6 / pow(2, 12)) / pow(2, 16);
    bmp180_p.X3 = ((bmp180_p.X1 + bmp180_p.X2) + 2) / pow(2, 2);
    bmp180_p.B4 = bmp180_calibr.AC4 * (unsigned int)(bmp180_p.X3 + 32768) / pow(2, 15);
    bmp180_p.B7 = ((unsigned int)bmp180_p.UP - bmp180_p.B3) * (50000 >> OSS);

    if (bmp180_p.B7 < 0x80000000) {
        bmp180_p.pressure = (bmp180_p.B7 * 2) / bmp180_p.B4;
    } else {
        bmp180_p.pressure = (bmp180_p.B7 / bmp180_p.B4) * 2;
    }

    bmp180_p.X1 = (bmp180_p.pressure / pow(2, 8)) * (bmp180_p.pressure / pow(2, 8));
    bmp180_p.X1 = (bmp180_p.X1 * 3038) / pow(2, 16);
    bmp180_p.X2 = (-7357 * bmp180_p.pressure) / pow(2, 16);
    bmp180_p.pressure = bmp180_p.pressure + (bmp180_p.X1 + bmp180_p.X2 + 3791) / pow(2, 4);

    return bmp180_p;
}

float BMP180_getHeight(void) {
    bmp180_pressure pres_str = BMP180_getPressure();
    return 44330 * (1 - pow(pres_str.pressure / SEA_LEVEL_PRESSURE, 1 / 5.255));
}
