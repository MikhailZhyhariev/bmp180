/* Name: bmp180.h
 * Author: Mikhail Zhyhariev
 * License: MIT
 */

#ifndef BMP180
#define BMP180

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

#define OSS_REG         0xF4
#define OSS_TEMP_VALUE  0x2E
#define OSS_PRES_VALUE  0x34
#define OSS             0x01
#define DATA_HIGH       0xF6
#define DATA_LOW        0xF7
#define DATA_XLOW       0xF8


#define BMP180_READ     0xEF
#define BMP180_WRITE    0xEE

#define SEA_LEVEL_PRESSURE 101325 // [Pa]

typedef struct {
    short AC1;
    short AC2;
    short AC3;
    unsigned short AC4;
    unsigned short AC5;
    unsigned short AC6;
    short B1;
    short B2;
    short MB;
    short MC;
    short MD;

    long B3;
    unsigned long B4;
    long B5;
    long B6;
    unsigned long B7;
} bmp180_calibration;

void BMP180_Init(void);

void _BMP180_moveToReg(unsigned char reg);

void BMP180_writeToReg(unsigned char reg, unsigned char value);

unsigned char BPM180_readRegValue(unsigned char reg);

long BMP180_readNthByteFromReg(unsigned char reg, unsigned char bytes);

void BMP180_getCalibrationData(void);

long BMP180_readUncompTemp(void);

long BMP180_readUncompPressure(void);

long BMP180_getTemp(void);

long BMP180_getPressure(void);

float BMP180_getHeight(void);

#endif
