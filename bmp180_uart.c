/* Name: bmp180_uart.c
 * Author: Mikhail Zhyhariev
 * License: MIT
 */

#include "uart/uart.h"
#include "bmp180.h"

/**
 * Checks sign of the data. If data is negative, inverts it and transmitting sign identifier
 * @param data
 * @return      positive data
 */
s32 BMP180_checkSign(s32 data) {
    if (data < 0) {
        USART_Transmit((MINUS >> 8) & 0xFF);
        USART_Transmit(MINUS & 0xFF);
        data = -(data);
    } else {
        USART_Transmit((PLUS >> 8) & 0xFF);
        USART_Transmit(PLUS & 0xFF);
    }

    return data;
}

/**
 * Transmitting a n-byte data
 * @param data
 * @param bytes - number of bytes of the register
 */
void BMP180_nthBytesTransmit(s32 data, u8 bytes) {
    data = BMP180_checkSign(data);
    for (u8 i = 0; i < bytes; i++) {
        USART_Transmit((data >> (8 * (bytes - i - 1))) & 0xFF);
    }
}
