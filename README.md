Description
------------

The BMP180 is the function compatible successor of the BMP085, a new generation of high precision digital pressure sensors for consumer applications.

Here is a small library for work with BMP180.

Supported devices:
-   AVR microcontrollers;
-   orangePi;
-   raspberryPi;
-   etc...

Initialize and usage
--------------------

To initialize, you must use the `BMP180_Init`.

To get temperature value you should use `BMP180_getTemp`.

To get height value you should use `BMP180_getHeight`.

Hardware pressure sampling accuracy modes
-----------------------------------------

You can use various mode. You can specify it in `bmp180.h` just change the value in:
```
#define OSS             YOUR_VALUE
```

Available modes you can see in the table below.

|          Mode         | OSS | Conversion time  pressure max [ms] |
|:---------------------:|:---:|:----------------------------------:|
|     ultra low mode    |  0  |                 4.5                |
|        standart       |  1  |                 7.5                |
|    high resolution    |  2  |                13.5                |
| ultra high resolution |  3  |                25.5                |


Custom variable type
--------------------

Library use `stdint.h` variable types.

```
#include <stdint.h>

// Signed custom variables types
typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;

// Unsigned custom variables types
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
```

Data transmit/receive protocol
------------------------------

In `bpm180.h` file you can specify the path to the I2C library.

```
// Path to I2C library. You can specify your path.
#include "twi/i2c.h"
```

You also can specify you own I2C functions:

```
/**
 * I2C functions name. You can use your own functions.
 * init     - initializing I2C protocol.
 * start    - start condition
 * stop     - stop condition
 * send     - sends the one-byte value. Takes "unsigned char" value as param.
 * get      - receives the one-byte value. Pass "1" if it's the last byte
 */
#define I2C_init()      (i2c_init())
#define I2C_start()     (i2c_start_cond())
#define I2C_stop()      (i2c_stop_cond())
#define I2C_send(x)     (i2c_send_byte(x))
#define I2C_get(x)      (i2c_get_byte(x))
```
