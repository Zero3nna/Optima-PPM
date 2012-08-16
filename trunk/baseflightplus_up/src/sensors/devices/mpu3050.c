#include "board.h"

///////////////////////////////////////////////////////////////////////////////
// MPU3050 Defines and Variables
///////////////////////////////////////////////////////////////////////////////

// Address

#define MPU3050_ADDRESS 0x68

// Registers

#define MPU3050_X_OFFS_H      0x0C
#define MPU3050_X_OFFS_L      0x0D
#define MPU3050_Y_OFFS_H      0x0E
#define MPU3050_Y_OFFS_L      0x0F
#define MPU3050_Z_OFFS_H      0x10
#define MPU3050_Z_OFFS_L      0x11
#define MPU3050_SMPLRT_DIV    0x15
#define MPU3050_DLPF_FS_SYNC  0x16
#define MPU3050_INT_CFG       0x17
#define MPU3050_TEMP_OUT      0x1B
#define MPU3050_GYRO_OUT      0x1D
#define MPU3050_USER_CTRL     0x3D
#define MPU3050_PWR_MGM       0x3E

// Bits

#define ACTL                  0x00
#define OPEN                  0x00
#define LATCH_INT_EN          0x20
#define INT_ANYRD_2CLEAR      0x10
#define RAW_RDY_EN            0x01

#define H_RESET               0x80
#define INTERNAL_OSC          0x00

#define MPU3050_FS_SEL_2000DPS  0x18

#define MPU3050_DLPF_10HZ       0x05
#define MPU3050_DLPF_20HZ       0x04
#define MPU3050_DLPF_42HZ       0x03
#define MPU3050_DLPF_98HZ       0x02
#define MPU3050_DLPF_188HZ      0x01
#define MPU3050_DLPF_256HZ      0x00

#define SAMPLE_RATE_DIVISOR_8KHZ 0x07        // 1000 Hz = 8000/(7 + 1)
#define SAMPLE_RATE_DIVISOR_1KHZ 0x00        // 1000 Hz = 1000/(0 + 1)

// MPU3050 14.375 LSBs per dps at Â±2000 Âº/s
// scale factor to get rad/s: (1/14.375*PI/180) = 0.00121414208834388144
#define MPU3050_GYRO_SCALE_FACTOR     0.00121414208834388144f

///////////////////////////////////////////////////////////////////////////////
// Read Gyro
///////////////////////////////////////////////////////////////////////////////

void mpu3050GyroRead(int16_t *values)
{
    uint8_t buf[6];

    // Get data from device
    i2cRead(MPU3050_ADDRESS, MPU3050_GYRO_OUT, 6, buf);

    values[XAXIS] = ((buf[0] << 8) | buf[1]);
    values[YAXIS] = ((buf[2] << 8) | buf[3]);
    values[ZAXIS] = -((buf[4] << 8) | buf[5]);
}

void mpu3050TempRead(int16_t *temperature)
{
    uint8_t buf[2];

    // Get data from device
    i2cRead(MPU3050_ADDRESS, MPU3050_TEMP_OUT, 2, buf);

    *temperature = (buf[0] << 8) | buf[1];
}

///////////////////////////////////////////////////////////////////////////////
// Gyro Initialization
///////////////////////////////////////////////////////////////////////////////

uint8_t mpu3050Detect(gyro_t *gyro)
{
    if(!i2cWrite(MPU3050_ADDRESS, MPU3050_SMPLRT_DIV, SAMPLE_RATE_DIVISOR_1KHZ)) {
        return false;
    }
    
    gyro->init = mpu3050Init;
    gyro->read = mpu3050GyroRead;
    gyro->temperature = mpu3050TempRead;
    
    return true;
}

void mpu3050Init(void)
{
    uint8_t i;
    uint8_t mpu3050LPF = MPU3050_DLPF_42HZ;
    
    delay(20);
    
    switch (cfg.gyroLPF) {
        case 256:
            mpu3050LPF = MPU3050_DLPF_256HZ;
            break;
        case 188:
            mpu3050LPF = MPU3050_DLPF_188HZ;
            break;
        case 98:
            mpu3050LPF = MPU3050_DLPF_98HZ;
            break;
        case 42:
            mpu3050LPF = MPU3050_DLPF_42HZ;
            break;
        case 20:
            mpu3050LPF = MPU3050_DLPF_20HZ;
            break;
        case 10:
            mpu3050LPF = MPU3050_DLPF_10HZ;
            break;
    }
    
    for(i = 0; i < 3; ++i) {
        sensors.gyroScaleFactor[i] = MPU3050_GYRO_SCALE_FACTOR;
    }
    
    i2cWrite(MPU3050_ADDRESS, MPU3050_PWR_MGM, H_RESET);
    i2cWrite(MPU3050_ADDRESS, MPU3050_PWR_MGM, INTERNAL_OSC);
    i2cWrite(MPU3050_ADDRESS, MPU3050_DLPF_FS_SYNC, mpu3050LPF | MPU3050_FS_SEL_2000DPS);
    if(mpu3050LPF == MPU3050_DLPF_256HZ)
        i2cWrite(MPU3050_ADDRESS, MPU3050_SMPLRT_DIV, SAMPLE_RATE_DIVISOR_8KHZ);
    else
        i2cWrite(MPU3050_ADDRESS, MPU3050_SMPLRT_DIV, SAMPLE_RATE_DIVISOR_1KHZ);
    i2cWrite(MPU3050_ADDRESS, MPU3050_INT_CFG, 0);
    
}

///////////////////////////////////////////////////////////////////////////////